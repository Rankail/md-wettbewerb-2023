project "Checker"  
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

    postbuildcommands {
        '{COPYFILE} "%{cfg.buildtarget.relpath}" "%{wks.location}/results/%{cfg.buildtarget.basename}_%{cfg.buildcfg}%{cfg.buildtarget.extension}"'
    }

    filter "configurations:*Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:*Release"
        defines { "NDEBUG" }
        optimize "Speed"