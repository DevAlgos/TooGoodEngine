IncludeDir = {}
IncludeDir["glfw"] = "../Libs/glfw/include"
IncludeDir["glad"] = "../Libs/glad/include"
IncludeDir["ImGui"] = "../Libs/imgui"
IncludeDir["glm"] = "../Libs/glm"
IncludeDir["stb_image"] = "../Libs/STB"
IncludeDir["freetype"] = "../Libs/freetype/include"
IncludeDir["OpenAl"]="../Libs/openal-soft/include"
IncludeDir["libsndfile"] = "../Libs/libsndfile/include"




project "Engine-Core"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    targetdir "bin/%{cfg.buildcfg}"
    staticruntime "off"

    files 
    {
        "src/**.h",
        "src/**.hpp",
        "src/**.c",
        "src/**.cpp",
        "ext/**.h",
        "ext/**.hpp",
        "ext/**.c",
        "ext/**.cpp"
    }

    prebuildcommands
    {
        "cmake ../Libs/libsndfile  -DLIBTYPE=STATIC",
        "cmake ../Libs/openal-soft -DLIBTYPE=STATIC"
    }

    includedirs 
    {
        "src",
        "ext",
        IncludeDir["glfw"],
        IncludeDir["glad"],
        IncludeDir["ImGui"],
        IncludeDir["glm"],
        IncludeDir["stb_image"],
        IncludeDir["freetype"],
        IncludeDir["OpenAl"],
        IncludeDir["libsndfile"] 
    }

    links 
    {
        "FreeType",
        "sndfile",
        "OpenAL32",
        "winmm",
        "GLFW",
        "Glad",
        "opengl32",
        "ImGui"
    }

    libdirs 
    {
        "../Libs/imgui/bin/"        .. outputdir ..  "/ImGui",
        "../Libs/glfw/bin/"         .. outputdir ..  "/GLFW",
        "../Libs/glad/bin/"         .. outputdir ..  "/GLAD",
        "../Libs/freetype/bin/"     .. outputdir ..  "/FreeType"
    } 

    targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

        libdirs
        {
            "../Libs/libsndfile/Debug",
            "../Libs/openal-soft/Debug/"
        }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

        libdirs
        {
            "../Libs/libsndfile/Release",
            "../Libs/openal-soft/Release/"
        }

    group "Engine-Dependencies"
        include "../Libs/glfw"
        include "../Libs/glad"
        include "../Libs/imgui"
        include "../Libs/freetype"
        
        externalproject "OpenAL"
             location "../Libs/openal-soft" 
             kind "StaticLib" 
             language "C++"
             cppdialect "C++20"

        externalproject "sndfile"
             location "../Libs/libsndfile" 
             kind "StaticLib" 
             language "C++"
             cppdialect "C++20"
    group ""
