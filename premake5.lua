workspace "ToGoodEngine"
    architecture "x64"
    configurations { "Debug", "Release" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["glfw"] = "Libs/glfw/include"
IncludeDir["glad"] = "Libs/glad/include"
IncludeDir["ImGui"] = "Libs/imgui"
IncludeDir["glm"] = "Libs/glm"
IncludeDir["stb_image"] = "Libs/STB"

project "Engine"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    targetdir ("bin/" .. outputdir)
    objdir ("bin-int/" .. outputdir)

    -- Define source files for your project
    files {
        "Engine/src/**.h",
        "Engine/src/**.hpp",
        "Engine/src/**.c",
        "Engine/src/**.cpp",
        "Engine/ext/**.h",
        "Engine/ext/**.hpp",
        "Engine/ext/**.c",
        "Engine/ext/**.cpp",
    }

    includedirs {
        "Engine/src",
        "Engine/ext",
        IncludeDir["glfw"],
        IncludeDir["glad"],
        IncludeDir["ImGui"],
        IncludeDir["glm"],
        IncludeDir["stb_image"],
    }

    links {
        "GLFW",
        "Glad",
        "opengl32",
        "ImGui"
    }

    libdirs {
        "Libs/imgui/bin/" .. outputdir .. "/ImGui",
        "Libs/glfw/bin/" .. outputdir .. "/GLFW",
        "Libs/glad/bin/" .. outputdir .. "/GLAD",
    }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

-- Include GLFW, GLAD, and ImGui projects
include "Libs/glfw"
include "Libs/glad"
include "Libs/imgui"