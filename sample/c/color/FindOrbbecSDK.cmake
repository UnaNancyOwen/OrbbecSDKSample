#.rst:
# FindOrbbecSDK
# ---------
#
# Find Orbbec SDK include dirs, and libraries.
#
# IMPORTED Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the :prop_tgt:`IMPORTED` targets:
#
# ``Orbbec::OrbbecSDK``
#  Defined if the system has Orbbec SDK.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module sets the following variables:
#
# ::
#
#   OrbbecSDK_FOUND               True in case Orbbec SDK is found, otherwise false
#   OrbbecSDK_ROOT                Path to the root of found Orbbec SDK installation
#
# Example Usage
# ^^^^^^^^^^^^^
#
# ::
#
#     find_package(OrbbecSDK REQUIRED)
#
#     add_executable(foo foo.cc)
#     target_link_libraries(foo Orbbec::OrbbecSDK)
#
# License
# ^^^^^^^
#
# Copyright (c) 2023 Tsukasa SUGIURA
# Distributed under the MIT License.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

find_path(OrbbecSDK_INCLUDE_DIR
  NAMES
    libobsensor/ObSensor.h
  HINTS
    $ENV{OrbbecSDK_ROOT}/include
    /usr/include
  PATHS
    "$ENV{PROGRAMW6432}/OrbbecSDK/SDK/"
  PATH_SUFFIXES
    include
)

find_library(OrbbecSDK_LIBRARY
  NAMES
    OrbbecSDK.lib
    libOrbbecSDK.so
  HINTS
    $ENV{OrbbecSDK_ROOT}/lib
    /usr/lib
  PATHS
    "$ENV{PROGRAMW6432}/OrbbecSDK/SDK/"
  PATH_SUFFIXES
    lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  OrbbecSDK DEFAULT_MSG
  OrbbecSDK_LIBRARY OrbbecSDK_INCLUDE_DIR
)

if(OrbbecSDK_FOUND)
  add_library(Orbbec::OrbbecSDK SHARED IMPORTED)
  set_target_properties(Orbbec::OrbbecSDK PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${OrbbecSDK_INCLUDE_DIR}")

  set_property(TARGET Orbbec::OrbbecSDK APPEND PROPERTY IMPORTED_CONFIGURATIONS "RELEASE")
  set_target_properties(Orbbec::OrbbecSDK PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX")
  if(WIN32)
    set_target_properties(Orbbec::OrbbecSDK PROPERTIES IMPORTED_IMPLIB_RELEASE "${OrbbecSDK_LIBRARY}")
  else()
    set_target_properties(Orbbec::OrbbecSDK PROPERTIES IMPORTED_LOCATION_RELEASE "${OrbbecSDK_LIBRARY}")
  endif()

  set_property(TARGET Orbbec::OrbbecSDK APPEND PROPERTY IMPORTED_CONFIGURATIONS "DEBUG")
  set_target_properties(Orbbec::OrbbecSDK PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX")
  if(WIN32)
    set_target_properties(Orbbec::OrbbecSDK PROPERTIES IMPORTED_IMPLIB_DEBUG "${OrbbecSDK_LIBRARY}")
  else()
    set_target_properties(Orbbec::OrbbecSDK PROPERTIES IMPORTED_LOCATION_DEBUG "${OrbbecSDK_LIBRARY}")
  endif()

  get_filename_component(OrbbecSDK_ROOT "${OrbbecSDK_INCLUDE_DIR}" PATH)
endif()