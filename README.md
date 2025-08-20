# treecat
<<<<<<< HEAD
`treecat` is a C++ utility that recursively collects the contents of text files from a specified directory and saves them into a single output file. It supports an optional -g flag to format the output in a way that is convenient for grep, displaying file paths and line counts.
=======

`treecat` — это утилита на C++, которая рекурсивно собирает содержимое текстовых файлов в указанной директории и сохраняет его в выходной файл.

## Установка и сборка 

Клонируйте репозиторий:

```bash
# Установка
git clone https://github.com/koin32/treecat.git
cd treecat
# Сборка
./build.sh
```

## Использование

Сохранение содержимого файлов: 


```bash
./build/treecat <directory_or_file_path> <output_file_name>
```

## Формат для `grep`

```bash
./build/treecat -g <directory_or_file_path> <output_file_name>
```
>>>>>>> f2c4f41 (Initial commit)
