cmake_minimum_required( VERSION 3.6 )

# Language
enable_language( CXX )

# Compiler Settings
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

# Project
project( sync_align LANGUAGES CXX )
add_executable( sync_align util.h orbbec.hpp orbbec.cpp main.cpp )

# (Option) Start-Up Project for Visual Studio
set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT "sync_align" )

# Find Package
find_package( OpenCV REQUIRED )
set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" )
find_package( OrbbecSDK REQUIRED )

# Set Package to Project
if( OrbbecSDK_FOUND AND OpenCV_FOUND )
  target_link_libraries( sync_align Orbbec::OrbbecSDK )
  target_link_libraries( sync_align ${OpenCV_LIBS} )
endif()
