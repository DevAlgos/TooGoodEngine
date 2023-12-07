IncludeDir = {}
IncludeDir["glfw"] = "../Libs/glfw/include"
IncludeDir["glad"] = "../Libs/glad/include"
IncludeDir["ImGui"] = "../Libs/imgui"
IncludeDir["glm"] = "../Libs/glm"
IncludeDir["stb_image"] = "../Libs/STB"
IncludeDir["freetype"] = "../Libs/freetype/include"
IncludeDir["OpenAl"]="../Libs/openal-soft/include"
IncludeDir["libsndfile"] = "../Libs/libsndfile/include"


function runPreBuildCommands()
    os.execute("cmake -B ../Libs/libsndfile -S ../Libs/libsndfile -DLIBTYPE=STATIC")
    os.execute("cmake -B ../Libs/openal-soft -S ../Libs/openal-soft -DLIBTYPE=STATIC")
end


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

    runPreBuildCommands()

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
        "OpenAL",
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
            uuid "BE2461B7-236F-4278-81D3-F0D476F9A4C0"
            kind "StaticLib" 
            language "C++"
            cppdialect "C++20"

        externalproject "sndfile"
            location "../Libs/libsndfile" 
            uuid "BE2461B7-236F-4278-81D3-F0D476F9A4CD"
            kind "StaticLib" 
            language "C++"
            cppdialect "C++20"
    group ""
