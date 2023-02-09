IncludeDir = {}
IncludeDir.SDL = "%{wks.location}/dependencies/SDL2/include"

LibraryDir = {}
LibraryDir.SDL = "%{wks.location}/dependencies/SDL2/lib"

workspace "md_2023"
    startproject "02"
    architecture "x86_64"
    configurations { "Debug", "Release" }

    flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

--[[include "01"
include "02"
include "03"]]
include "03"
--[[include "03_conn_list_draw"
include "CircleIntersection"]]
include "Display"