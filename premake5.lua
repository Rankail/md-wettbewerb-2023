IncludeDir = {}
IncludeDir.SDL = "%{wks.location}/dependencies/SDL2/include"

LibraryDir = {}
LibraryDir.SDL = "%{wks.location}/dependencies/SDL2/lib"

workspace "md_2023"
    startproject "Solver"
    architecture "x86_64"
    configurations { "Debug", "Release", "SDL_Debug", "SDL_Release" }

    flags {
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "Solver"
include "Display"
include "Scramble"
include "ImageFromCircles"
include "ImageFromTypes"
include "FrameAnimation"
include "Checker"