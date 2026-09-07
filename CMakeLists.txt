cmake_minimum_required(VERSION 3.5.0)
project(DSC_Analysis)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

find_package(ITK NO_MODULE REQUIRED)
include(${ITK_USE_FILE})

set(DSCMRIAnalysis_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
add_subdirectory(PkSolver)
add_subdirectory(CLI)
