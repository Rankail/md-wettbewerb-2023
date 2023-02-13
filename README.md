# Mathe-Dual Wettbewerb 2023

Überlegungen und Implementierungen von Algorithmen für den Mathe-Dual-Wettbewerb 2023.

## Setup
You need [premake](https://premake.github.io/)

### Windows
```
cd vs-cpp
premake5 vs2022
```
If you don't have Visual Studio 2022 you can also use `vs2019`, `vs2017` and so on.\
For Cygwin and MinGW use `gmake2`.

### Linux
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
./04_draw_Release.exe inputfile outputfile [weighting]
```
\
Render outputfile:
```
./Display_Release.exe outputfile
```
If you are using Linux you need to install `libsdl2-dev`.

## Optimizations
Possible Positions of circles are saved as 'Connections'.\
Those 'Connections' are: Corners, the right and left side of a Wall-Circle- and Circle-Circle-Connection
Each connection is first saved in a list of unknown Connections
While searching for a fitting position for the next Circle the max-Radius for each Connection is calculated by trying each Radius from smallest to largest. If the Max-Radius is the radius of the Circle that needs to be placed it's a perfect fit and that Connection is used. If there is not a perfect fit the next-best position is used.\
All other Connections which maxRadius has been calculated are moved to a list of calculated Connections. Then all Connections that could potentially collide with the newly placed circle are removed from the calculated list. Connections with a maxRadius of 0 are deleted. Connections that are being recalculated don't need to check all radii.