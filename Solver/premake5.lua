project "Solver"  
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

    postbuildcommands {
        '{COPYFILE} "%{cfg.buildtarget.relpath}" "%{wks.location}/inputs/%{cfg.buildtarget.basename}_%{cfg.buildcfg}%{cfg.buildtarget.extension}"',
    }

    filter {"configurations:SDL_*", "system:windows"}

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

    filter  {"configurations:SDL_*", "system:linux"}
        links {
            "SDL2"
        }

    filter "configurations:*Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:*Release"
        defines { "NDEBUG" }
        optimize "Speed"

    filter "configurations:SDL_*"
        defines { "DRAW_SDL" }
        postbuildcommands { '{COPYFILE} "%{wks.location}dependencies/SDL2/lib/*.dll" "%{wks.location}/inputs/"' }