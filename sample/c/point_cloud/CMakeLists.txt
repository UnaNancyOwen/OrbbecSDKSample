cmake_minimum_required( VERSION 3.6 )

# Language
enable_language( CXX )

# Compiler Settings
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

# Project
project( point_cloud LANGUAGES CXX )
add_executable( point_cloud check_error.h orbbec.hpp orbbec.cpp main.cpp )

# (Option) Start-Up Project for Visual Studio
set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT "point_cloud" )

# Find Package
find_package( Open3D REQUIRED )
find_package( OpenMP REQUIRED )
set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" )
find_package( OrbbecSDK REQUIRED )

# Set Package to Project
if( OrbbecSDK_FOUND AND Open3D_FOUND )
  target_link_libraries( point_cloud Orbbec::OrbbecSDK )
  target_link_libraries( point_cloud Open3D::Open3D )
endif()

if(OpenMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

