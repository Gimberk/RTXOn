-- premake5.lua
workspace "RTXOn"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "RTXOn"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include "Walnut/WalnutExternal.lua"

include "RTXOn"