project "04_draw"  
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

    debugargs {
        "../inputs/forest01.txt", "../results/forest01.txt.out"
    }

    postbuildcommands {
        '{COPYFILE} "%{cfg.buildtarget.relpath}" "%{wks.location}/inputs/%{cfg.buildtarget.basename}_%{cfg.buildcfg}.exe"'
    }

    filter "system:windows"

        includedirs {
            "%{IncludeDir.SDL}",
        }

        libdirs {
            "%{LibraryDir.SDL}"
        }

        links {
            "SDL2", "SDL2main"
        }

        prebuildcommands {
            '{COPYFILE} "%{wks.location}dependencies/SDL2/lib/*.dll" "%{wks.location}/%{prj.name}"'
        }

    filter "system:linux"
        links {
            "SDL2"
        }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "Speed"