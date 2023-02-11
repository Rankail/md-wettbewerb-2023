# Mathe-Dual Wettbewerb 2023

Überlegungen und Implementierungen von Algorithmen für den Mathe-Dual-Wettbewerb 2023.

## Setup
You need [premake](https://premake.github.io/)

### Windows
```
cd vs-cpp
premake5 vs2022
```
If you don't have Visual Studio 2022 you can also use `vs2019`, `vs2917` and so on.\
For Cygwin and MinGW use `gmake2`.

### Linux
```
cd vs-cpp
premake5 gmake2
make config=release
```

## Execute

The executables are in `bin`

This generates output from the specified inputfile:
```
03.exe [inputfile] [outputfile]
```
\
Renders an outputfile (only works under Windows for now):
```
Display.exe [file]
```


