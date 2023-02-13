project "02"  
    kind "ConsoleApp" 
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    
    files {
        "src/**.h",
        "src/**.cpp",
        "src/**.txt"
    }

    prebuildcommands {
        '{COPYFILE} "%{wks.location}dependencies/SDL2/lib/*.dll" "%{wks.location}%{prj.name}"'
    }

    filter "system:windows"

        includedirs {
            "%{IncludeDir.SDL}",
        }

        libdirs{
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