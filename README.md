# treecat

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