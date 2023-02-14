# Mathe-Dual Wettbewerb 2023

## Setup
You need [premake](https://premake.github.io/)\
There are 4 Configurations: `Debug` `Release` `SDL_Debug` `SDL_Release`\
The SDL-Configurations include SDL for visual debugging.

### Windows
```
cd vs-cpp
premake5 vs2022
```
If you don't have Visual Studio 2022 you can also use `vs2019`, `vs2017` and so on.\
For Cygwin and MinGW use `gmake2`.

### Linux
If you are using Ubuntu or Debian install `libsdl2-dev`\
For other linux-distributions you can look [here](https://lazyfoo.net/tutorials/SDL/01_hello_SDL/linux/index.php)
```
cd vs-cpp
premake5 gmake2
make 04 config=release
```

## Execute

The executables can be found in `bin` and are copied to `inputs` or `results`

```
./04_Release.exe inputfile outputfile [weighting]
```
Weighting:\
0-1 => constant to linear\
1-2 => linear to quadratic\
\
Render and compute:
```
./Solver_Release.exe {--manual | INPUTFILE OUTPUTFILE [WEIGHTING]}
```
\
Render outputfile:
```
./Display_Release.exe [FILE]
```