# ======================================================================
#  laropticks main build file
#  
#  
# ======================================================================
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
find_package(cetmodules 3.24.00 REQUIRED)
project(laropticks VERSION 10.09.01 LANGUAGES CXX)

include(CetCMakeEnv)
cet_cmake_env()

# -Wno-unused-variable needed because of CLHEP
cet_set_compiler_flags(DIAGS CAUTIOUS
  WERROR
  NO_UNDEFINED
  EXTRA_FLAGS -pedantic -Wno-unused-local-typedefs -Wno-unused-variable
)

cet_report_compiler_flags(REPORT_THRESHOLD VERBOSE)

cet_cmake_module_directories(Modules BINARY)


find_package(artg4tk REQUIRED EXPORT)
find_package(art REQUIRED EXPORT)
find_package(art_root_io REQUIRED)
find_package(cetlib REQUIRED EXPORT)
find_package(cetlib_except REQUIRED EXPORT)
find_package(fhiclcpp REQUIRED EXPORT)
find_package(messagefacility REQUIRED)

find_package(CLHEP COMPONENTS Evaluator Geometry Random Vector REQUIRED EXPORT)
find_package(Geant4 REQUIRED EXPORT)
find_package(ROOT COMPONENTS Core EG GenVector Geom Gpad Hist MathCore Matrix Physics RIO RooFitCore Tree REQUIRED EXPORT)
find_package(larsim REQUIRED EXPORT)
find_package(lardataobj REQUIRED EXPORT)


## Opticks package
list(APPEND CMAKE_MODULE_PATH "$ENV{OPTICKS_HOME}/cmake/Modules")
set(OPTICKS_PREFIX "$ENV{OPTICKS_PREFIX}")
#include(OpticksBuildOptions)
find_dependency(PLog)
find_package(G4CX REQUIRED CONFIG)
find_package(U4 REQUIRED CONFIG)

# macros for dictionary and simple_plugin
include(ArtDictionary)
include(CetMake)
include(BuildPlugins)

# include sub directories
add_subdirectory(laropticks)
# packaging utility
cet_cmake_config()
