workspace "TooGoodEngine"
    architecture "x64"
    configurations { "Debug", "Release" }
    startproject "App"

    filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }


outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Engine-Core"
    include "Engine-Core/premake5.lua"
group ""

include "App/premake5.lua"
include "App"