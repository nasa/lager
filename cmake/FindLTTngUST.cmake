#.rst:
# FindLTTngUST
# ------------
#
# This module finds the `LTTng-UST <http://lttng.org/>`__ library.
#
# Imported target
# ^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` target:
#
# ``LTTng::UST``
#   The LTTng-UST library, if found
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module sets the following
#
# ``LTTngUST_FOUND``
#   ``TRUE`` if system has LTTng-UST
# ``LTTngUST_INCLUDE_DIRS``
#   The LTTng-UST include directories
# ``LTTngUST_LIBRARIES``
#   The libraries needed to use LTTng-UST
# ``LTTngUST_VERSION_STRING``
#   The LTTng-UST version
# ``LTTngUST_HAS_TRACEF``
#   ``TRUE`` if the ``tracef()`` API is available in the system's LTTng-UST
# ``LTTngUST_HAS_TRACELOG``
#   ``TRUE`` if the ``tracelog()`` API is available in the system's LTTng-UST

#=============================================================================
# Copyright 2016 Kitware, Inc.
# Copyright 2016 Philippe Proulx <pproulx@efficios.com>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

find_path(LTTngUST_INCLUDE_DIRS NAMES lttng/tracepoint.h)
# Must also check for the path of generated header files since out-of-tree
# build is a possibility (Yocto).
find_path(LTTngUST_INCLUDE_DIRS_GENERATED NAMES lttng/ust-config.h)
find_library(LTTngUST_LIBRARIES NAMES lttng-ust)

if(LTTngUST_INCLUDE_DIRS AND LTTngUST_INCLUDE_DIRS_GENERATED AND LTTngUST_LIBRARIES)
  # find tracef() and tracelog() support
  set(LTTngUST_HAS_TRACEF 0)
  set(LTTngUST_HAS_TRACELOG 0)

  if(EXISTS "${LTTngUST_INCLUDE_DIRS}/lttng/tracef.h")
    set(LTTngUST_HAS_TRACEF TRUE)
  endif()

  if(EXISTS "${LTTngUST_INCLUDE_DIRS}/lttng/tracelog.h")
    set(LTTngUST_HAS_TRACELOG TRUE)
  endif()

  # get version
  set(lttngust_version_file "${LTTngUST_INCLUDE_DIRS_GENERATED}/lttng/ust-version.h")

  if(EXISTS "${lttngust_version_file}")
    file(STRINGS "${lttngust_version_file}" lttngust_version_major_string
         REGEX "^[\t ]*#define[\t ]+LTTNG_UST_MAJOR_VERSION[\t ]+[0-9]+[\t ]*$")
    file(STRINGS "${lttngust_version_file}" lttngust_version_minor_string
         REGEX "^[\t ]*#define[\t ]+LTTNG_UST_MINOR_VERSION[\t ]+[0-9]+[\t ]*$")
    file(STRINGS "${lttngust_version_file}" lttngust_version_patch_string
         REGEX "^[\t ]*#define[\t ]+LTTNG_UST_PATCHLEVEL_VERSION[\t ]+[0-9]+[\t ]*$")
    string(REGEX REPLACE ".*([0-9]+).*" "\\1"
           lttngust_v_major "${lttngust_version_major_string}")
    string(REGEX REPLACE ".*([0-9]+).*" "\\1"
           lttngust_v_minor "${lttngust_version_minor_string}")
    string(REGEX REPLACE ".*([0-9]+).*" "\\1"
           lttngust_v_patch "${lttngust_version_patch_string}")
    set(LTTngUST_VERSION_STRING
        "${lttngust_v_major}.${lttngust_v_minor}.${lttngust_v_patch}")
    unset(lttngust_version_major_string)
    unset(lttngust_version_minor_string)
    unset(lttngust_version_patch_string)
    unset(lttngust_v_major)
    unset(lttngust_v_minor)
    unset(lttngust_v_patch)
  else()
    message(FATAL_ERROR "Missing version header")
  endif()

  unset(lttngust_version_file)

  if(NOT TARGET LTTng::UST)
    add_library(LTTng::UST UNKNOWN IMPORTED)
    set_target_properties(LTTng::UST PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${LTTngUST_INCLUDE_DIRS};${LTTngUST_INCLUDE_DIRS_GENERATED}"
      INTERFACE_LINK_LIBRARIES ${CMAKE_DL_LIBS}
      IMPORTED_LINK_INTERFACE_LANGUAGES "C"
      IMPORTED_LOCATION "${LTTngUST_LIBRARIES}")
  endif()

  # add libdl to required libraries
  set(LTTngUST_LIBRARIES ${LTTngUST_LIBRARIES} ${CMAKE_DL_LIBS})
  find_program(LTTNG_GEN_TP lttng-gen-tp)
  if (NOT LTTNG_GEN_TP)
    message(FATAL_ERROR "lttng-gen-tp not found")
  endif()
endif()

# handle the QUIETLY and REQUIRED arguments and set LTTngUST_FOUND to
# TRUE if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LTTngUST FOUND_VAR LTTngUST_FOUND
  REQUIRED_VARS LTTngUST_LIBRARIES
  LTTngUST_INCLUDE_DIRS
  VERSION_VAR LTTngUST_VERSION_STRING)
mark_as_advanced(LTTngUST_LIBRARIES LTTngUST_INCLUDE_DIRS)

# generates the tracepoint files given a list of template files
# copycatted from protobuf's generate cpp function
function(lttngust_gen srcs hdrs)
  cmake_parse_arguments(lttngust "" "" "" ${ARGN})
  set(TP_FILES "${lttngust_UNPARSED_ARGUMENTS}")
  if (NOT TP_FILES)
    message(SEND_ERROR "Error: lttngust_gen() called without any tp files")
    return()
  endif()

  set(${srcs})
  set(${hdrs})
  foreach(FIL ${TP_FILES})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(FIL_WE ${FIL} NAME_WE)
    set(_gen_src "${FIL_WE}.tp.c")
    set(_gen_hdr "${FIL_WE}.tp.h")
    list(APPEND ${srcs} "${_gen_src}")
    list(APPEND ${hdrs} "${_gen_hdr}")

    add_custom_command(
      OUTPUT "${_gen_src}" "${_gen_hdr}"
      COMMAND ${LTTNG_GEN_TP} 
      ARGS ${ABS_FIL} "-o${_gen_src}" "-o${_gen_hdr}"
      DEPENDS ${ABS_FIL} ${LTTNG_GEN_TP}
      COMMENT "Running lttng generator on ${FIL}"
    )

  endforeach()

  set(${srcs} "${${srcs}}" PARENT_SCOPE)
  set(${hdrs} "${${hdrs}}" PARENT_SCOPE)
  
endfunction(lttngust_gen)