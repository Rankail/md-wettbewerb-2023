# Mathe-Dual Wettbewerb 2023 (Team PI)

## Task
You are given the size of a rectangular area and a list of circle-types that aren't necessarily unique.\
Coordinates and radii are doubles.\
The input has this format:
```
Name of Testcase
width height
radius type-name
radius type-name
radius type-name
...
```
Place circles in the rectangle so they don't overlap and the product of the covered area and 1 minus the Simpson-index of the circles grouped by type is maximal. The position of a circle is its center-point.\
The output should have this format:
```
x y radius index-of-type
x y radius index-of-type
x y radius index-of-type
...
```

# How the Solver works
The solver keeps track of "Connections". Those are corners (Corner-connection), a circle touching a wall (Wall-connection) and two circles touching each other (Circle-connection). Circles are placed so they touch both parts of a connection. There are two possible sides for Wall-/Circle-connections, which are stored in two connections.\
The solver takes a weight, which controls how often it tries to place a circle of a radius. Trying smaller circles more often does not lead to better results. The weight maps from 0, where all radii have the same weight, to 1, where the weight is distributed linearly, to 2, where the weight is distributed quadratically. The weight is accumulated for every circle-type. The largest circle-type weight increases by 1 every iteration. After updating the weights of all circle types, the solver iterates over all radii from largest to smallest. If a circle's weight is greater than or equal to 1, the solver tries to find a good connection to place it.\
Selecting a connection is based on a few factors. Connections can have a Max-Radius that needs to be calculated, which is quite expensive. A connection with a max-radius equal to the circle-type that should be placed is an (almost) perfect fit. Before choosing a connection, they are sorted by type (Corner first, then Wall, then Circle-Connection) and max-radius. If there are calculated connections that are a perfect fit for the current radius, the first of those is chosen; otherwise, the unknown connections' max-radius is calculated until a perfect fit is found. If no perfect fit is found, the next best connection is chosen. If there is no connection where the radius fits, it is skipped.\
After placing a new circle, all connections near it are marked as unknown again, so they are checked again before further usage (only checked up to the old max-radius).

The connections aren't sorted perfectly before selection because there is some randomness mixed in. The solver still remains deterministic because you can specify a seed.

The solver keeps track of the maximum score of the placed circles. If the maximum does not change for 3000 circles, the solver stops and cuts the circles back to that maximum.

# What i wanted/forgot/was to lazy to implement and some other thoughts

- Just because a smaller circle doesn't fit at a connection point doesn't infer that a larger one doesn't either. Every radius must be checked and saved individually.
- Only the given radii are checked when calculating the max-radius. Instead, after finding a fitting radius, a gap should be calculated to determine how much the circle size could potentially be increased.
- Instead of cutting the circles back to the maximum, remove one of the most common types one by one.
- looking further than just gaps and instead rating connections by the resulting connections.
---
- Is it even good to restrict the selection further? The randomness accounted for a lot of the additional points we gained.
- Calculate a score instead of sorting by one attribute after another.
- Genetic algorithms would probably not lead to any great results because you can never say how it is going to play out in the end and even a little change can make big differences.

# Setup
You need [premake](https://premake.github.io/)\
There are 4 Configurations: `Debug` `Release` `SDL_Debug` `SDL_Release`\
The SDL-Configurations include SDL for visual debugging.

## Windows
```
cd vs-cpp
premake5 vs2022
```
If you don't have Visual Studio 2022 you can also use `vs2019`, `vs2017` and so on.\
For Cygwin and MinGW use `gmake2`.

## Linux
If you are using Ubuntu or Debian install `libsdl2-dev`\
For other linux-distributions you can look [here](https://lazyfoo.net/tutorials/SDL/01_hello_SDL/linux/index.php)
```
$ premake5 gmake2
$ make Solver config=release
```

# Execute

The executables can be found in `bin` and are copied to `inputs` or `results` after building

## Solver
```
./Solver [INPUTFILE WEIGHTING SEED] [--out=OUTPUTFILE]
```
Weighting (of radii):\
0-1 => constant to linear\
1-2 => linear to quadratic\
Seed:\
0-4294967295

## Display:
Render output
```
./Display [FILE]
```
Press space to show animation of circles appearing in order

## Scrambler:
```
./Scramble [FILE SCRAMBLE_TYPE]
```
Sort by:\
[1] Random\
[2] Distance from center

## ImageFromCircles:
Places Circles to make image
```
./ImageFromCircles [IMAGE INPUT OUTPUT GAP_SCALE SCALE]
```
GAP_SCALE: scale of gap between pixels; relative to biggest color circle

## ImageFromTypes
Sets types of circles to make picture; **only works with equal radii!**
```
./ImageFromTypes [IMAGE OLD_OUTPUT W H NEW_OUTPUT]
```

## FrameAnimation
Sets order of circles according to key-frames\
-> shows circles mapped on black pixels of frame
```
./FrameAnimation [OLD_OUTPUT W H NEW_OUTPUT FRAMES...]
```