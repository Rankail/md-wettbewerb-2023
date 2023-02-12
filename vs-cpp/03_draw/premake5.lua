project "03_draw"  
    kind "ConsoleApp" 
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    
    files {
        "src/**.h",
        "src/**.cpp",
        "src/**.txt"
    }

    includedirs
    {
        "%{IncludeDir.SDL}",
    }

    libdirs
    {
        "%{LibraryDir.SDL}"
    }

    links
    {
        "SDL2", "SDL2main"
    }


    postbuildcommands {
        '{COPYFILE} "%{cfg.buildtarget.relpath}" "%{wks.location}exe/%{cfg.buildtarget.basename}_%{cfg.buildcfg}.exe"'
    }

    filter "system:windows"
        prebuildcommands
        {
            '{COPYFILE} "%{wks.location}dependencies/SDL2/lib/*.dll" "%{wks.location}%{prj.name}"'
        }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "Speed"