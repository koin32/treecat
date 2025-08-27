#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>

namespace fs = std::filesystem;

struct FileInfo {
    std::string content;
    size_t lineCount;
    fs::path path;
    std::vector<std::string> lines;
};

// Функция для разделения строки на подстроки по разделителю
std::vector<std::string> splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}

// Функция для чтения содержимого файла
FileInfo readFileContents(const fs::path& path) {
    FileInfo info;
    info.path = path;
    
    if (fs::is_regular_file(path)) {
        std::ifstream inputFile(path);
        if (inputFile) {
            std::string line;
            std::stringstream contentStream;
            size_t count = 0;

            while (std::getline(inputFile, line)) {
                info.lines.push_back(line);
                contentStream << line << "\n";
                count++;
            }

            info.content = contentStream.str();
            info.lineCount = count;
        }
    }
    return info;
}

// Функция для сохранения в формате grep
void saveGrepFormat(const std::vector<FileInfo>& files, std::ofstream& outputFile, bool showContent = false) {
    for (const auto& file : files) {
        if (file.lineCount > 0) {
            outputFile << file.path.string() << ":" << file.lineCount;
            if (showContent && !file.content.empty()) {
                outputFile << ":\n" << file.content;
            }
            outputFile << "\n";
        }
    }
}

// Функция для сохранения в полном формате
void saveFullFormat(const std::vector<FileInfo>& files, std::ofstream& outputFile) {
    for (const auto& file : files) {
        if (file.lineCount > 0) {
            outputFile << "Contents of " << file.path << " (" << file.lineCount << " lines):\n";
            outputFile << file.content << "\n";
        }
    }
}

// Функция для рекурсивного сбора файлов
void collectFiles(const fs::path& path, std::vector<FileInfo>& files) {
    if (fs::is_regular_file(path)) {
        files.push_back(readFileContents(path));
    } else if (fs::is_directory(path)) {
        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (fs::is_regular_file(entry.path())) {
                files.push_back(readFileContents(entry.path()));
            }
        }
    }
}

// Функция для генерации diff между двумя файлами
void generateDiff(const FileInfo& file1, const FileInfo& file2, std::ofstream& outputFile, const fs::path& basePath1, const fs::path& basePath2) {
    std::string relPath1 = fs::relative(file1.path, basePath1).string();
    std::string relPath2 = fs::relative(file2.path, basePath2).string();
    
    outputFile << "diff --git a/" << relPath1 << " b/" << relPath2 << "\n";
    outputFile << "--- a/" << relPath1 << "\n";
    outputFile << "+++ b/" << relPath2 << "\n";
    
    const auto& lines1 = file1.lines;
    const auto& lines2 = file2.lines;
    
    size_t i = 0, j = 0;
    bool inBlock = false;
    
    while (i < lines1.size() || j < lines2.size()) {
        if (i < lines1.size() && j < lines2.size() && lines1[i] == lines2[j]) {
            if (inBlock) {
                outputFile << " " << lines1[i] << "\n";
                inBlock = false;
            }
            i++;
            j++;
        } else {
            if (!inBlock) {
                outputFile << "@@ -" << (i + 1) << " +" << (j + 1) << " @@\n";
                inBlock = true;
            }
            
            if (i < lines1.size() && (j >= lines2.size() || lines1[i] != lines2[j])) {
                outputFile << "-" << lines1[i] << "\n";
                i++;
            }
            
            if (j < lines2.size() && (i >= lines1.size() || lines1[i] != lines2[j])) {
                outputFile << "+" << lines2[j] << "\n";
                j++;
            }
        }
    }
    
    outputFile << "\n";
}

// Функция для сравнения двух директорий в формате diff
void compareDirectoriesDiff(const fs::path& path1, const fs::path& path2, std::ofstream& outputFile) {
    std::vector<FileInfo> files1, files2;
    
    // Собираем файлы из обеих директорий
    collectFiles(path1, files1);
    collectFiles(path2, files2);
    
    // Создаем карты для быстрого поиска
    std::map<std::string, FileInfo> map1, map2;
    
    for (const auto& file : files1) {
        std::string relativePath = fs::relative(file.path, path1).string();
        map1[relativePath] = file;
    }
    
    for (const auto& file : files2) {
        std::string relativePath = fs::relative(file.path, path2).string();
        map2[relativePath] = file;
    }
    
    outputFile << "=== DIFF COMPARISON REPORT ===\n";
    outputFile << "Comparing: " << path1 << " (a/) vs " << path2 << " (b/)\n\n";
    
    // Файлы только в первой директории (удаленные)
    for (const auto& pair : map1) {
        if (map2.find(pair.first) == map2.end()) {
            outputFile << "Only in " << path1 << ": " << pair.first << "\n";
            outputFile << "--- a/" << pair.first << "\n";
            outputFile << "+++ /dev/null\n";
            outputFile << "@@ -1," << pair.second.lineCount << " +0,0 @@\n";
            for (const auto& line : pair.second.lines) {
                outputFile << "-" << line << "\n";
            }
            outputFile << "\n";
        }
    }
    
    // Файлы только во второй директории (добавленные)
    for (const auto& pair : map2) {
        if (map1.find(pair.first) == map1.end()) {
            outputFile << "Only in " << path2 << ": " << pair.first << "\n";
            outputFile << "--- /dev/null\n";
            outputFile << "+++ b/" << pair.first << "\n";
            outputFile << "@@ -0,0 +1," << pair.second.lineCount << " @@\n";
            for (const auto& line : pair.second.lines) {
                outputFile << "+" << line << "\n";
            }
            outputFile << "\n";
        }
    }
    
    // Сравниваем общие файлы
    for (const auto& pair : map1) {
        if (map2.find(pair.first) != map2.end()) {
            const auto& file1 = pair.second;
            const auto& file2 = map2[pair.first];
            
            if (file1.content != file2.content) {
                outputFile << "Modified: " << pair.first << "\n";
                generateDiff(file1, file2, outputFile, path1, path2);
            }
        }
    }
    
    // Файлы без изменений
    outputFile << "=== UNCHANGED FILES ===\n";
    for (const auto& pair : map1) {
        if (map2.find(pair.first) != map2.end()) {
            const auto& file1 = pair.second;
            const auto& file2 = map2[pair.first];
            
            if (file1.content == file2.content) {
                outputFile << "Unchanged: " << pair.first << " (" << file1.lineCount << " lines)\n";
            }
        }
    }
}

// Функция для сравнения двух директорий в простом формате
void compareDirectoriesSimple(const fs::path& path1, const fs::path& path2, std::ofstream& outputFile) {
    std::vector<FileInfo> files1, files2;
    
    collectFiles(path1, files1);
    collectFiles(path2, files2);
    
    std::map<std::string, FileInfo> map1, map2;
    
    for (const auto& file : files1) {
        map1[fs::relative(file.path, path1).string()] = file;
    }
    
    for (const auto& file : files2) {
        map2[fs::relative(file.path, path2).string()] = file;
    }
    
    outputFile << "=== COMPARISON REPORT ===\n\n";
    
    outputFile << "Files only in " << path1 << ":\n";
    for (const auto& pair : map1) {
        if (map2.find(pair.first) == map2.end()) {
            outputFile << "  + " << pair.first << " (" << pair.second.lineCount << " lines)\n";
        }
    }
    
    outputFile << "\nFiles only in " << path2 << ":\n";
    for (const auto& pair : map2) {
        if (map1.find(pair.first) == map1.end()) {
            outputFile << "  + " << pair.first << " (" << pair.second.lineCount << " lines)\n";
        }
    }
    
    outputFile << "\nCommon files with differences:\n";
    for (const auto& pair : map1) {
        if (map2.find(pair.first) != map2.end()) {
            const auto& file1 = pair.second;
            const auto& file2 = map2[pair.first];
            
            if (file1.content != file2.content) {
                outputFile << "  * " << pair.first << ":\n";
                outputFile << "    - " << path1 << ": " << file1.lineCount << " lines\n";
                outputFile << "    - " << path2 << ": " << file2.lineCount << " lines\n";
            }
        }
    }
}

void printUsage(const char* programName) {
    std::cerr << "Usage:\n";
    std::cerr << "  " << programName << " [-g] <input_path> [-o <output_file>]\n";
    std::cerr << "  " << programName << " -d [-diff] <dir1> <dir2> [-o <output_file>]\n";
    std::cerr << "\nOptions:\n";
    std::cerr << "  -g          : Grep format (file:line_count)\n";
    std::cerr << "  -g -v       : Grep format with content (file:line_count:content)\n";
    std::cerr << "  -d          : Compare two directories (simple format)\n";
    std::cerr << "  -d -diff    : Compare two directories (diff format)\n";
    std::cerr << "  -o <file>   : Specify output file path\n";
    std::cerr << "  --help      : Show this help message\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    bool grepFormat = false;
    bool grepVerbose = false;
    bool compareMode = false;
    bool diffFormat = false;
    fs::path inputPath1, inputPath2;
    fs::path outputPath = "output.txt";
    
    // Парсинг аргументов
    std::vector<std::string> args(argv + 1, argv + argc);
    
    for (size_t i = 0; i < args.size(); i++) {
        if (args[i] == "-g") {
            grepFormat = true;
            if (i + 1 < args.size() && args[i + 1] == "-v") {
                grepVerbose = true;
                i++;
            }
        } else if (args[i] == "-d") {
            compareMode = true;
            if (i + 1 < args.size() && args[i + 1] == "-diff") {
                diffFormat = true;
                i++;
            }
        } else if (args[i] == "-o") {
            if (i + 1 < args.size()) {
                outputPath = args[i + 1];
                i++;
            } else {
                std::cerr << "Error: -o requires output path argument\n";
                return 1;
            }
        } else if (args[i] == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (args[i][0] != '-') {
            if (compareMode) {
                if (inputPath1.empty()) {
                    inputPath1 = args[i];
                } else if (inputPath2.empty()) {
                    inputPath2 = args[i];
                }
            } else {
                inputPath1 = args[i];
            }
        }
    }

    // Валидация аргументов
    if (compareMode) {
        if (inputPath1.empty() || inputPath2.empty()) {
            std::cerr << "Error: -d mode requires two directories to compare\n";
            printUsage(argv[0]);
            return 1;
        }
        if (!fs::exists(inputPath1) || !fs::exists(inputPath2)) {
            std::cerr << "Error: One or both directories do not exist\n";
            return 1;
        }
    } else {
        if (inputPath1.empty()) {
            std::cerr << "Error: No input path specified\n";
            printUsage(argv[0]);
            return 1;
        }
        if (!fs::exists(inputPath1)) {
            std::cerr << "Error: Input path does not exist: " << inputPath1 << "\n";
            return 1;
        }
    }

    // Создаем выходной файл
    std::ofstream outputFile(outputPath);
    if (!outputFile) {
        std::cerr << "Error creating output file: " << outputPath << std::endl;
        return 1;
    }

    if (compareMode) {
        if (diffFormat) {
            compareDirectoriesDiff(inputPath1, inputPath2, outputFile);
            std::cout << "Diff comparison report saved to: " << outputPath << std::endl;
        } else {
            compareDirectoriesSimple(inputPath1, inputPath2, outputFile);
            std::cout << "Comparison report saved to: " << outputPath << std::endl;
        }
    } else {
        std::vector<FileInfo> files;
        collectFiles(inputPath1, files);
        
        if (grepFormat) {
            saveGrepFormat(files, outputFile, grepVerbose);
            std::cout << "Grep format saved to: " << outputPath << std::endl;
        } else {
            saveFullFormat(files, outputFile);
            std::cout << "Full contents saved to: " << outputPath << std::endl;
        }
    }

    outputFile.close();
    return 0;
}