IncludeDir = {}
IncludeDir["glfw"] = "../Libs/glfw/include"
IncludeDir["glad"] = "../Libs/glad/include"
IncludeDir["ImGui"] = "../Libs/imgui"
IncludeDir["glm"] = "../Libs/glm"
IncludeDir["stb_image"] = "../Libs/STB"
IncludeDir["freetype"] = "../Libs/freetype/include"
IncludeDir["OpenAl"]="../Libs/openal-soft-windows-build/include"
IncludeDir["libsndfile"] = "../Libs/libsndfile-windows-build/include"
IncludeDir["assimp"] = "../Libs/assimp/include"


function findPython()
   local pythonDir = os.getenv("PYTHONHOME") or os.getenv("PYTHONPATH")
   
   if pythonDir then
       includedirs { pythonDir .. "/include" }
       libdirs { pythonDir .. "/libs" } -- Change this according to your Python installation structure
       links { "python3" } -- Adjust the Python library name as needed
       links { "python310" }
   else
       -- Handle the case when Python directory is not found
       print("Python directory not found in environment variables.")
   end
end

project "App"
   kind "ConsoleApp"
   location "."
   language "C++"
   cppdialect "C++20"
   targetdir "bin/%{cfg.buildcfg}"
   staticruntime "off"

   files 
   { 
     "src/**.h", 
     "src/**.cpp" 
   }

   includedirs
   {
      "src",
	  "../Engine-Core/src",
      "../Engine-Core/ext",

      IncludeDir["glfw"],
      IncludeDir["glad"],
      IncludeDir["ImGui"],
      IncludeDir["glm"],
      IncludeDir["stb_image"],
      IncludeDir["freetype"],
      IncludeDir["OpenAl"],
      IncludeDir["libsndfile"],
      IncludeDir["assimp"]
   }

   findPython()

   links
   {
      "Engine-Core"
   }

   targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
   objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

   filter "configurations:Debug"
       defines { "DEBUG" }
       runtime "Debug"
       symbols "On"

   filter "configurations:Release"
       defines { "RELEASE" }
       runtime "Release"
       optimize "On"
       symbols "On"