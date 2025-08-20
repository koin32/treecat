#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <sstream>

namespace fs = std::filesystem;

void saveFileContents(const fs::path& path, std::ofstream& outputFile, bool grepFormat) {
    if (fs::is_regular_file(path)) {
        std::ifstream inputFile(path);
        if (inputFile) {
            std::string line;
            size_t lineCount = 0;

            // Считаем количество строк
            while (std::getline(inputFile, line)) {
                lineCount++;
            }

            // Сбросим указатель на начало файла
            inputFile.clear();
            inputFile.seekg(0);

            // Записываем путь и количество строк
            if (grepFormat) {
                outputFile << path << ": " << lineCount << " lines\n";
            } else {
                outputFile << "Contents of " << path << " (" << lineCount << " lines):\n";
            }

            // Считываем содержимое файла
            while (std::getline(inputFile, line)) {
                outputFile << line << "\n"; // Записываем каждую строку
            }
            outputFile << "\n"; // Разделяем содержимое файлов
        } else {
            std::cerr << "Error opening file: " << path << std::endl;
        }
    } else if (fs::is_directory(path)) {
        for (const auto& entry : fs::directory_iterator(path)) {
            saveFileContents(entry.path(), outputFile, grepFormat);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 4) {
        std::cerr << "Usage: " << argv[0] << " [-g] <directory_or_file_path> <output_file_name>\n";
        return 1;
    }

    bool grepFormat = false;
    fs::path inputPath;
    std::string outputFileName;

    // Обработка аргументов
    if (argc == 4 && std::string(argv[1]) == "-g") {
        grepFormat = true;
        inputPath = argv[2];
        outputFileName = argv[3];
    } else {
        inputPath = argv[1];
        outputFileName = argv[2];
    }

    std::ofstream outputFile(outputFileName);
    if (!outputFile) {
        std::cerr << "Error creating output file: " << outputFileName << std::endl;
        return 1;
    }

    saveFileContents(inputPath, outputFile, grepFormat);
    outputFile.close();

    std::cout << "Contents saved to " << outputFileName << std::endl;
    return 0;
}
