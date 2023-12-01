IncludeDir = {}
IncludeDir["glfw"] = "../Libs/glfw/include"
IncludeDir["glad"] = "../Libs/glad/include"
IncludeDir["ImGui"] = "../Libs/imgui"
IncludeDir["glm"] = "../Libs/glm"
IncludeDir["stb_image"] = "../Libs/STB"
IncludeDir["freetype"] = "../Libs/freetype/include"

project "App"
   kind "ConsoleApp"
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
      IncludeDir["freetype"]
   }

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