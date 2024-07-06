IncludeDir = {}
IncludeDir["glfw"] = "../Libs/glfw/include"
IncludeDir["glad"] = "../Libs/glad/include"
IncludeDir["ImGui"] = "../Libs/imgui"
IncludeDir["glm"] = "../Libs/glm"
IncludeDir["stb_image"] = "../Libs/STB"
IncludeDir["freetype"] = "../Libs/freetype/include"
IncludeDir["OpenAl"]="../Libs/openal-soft-windows-build/include"
IncludeDir["libsndfile"] = "../Libs/libsndfile-windows-build/include"
IncludeDir["json"] = "../Libs/json-nlohmann/single_include"
IncludeDir["assimp"] = "../Libs/assimp/include"

function findPython()
    local pythonDir = os.getenv("PYTHONHOME") or os.getenv("PYTHONPATH")
    
    if pythonDir then
        includedirs { pythonDir .. "/include" }
        libdirs { pythonDir .. "/libs" }
        links { "python3" } 
    else
 
        print("Python directory not found in environment variables.")
    end
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

    findPython()

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
        IncludeDir["libsndfile"],
        IncludeDir["json"],
        IncludeDir["assimp"]
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
            "../Libs/libsndfile-windows-build/Debug-Bin",
            "../Libs/openal-soft-windows-build/Debug-Bin",
            "../Libs/assimp/contrib/zlib/Debug",
            "../Libs/assimp/lib/Debug"
        }

        links { "zlibstaticd", "assimp-vc143-mtd" }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

        libdirs
        {
            "../Libs/libsndfile-windows-build/Release-Bin",
            "../Libs/openal-soft-windows-build/Release-Bin",
            "../Libs/assimp/contrib/zlib/Release",
            "../Libs/assimp/lib/Release"
        }        
        
        links { "zlibstatic", "assimp-vc143-mt" }

    group "Engine-Dependencies"
        include "../Libs/glfw"
        include "../Libs/glad"
        include "../Libs/imgui"
        include "../Libs/freetype"
    group ""
