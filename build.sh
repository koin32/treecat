#!/bin/bash

# Проверяем, существует ли директория build
if [ -d "build" ]; then
  echo "Директория 'build' уже существует. Удаляем её..."
  rm -rf build
fi

# Создаем директорию build
mkdir build
cd build || exit

# Запускаем CMake для конфигурации проекта
echo "Запуск CMake..."
cmake ..

# Собираем проект
echo "Сборка проекта..."
make

# Проверяем, была ли сборка успешной
if [ $? -eq 0 ]; then
  echo "Сборка завершена успешно!"
else
  echo "Ошибка сборки!"
  exit 1
fi
