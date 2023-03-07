# Mathe-Dual Wettbewerb 2023 (Team PI)

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
$ premake5 gmake2
$ make Solver config=release
```

## Execute

The executables can be found in `bin` and are copied to `inputs` or `results` after building

### Solver
```
./Solver [INPUTFILE WEIGHTING SEED] [--out=OUTPUTFILE]
```
Weighting (of radii):\
0-1 => constant to linear\
1-2 => linear to quadratic\
Seed:\
0-4294967295

### Display:
Render output
```
./Display [FILE]
```
Press space to show animation of circles appearing in order

### Scrambler:
```
./Scramble [FILE SCRAMBLE_TYPE]
```
Sort by:\
[1] Random\
[2] Distance from center

### ImageFromCircles:
Places Circles to make image
```
./ImageFromCircles [IMAGE INPUT OUTPUT GAP_SCALE SCALE]
```
GAP_SCALE: scale of gap between pixels; relative to biggest color circle

### ImageFromTypes
Sets types of circles to make picture (only works with equal radii)
```
./ImageFromTypes [IMAGE OLD_OUTPUT W H NEW_OUTPUT]
```

### FrameAnimation
Sets order of circles according to key-frames\
-> shows circles mapped on black pixels of frame
```
./FrameAnimation [OLD_OUTPUT W H NEW_OUTPUT FRAMES...]
```