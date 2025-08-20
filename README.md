# treecat

`treecat` is a C++ utility that recursively collects the contents of text files in a specified directory and saves it to an output file.

## Installation and Build

Clone the repository:

```bash
# Installation
git clone https://github.com/koin32/treecat.git
cd treecat
# Build
./build.sh
```

## Usage

Save the contents of files: 


```bash
./build/treecat <directory_or_file_path> <output_file_name>
```

## Format for `grep`

```bash
./build/treecat -g <directory_or_file_path> <output_file_name>
```