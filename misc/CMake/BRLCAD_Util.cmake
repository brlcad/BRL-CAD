#               B R L C A D _ U T I L . C M A K E
# BRL-CAD
#
# Copyright (c) 2011-2016 United States Government as represented by
# the U.S. Army Research Laboratory.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following
# disclaimer in the documentation and/or other materials provided
# with the distribution.
#
# 3. The name of the author may not be used to endorse or promote
# products derived from this software without specific prior written
# permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
###

#-----------------------------------------------------------------------------
# We want to support a "distclean" build target that will clear all
# CMake-generated files from a source directory in the case of an
# in-source-dir configuration.  Not recommended, but we'll try to
# recover if it happens.
define_property(GLOBAL PROPERTY CMAKE_DISTCLEAN_TARGET_LIST BRIEF_DOCS "All CMake generated files" FULL_DOCS "List of all files generated by CMake")
function(distclean)
  foreach(item ${ARGN})
    set_property(GLOBAL APPEND PROPERTY CMAKE_DISTCLEAN_TARGET_LIST "${item}")
  endforeach(item ${ARGN})
endfunction(distclean)

#-----------------------------------------------------------------------------
# Use a variation on Fraser's approach for capturing command line args from
# http://stackoverflow.com/questions/10205986/how-to-capture-cmake-command-line-arguments
# to log what variables have been passed in from the user via -D arguments - haven't
# found a variable that saves the original ARGV list except for those defined in
# -P script mode, which doesn't help here.
function(record_cmdline_args)
  get_cmake_property(VARS VARIABLES)
  foreach(VAR ${VARS})
    get_property(VAR_HELPSTRING CACHE ${VAR} PROPERTY HELPSTRING)
    # Rather than look for "No help, variable specified on the command line."
    # exactly, match a slightly more robust subset...
    string(TOLOWER "${VAR_HELPSTRING}" VAR_HELPSTRING)
    if("${VAR_HELPSTRING}" MATCHES "specified on the command line")
      get_property(VAR_TYPE CACHE ${VAR} PROPERTY TYPE)
      if(NOT VAR_TYPE STREQUAL "UNINITIALIZED")
	set(VAR "${VAR}:${VAR_TYPE}")
      endif(NOT VAR_TYPE STREQUAL "UNINITIALIZED")
      set(CMAKE_ARGS "${CMAKE_ARGS} -D${VAR}=${${VAR}}")
    endif("${VAR_HELPSTRING}" MATCHES "specified on the command line")
  endforeach(VAR ${VARS})
  file(APPEND "${CMAKE_BINARY_DIR}/CMakeFiles/CMakeOutput.log" "${CMAKE_COMMAND} \"${CMAKE_SOURCE_DIR}\" ${CMAKE_ARGS}\n")
endfunction(record_cmdline_args)

#---------------------------------------------------------------------
# Wrap the default message() function to also append ALL messages to a
# CMakeOutput.log file in addition to usual console printing.
# Note - only do this after calling project, since this override seems to do
# unexpected things to the messages returned by that command

function(message)

  # bleh, don't know a clean+safe way to avoid string comparing the
  # optional arg, so we extract it and test.
  list(GET ARGV 0 MessageType)

  if (MessageType STREQUAL FATAL_ERROR OR MessageType STREQUAL SEND_ERROR OR MessageType STREQUAL WARNING OR MessageType STREQUAL AUTHOR_WARNING OR MessageType STREQUAL STATUS)
    list(REMOVE_AT ARGV 0)
    _message(${MessageType} "${ARGV}")
    file(APPEND "${BRLCAD_BINARY_DIR}/CMakeFiles/CMakeOutput.log" "${MessageType}: ${ARGV}\n")
  else (MessageType STREQUAL FATAL_ERROR OR MessageType STREQUAL SEND_ERROR OR MessageType STREQUAL WARNING OR MessageType STREQUAL AUTHOR_WARNING OR MessageType STREQUAL STATUS)
    _message("${ARGV}")
    file(APPEND "${BRLCAD_BINARY_DIR}/CMakeFiles/CMakeOutput.log" "${ARGV}\n")
  endif (MessageType STREQUAL FATAL_ERROR OR MessageType STREQUAL SEND_ERROR OR MessageType STREQUAL WARNING OR MessageType STREQUAL AUTHOR_WARNING OR MessageType STREQUAL STATUS)

  # ~10% slower alternative that avoids adding '--' to STATUS messages
  # execute_process(COMMAND ${CMAKE_COMMAND} -E echo "${ARGV}")

endfunction(message)

#-----------------------------------------------------------------------------
# Pretty-printing macro that generates a box around a string and prints the
# resulting message.
function(BOX_PRINT input_string border_string)
  string(LENGTH ${input_string} MESSAGE_LENGTH)
  string(LENGTH ${border_string} SEPARATOR_STRING_LENGTH)
  while(${MESSAGE_LENGTH} GREATER ${SEPARATOR_STRING_LENGTH})
    set(SEPARATOR_STRING "${SEPARATOR_STRING}${border_string}")
    string(LENGTH ${SEPARATOR_STRING} SEPARATOR_STRING_LENGTH)
  endwhile(${MESSAGE_LENGTH} GREATER ${SEPARATOR_STRING_LENGTH})
  message("${SEPARATOR_STRING}")
  message("${input_string}")
  message("${SEPARATOR_STRING}")
endfunction()

#-----------------------------------------------------------------------------
# We need a way to tell whether one path is a subpath of another path without
# relying on regular expressions, since file paths may have characters in them
# that will trigger regex matching behavior when we don't want it.  (To test,
# for example, use a build directory name of build++)
#
# Sets ${result_var} to 1 if the candidate subpath is actually a subpath of
# the supplied "full" path, otherwise sets it to 0.
#
# The routine below does the check without using regex matching, in order to
# handle path names that contain characters that would be interpreted as active
# in a regex string.
function(IS_SUBPATH candidate_subpath full_path result_var)

  # Just assume it isn't until we prove it is
  set(${result_var} 0 PARENT_SCOPE)

  # get the CMake form of the path so we have something consistent to work on
  file(TO_CMAKE_PATH "${full_path}" c_full_path)
  file(TO_CMAKE_PATH "${candidate_subpath}" c_candidate_subpath)

  # check the string lengths - if the "subpath" is longer than the full path,
  # there's not point in going further
  string(LENGTH "${c_full_path}" FULL_LENGTH)
  string(LENGTH "${c_candidate_subpath}" SUB_LENGTH)
  if("${SUB_LENGTH}" GREATER "${FULL_LENGTH}")
    return()
  endif("${SUB_LENGTH}" GREATER "${FULL_LENGTH}")

  # OK, maybe it's a subpath - time to actually check
  string(SUBSTRING "${c_full_path}" 0 ${SUB_LENGTH} c_full_subpath)
  if(NOT "${c_full_subpath}" STREQUAL "${c_candidate_subpath}")
    return()
  endif(NOT "${c_full_subpath}" STREQUAL "${c_candidate_subpath}")

  # If we get here, it's a subpath
  set(${result_var} 1 PARENT_SCOPE)

endfunction(IS_SUBPATH)

#---------------------------------------------------------------------
# Define a macro for building lists of files.  Distcheck needs to know
# what files are "supposed" to be present in order to make sure the
# source tree is clean prior to building a distribution tarball, hence
# this macro stores its results in files and not variables  It's a
# no-op in a SUBBUILD.
#
# For this macro to work correctly, it is important that any target
# definitions or explicit calls to this macro supply relative paths for
# files present in the source tree.  Generated files fed into
# compilation targets are not one of the things that should be in lists
# generated by this macro, and the only way to reliably recognize them
# is to reject files specified using their full path.  Such files must
# use their full path in the build logic in order for out-of-src-dir
# builds to function, so as long as no full paths are used for files
# actually IN the source tree this method is reliable.  The macro will
# try to catch improperly specified files, but if the build directory
# and the source directory are one and the same this will not be
# possible.

define_property(GLOBAL PROPERTY CMAKE_IGNORE_FILES BRIEF_DOCS "distcheck ignore files" FULL_DOCS "List of files known to CMake")
define_property(GLOBAL PROPERTY CMAKE_IGNORE_DIRS BRIEF_DOCS "distcheck ignore dirs" FULL_DOCS "List of directories marked as fully known to CMake")

# If the build directory is not the same as the source directory, we can
# enforce the convention that only generated files be specified with their full
# name.
function(CHECK_SOURCE_DIR_FULLPATH ITEM_PATH)

  # We can only do this if BINARY_DIR != SOURCE_DIR
  if(NOT "${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    return()
  endif(NOT "${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")

  # If it's a full path in the binary dir, it's fine
  IS_SUBPATH("${CMAKE_BINARY_DIR}" "${ITEM_PATH}" SUBPATH_TEST)
  if(NOT"${SUBPATH_TEST}" STREQUAL "0")
    return()
  endif(NOT "${SUBPATH_TEST}" STREQUAL "0")

  # If it's a full path in the source dir, it's fatal
  IS_SUBPATH("${CMAKE_SOURCE_DIR}" "${ITEM_PATH}" SUBPATH_TEST)
  if("${SUBPATH_TEST}" STREQUAL "1")
    message(FATAL_ERROR "${ITEM} is listed in \"${CMAKE_CURRENT_SOURCE_DIR}\" using its absolute path.  Please specify the location of this file using a relative path.")
  endif("${SUBPATH_TEST}" STREQUAL "1")

endfunction(CHECK_SOURCE_DIR_FULLPATH ITEM_PATH)

function(CMFILE ITEM)

  get_filename_component(ITEM_PATH "${ITEM}" PATH)
  if(NOT "${ITEM_PATH}" STREQUAL "")
    # The hard case - path specified, need some validation.

    # If this becomes a problem with the third party build
    # systems at some point in the future, we may have to
    # exclude src/other paths from this check.
    CHECK_SOURCE_DIR_FULLPATH("${ITEM_PATH}")

    # Ignore files specified using full paths, since they
    # should be generated files and are not part of the
    # source code repository.
    get_filename_component(ITEM_ABS_PATH "${ITEM_PATH}" ABSOLUTE)
    if("${ITEM_PATH}" STREQUAL "${ITEM_ABS_PATH}")
      return()
    endif("${ITEM_PATH}" STREQUAL "${ITEM_ABS_PATH}")
  endif(NOT "${ITEM_PATH}" STREQUAL "")

  # Handle fatal cases
  if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${ITEM}")
    message(FATAL_ERROR "Trying to ignore directory: ${CMAKE_CURRENT_SOURCE_DIR}/${ITEM}")
  endif(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${ITEM}")
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${ITEM}")
    message(FATAL_ERROR "Attempting to ignore non-existent file ${ITEM}, in directory \"${CMAKE_CURRENT_SOURCE_DIR}\"")
  endif(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${ITEM}")

  # We're good - log it
  set_property(GLOBAL APPEND PROPERTY CMAKE_IGNORE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${ITEM}")

  # Track the directories as well
  get_property(IGNORE_PATHS GLOBAL PROPERTY CMAKE_IGNORE_DIRS)
  get_filename_component(NPATH "${CMAKE_CURRENT_SOURCE_DIR}/${ITEM}" PATH)
  while(NOT "${NPATH}" STREQUAL "${CMAKE_SOURCE_DIR}")
    list(APPEND IGNORE_PATHS "${NPATH}")
    get_filename_component(NPATH "${NPATH}" PATH)
  endwhile(NOT "${NPATH}" STREQUAL "${CMAKE_SOURCE_DIR}")
  list(REMOVE_DUPLICATES IGNORE_PATHS)
  set_property(GLOBAL PROPERTY CMAKE_IGNORE_DIRS "${IGNORE_PATHS}")

endfunction(CMFILE ITEM)

function(CMAKEFILES)
  if(NOT BRLCAD_IS_SUBBUILD)

    # CMake flags to add_library/add_executable aren't going to be valid filenames - just
    # yank them up front.
    set(ITEMS ${ARGN})
    list(REMOVE_ITEM ITEMS SHARED STATIC OBJECT WIN32 UNKNOWN IMPORTED MODULE INTERFACE EXCLUDE_FROM_ALL)
    if(ITEMS)
      list(FILTER ITEMS EXCLUDE REGEX ".*TARGET_OBJECTS.*")
    endif(ITEMS)

    foreach(ITEM ${ITEMS})
      CMFILE("${ITEM}")
    endforeach(ITEM ${ITEMS})

  endif(NOT BRLCAD_IS_SUBBUILD)
endfunction(CMAKEFILES FILESLIST)

# Routine to tell distcheck to ignore a series of items in a directory.  Items may themselves
# be directories.  Primarily useful when working with src/other dist lists.
function(CMAKEFILES_IN_DIR filestoignore targetdir)
  if(NOT BRLCAD_IS_SUBBUILD)
    set(CMAKE_IGNORE_LIST "")
    foreach(filepath ${${filestoignore}})
      set(CMAKE_IGNORE_LIST ${CMAKE_IGNORE_LIST} "${targetdir}/${filepath}")
    endforeach(filepath ${filestoignore})
    CMAKEFILES(${CMAKE_IGNORE_LIST})
  endif(NOT BRLCAD_IS_SUBBUILD)
endfunction(CMAKEFILES_IN_DIR)

#-----------------------------------------------------------------------------
# It is sometimes convenient to be able to supply both a filename and a
# variable name containing a list of files to a single macro.
# This routine handles both forms of input - separate variables are
# used to indicate which variable names are supposed to contain the
# initial list contents and the full path version of that list.  Thus,
# macros using the normalize macro get the list in a known variable and
# can use it reliably, regardless of whether inlist contained the actual
# list contents or a variable.
macro(NORMALIZE_FILE_LIST inlist targetvar fullpath_targetvar)

  # First, figure out whether we have list contents or a list name
  set(havevarname 0)
  foreach(maybefilename ${inlist})
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${maybefilename}")
      set(havevarname 1)
    endif(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${maybefilename}")
  endforeach(maybefilename ${${targetvar}})

  # Put the list contents in the targetvar variable and
  # generate a target name.
  if(NOT havevarname)

    set(${targetvar} "${inlist}")

    # Initial clean-up
    string(REGEX REPLACE " " "_" targetstr "${inlist}")
    string(REGEX REPLACE "/" "_" targetstr "${targetstr}")
    string(REGEX REPLACE "\\." "_" targetstr "${targetstr}")

    # For situations like file copying, where we sometimes need to autogenerate
    # target names, it is important to make sure we can avoid generating absurdly
    # long names.  To do this, we run candidate names through a length filter
    # and use their MD5 hash if they are longer than 30 characters.
    # It's cryptic but the odds are very good the result will be a unique
    # target name and the string will be short enough, which is what we need.
    string(LENGTH "${targetstr}" STRLEN)
    if ("${STRLEN}" GREATER 30)
      string(MD5 targetname "${targetstr}")
    else ("${STRLEN}" GREATER 30)
      set(targetname "${targetstr}")
    endif ("${STRLEN}" GREATER 30)

  else(NOT havevarname)

    set(${targetvar} "${${inlist}}")
    set(targetname "${inlist}")

  endif(NOT havevarname)

  # Mark the inputs as files to ignore in distcheck
  CMAKEFILES(${${targetvar}})

  # For some uses, we need the contents of the input list
  # with full paths.  Generate a list that we're sure has
  # full paths, and return that to the second variable.
  set(${fullpath_targetvar} "")
  foreach(filename ${${targetvar}})
    get_filename_component(file_fullpath "${filename}" ABSOLUTE)
    set(${fullpath_targetvar} ${${fullpath_targetvar}} "${file_fullpath}")
  endforeach(filename ${${targetvar}})

  # Some macros will also want a valid build target name
  # based on the input - if a third input parameter has
  # been supplied, return the target name using it.
  if(NOT "${ARGV3}" STREQUAL "")
    set(${ARGV3} "${targetname}")
  endif(NOT "${ARGV3}" STREQUAL "")

endmacro(NORMALIZE_FILE_LIST)

#-----------------------------------------------------------------------------
# It is sometimes necessary for build logic to be aware of all instances
# of a certain category of target that have been defined for a particular
# build directory - for example, the pkgIndex.tcl generation targets need
# to ensure that all data copying targets have done their work before they
# generate their indexes.  To support this, macros are define that allow
# globally available lists to be defined, maintained and accessed.  We use
# this approach instead of directory properties because CMake's documentation
# seems to indicate that directory properties also apply to subdirectories,
# and we want these lists to be associated with one and only one directory.

macro(BRLCAD_ADD_DIR_LIST_ENTRY list_name dir_in list_entry)
  string(REGEX REPLACE "/" "_" currdir_str ${dir_in})
  string(TOUPPER "${currdir_str}" currdir_str)
  get_property(${list_name}_${currdir_str} GLOBAL PROPERTY DATA_TARGETS_${currdir_str})
  if(NOT ${list_name}_${currdir_str})
    define_property(GLOBAL PROPERTY CMAKE_LIBRARY_TARGET_LIST BRIEF_DOCS "${list_name}" FULL_DOCS "${list_name} for directory ${dir_in}")
  endif(NOT ${list_name}_${currdir_str})
  set_property(GLOBAL APPEND PROPERTY ${list_name}_${currdir_str} ${list_entry})
endmacro(BRLCAD_ADD_DIR_LIST_ENTRY)

macro(BRLCAD_GET_DIR_LIST_CONTENTS list_name dir_in outvar)
  string(REGEX REPLACE "/" "_" currdir_str ${dir_in})
  string(TOUPPER "${currdir_str}" currdir_str)
  get_property(${list_name}_${currdir_str} GLOBAL PROPERTY ${list_name}_${currdir_str})
  set(${outvar} "${DATA_TARGETS_${currdir_str}}")
endmacro(BRLCAD_GET_DIR_LIST_CONTENTS)


#-----------------------------------------------------------------------------
# Determine whether a list of source files contains all C, all C++, or
# mixed source types.
function(SRCS_LANG sourceslist resultvar targetname)
  # Check whether we have a mixed C/C++ library or just a single language.
  # If the former, different compilation flag management is needed.
  set(has_C 0)
  set(has_CXX 0)
  foreach(srcfile ${sourceslist})
    get_property(file_language SOURCE ${srcfile} PROPERTY LANGUAGE)
    if(NOT file_language)
      get_filename_component(srcfile_ext ${srcfile} EXT)
      if(${srcfile_ext} MATCHES ".cxx$" OR ${srcfile_ext} MATCHES ".cpp$" OR ${srcfile_ext} MATCHES ".cc$")
        set(has_CXX 1)
        set(file_language CXX)
      elseif(${srcfile_ext} STREQUAL ".c")
        set(has_C 1)
        set(file_language C)
      endif(${srcfile_ext} MATCHES ".cxx$" OR ${srcfile_ext} MATCHES ".cpp$" OR ${srcfile_ext} MATCHES ".cc$")
    endif(NOT file_language)
    if(NOT file_language)
      message("WARNING - file ${srcfile} listed in the ${targetname} sources list does not appear to be a C or C++ file.")
    endif(NOT file_language)
  endforeach(srcfile ${sourceslist})
  set(${resultvar} "UNKNOWN" PARENT_SCOPE)
  if(has_C AND has_CXX)
    set(${resultvar} "MIXED" PARENT_SCOPE)
  elseif(has_C AND NOT has_CXX)
    set(${resultvar} "C" PARENT_SCOPE)
  elseif(NOT has_C AND has_CXX)
    set(${resultvar} "CXX" PARENT_SCOPE)
  endif(has_C AND has_CXX)
endfunction(SRCS_LANG)

#---------------------------------------------------------------------------
# Add dependencies to a target, but only if they are defined as targets in
# CMake
function(ADD_TARGET_DEPS tname)
  if(TARGET ${tname})
    foreach(target ${ARGN})
      if(TARGET ${target})
	add_dependencies(${tname} ${target})
      endif(TARGET ${target})
    endforeach(target ${ARGN})
  endif(TARGET ${tname})
endfunction(ADD_TARGET_DEPS tname)

#---------------------------------------------------------------------------
# Write out an execution script to run commands with the necessary
# variables set to allow execution in the build directory, even if
# there are installed libraries present in the final installation
# directory.
function(generate_cmd_script cmd_exe script_file)

  cmake_parse_arguments(GCS "" "OLOG;ELOG" "CARGS" ${ARGN})

  # Initialize file 
  file(WRITE "${script_file}" "# Script to run ${cmd_exe}\n")

  # Handle multiconfig (must be run-time determination for Visual Studio and XCode)
  # TODO - logic writing this trick needs to become some sort of standard routine...
  file(APPEND "${script_file}" "if(EXISTS \"${CMAKE_BINARY_DIR}/CMakeTmp/CURRENT_PATH/Release\")\n")
  file(APPEND "${script_file}" "  set(CBDIR \"${CMAKE_BINARY_DIR}/Release\")\n")
  file(APPEND "${script_file}" "elseif(EXISTS \"${CMAKE_BINARY_DIR}/CMakeTmp/CURRENT_PATH/Debug\")\n")
  file(APPEND "${script_file}" "  set(CBDIR \"${CMAKE_BINARY_DIR}/Debug\")\n")
  file(APPEND "${script_file}" "else(EXISTS \"${CMAKE_BINARY_DIR}/CMakeTmp/CURRENT_PATH/Release\")\n")
  file(APPEND "${script_file}" "  set(CBDIR \"${CMAKE_BINARY_DIR}\")\n")
  file(APPEND "${script_file}" "endif(EXISTS \"${CMAKE_BINARY_DIR}/CMakeTmp/CURRENT_PATH/Release\")\n")

  # BRLCAD_ROOT is the hammer that makes certain we are running
  # things found in the build directory
  file(APPEND "${script_file}" "set(ENV{BRLCAD_ROOT} \"\${CBDIR}\")\n")

  # Substitute in the correct binary path anywhere it is needed in the args
  file(APPEND "${script_file}" "string(REPLACE \"CURRENT_BUILD_DIR\" \"\${CBDIR}\" FIXED_CMD_ARGS \"${GCS_CARGS}\")\n")

  # Use the CMake executable to figure out if we need an extension
  get_filename_component(EXE_EXT "${CMAKE_COMMAND}" EXT)

  # Write the actual cmake command to run the process
  file(APPEND "${script_file}" "execute_process(COMMAND \"\${CBDIR}/${BIN_DIR}/${cmd_exe}${EXE_EXT}\" \${FIXED_CMD_ARGS} RESULT_VARIABLE CR OUTPUT_VARIABLE CO ERROR_VARIABLE CE)\n")

  # Log the outputs, if we are supposed to do that
  if(GCS_OLOG)
    file(APPEND "${script_file}" "file(APPEND \"${GCS_OLOG}\" \"\${CO}\")\n")
  endif(GCS_OLOG)
  if(GCS_ELOG)
    file(APPEND "${script_file}" "file(APPEND \"${GCS_ELOG}\" \"\${CE}\")\n")
  endif(GCS_ELOG)

  # Fail the command if the result was non-zero
  file(APPEND "${script_file}" "if(CR)\n")
  file(APPEND "${script_file}" "  message(FATAL_ERROR \"\${CBDIR}/${BIN_DIR}/${cmd_exe}${EXE_EXT} failure: \${CR}\\n\${CO}\\n\${CE}\")\n")
  file(APPEND "${script_file}" "endif(CR)\n")

  # If we are doing distclean, let CMake know to remove the script and log files
  if(COMMAND distclean)
    set(distclean_files "${script_file}" "${GCS_OLOG}" "${GCS_ELOG}")
    list(REMOVE_DUPLICATES distclean_files)
    distclean(${distclean_files})
  endif(COMMAND distclean)

endfunction(generate_cmd_script)

#---------------------------------------------------------------------------
# Set variables reporting time of configuration.  Sets CONFIG_DATE and
# CONFIG_DATESTAMP in parent scope.
#
# Unfortunately, CMake doesn't give you variables with current day,
# month, etc.  There are several possible approaches to this, but most
# (e.g. the date command) are not cross platform. We build a small C
# program which supplies the needed info.
function(set_config_time)

  # We don't want any parent C flags here - hopefully, the below code will
  # "just work" in any environment we are about.  The gnu89 standard doesn't
  # like the %z print specifier, but unless/until we hit a compiler that really
  # can't deal with it don't worry about it.
  set(CMAKE_C_FLAGS "")

  set(rfc2822_src "
/* RFC2822 timestamp and individual day, month and year files */

#include <time.h>
#include <stdio.h>
#include <string.h>

int main(int argc, const char **argv) {
   time_t t = time(NULL);
   struct tm *currenttime = localtime(&t);
   if (argc <= 1) {
      char rfc2822[200];
      strftime(rfc2822, sizeof(rfc2822), \"%a, %d %b %Y %H:%M:%S %z\", currenttime);
      printf(\"%s\", rfc2822);
      return 0;
   }
   if (strncmp(argv[1], \"day\", 4) == 0) {
      printf(\"%02d\", currenttime->tm_mday);
   }
   if (strncmp(argv[1], \"month\", 5) == 0) {
      printf(\"%02d\", currenttime->tm_mon + 1);
   }
   if (strncmp(argv[1], \"year\", 4) == 0) {
      printf(\"%d\", currenttime->tm_year + 1900);
   }
   return 0;
}")

  # Build the code so we can run it
  file(WRITE "${CMAKE_BINARY_DIR}/CMakeTmp/rfc2822.c" "${rfc2822_src}")
  try_compile(rfc2822_build "${CMAKE_BINARY_DIR}/CMakeTmp"
    SOURCES "${CMAKE_BINARY_DIR}/CMakeTmp/rfc2822.c"
    OUTPUT_VARIABLE RFC2822_BUILD_INFO
    COPY_FILE "${CMAKE_BINARY_DIR}/CMakeTmp/rfc2822${EXE_EXT}")
  if(NOT rfc2822_build)
    message(FATAL_ERROR "Could not build rfc2822 timestamp pretty-printing utility: ${RFC2822_BUILD_INFO}")
  endif(NOT rfc2822_build)
  file(REMOVE "${CMAKE_BINARY_DIR}/CMakeTmp/rfc2822.c")

  # Build up and set CONFIG_DATE
  execute_process(COMMAND "${CMAKE_BINARY_DIR}/CMakeTmp/rfc2822" day OUTPUT_VARIABLE CONFIG_DAY)
  string(STRIP ${CONFIG_DAY} CONFIG_DAY)
  execute_process(COMMAND "${CMAKE_BINARY_DIR}/CMakeTmp/rfc2822" month OUTPUT_VARIABLE CONFIG_MONTH)
  string(STRIP ${CONFIG_MONTH} CONFIG_MONTH)
  execute_process(COMMAND "${CMAKE_BINARY_DIR}/CMakeTmp/rfc2822" year OUTPUT_VARIABLE CONFIG_YEAR)
  string(STRIP ${CONFIG_YEAR} CONFIG_YEAR)
  set(CONFIG_DATE "${CONFIG_YEAR}${CONFIG_MONTH}${CONFIG_DAY}" PARENT_SCOPE)

  # Set DATESTAMP
  execute_process(COMMAND "${CMAKE_BINARY_DIR}/CMakeTmp/rfc2822" OUTPUT_VARIABLE RFC2822_STRING)
  string(STRIP ${RFC2822_STRING} RFC2822_STRING)
  set(CONFIG_DATESTAMP "${RFC2822_STRING}" PARENT_SCOPE)

  # Cleanup
  file(REMOVE "${CMAKE_BINARY_DIR}/CMakeTmp/rfc2822")

endfunction(set_config_time)

#---------------------------------------------------------------------------
# Code for timing configuration and building of BRL-CAD.  These executables
# are used to define build targets for cross-platform reporting.  Run after
# set_config_time.
function(generate_timer_exes)

  #########################################################################
  # Print a basic time stamp

  set(printtimestamp_src "
#define _CRT_SECURE_NO_WARNINGS 1

#include <time.h>
#include <stdio.h>

int main(int argc, const char **argv)
{
  time_t t = time(NULL);
  struct tm *currenttime = localtime(&t);
  if (argc < 2) return 1;
  printf(\"\\n%s%s\\n\", argv[1], asctime(currenttime));
  return 0;
}
")

  file(WRITE "${CMAKE_BINARY_DIR}/CMakeTmp/printtimestamp.c" "${printtimestamp_src}")
  try_compile(pts_build ${CMAKE_BINARY_DIR}/CMakeTmp
    "${CMAKE_BINARY_DIR}/CMakeTmp/printtimestamp.c"
    COPY_FILE ${CMAKE_BINARY_DIR}/CMakeTmp/pts${EXE_EXT})
  if(NOT pts_build)
    message(FATAL_ERROR "Could not build timestamp pretty-printing utility")
  endif(NOT pts_build)

  #########################################################################
  # We need some way to know at the end of the build when we kicked things
  # off (both configuration and the build itself).  Create a simple program
  # that can stash that info in a file

  set(sstamp_src "
#define _CRT_SECURE_NO_WARNINGS 1

#include <time.h>
#include <stdio.h>

int main(int argc, const char **argv) {
  FILE *outfp = NULL;
  time_t t = time(NULL);
  if (argc < 2) return 1;
  outfp = fopen(argv[1], \"w\");
  fprintf(outfp, \"%ld\", (long)t);
  fclose(outfp);
  return 0;
}
")

  # Build the code so we can run it
  file(WRITE "${CMAKE_BINARY_DIR}/CMakeTmp/sstamp.c" "${sstamp_src}")
  try_compile(sstamp_build "${CMAKE_BINARY_DIR}/CMakeTmp"
    SOURCES "${CMAKE_BINARY_DIR}/CMakeTmp/sstamp.c"
    OUTPUT_VARIABLE SSTAMP_BUILD_INFO
    COPY_FILE "${CMAKE_BINARY_DIR}/CMakeTmp/sstamp${EXE_EXT}")
  if(NOT sstamp_build)
    message(FATAL_ERROR "Could not build second timestamping utility: ${SSTAMP_BUILD_INFO}")
  endif(NOT sstamp_build)
  file(REMOVE "${CMAKE_BINARY_DIR}/CMakeTmp/sstamp.c")
  if(COMMAND distclean)
    distclean("${CMAKE_BINARY_DIR}/CMakeTmp/sstamp")
  endif(COMMAND distclean)

  #########################################################################
  # To report at the end what the actual deltas are, we need to read in the
  # time stamps from the previous program and do some math.

  # The install instructions at the end of the message are tool specific - key
  # off of generators or build tools.
  if("${CMAKE_GENERATOR}" MATCHES "Make")
    set(INSTALL_LINE "Run 'make install' to begin installation into ${CMAKE_INSTALL_PREFIX}")
    set(BENCHMARK_LINE "Run 'make benchmark' to run the BRL-CAD Benchmark Suite")
  endif("${CMAKE_GENERATOR}" MATCHES "Make")
  if("${CMAKE_GENERATOR}" MATCHES "Ninja")
    set(INSTALL_LINE "Run 'ninja install' to begin installation into ${CMAKE_INSTALL_PREFIX}")
    set(BENCHMARK_LINE "Run 'ninja benchmark' to run the BRL-CAD Benchmark Suite")
  endif("${CMAKE_GENERATOR}" MATCHES "Ninja")
  if("${CMAKE_GENERATOR}" MATCHES "Xcode")
    set(INSTALL_LINE "Run 'xcodebuild -target install' to begin installation into ${CMAKE_INSTALL_PREFIX}")
    set(BENCHMARK_LINE "Run 'xcodebuild -target benchmark' to run the BRL-CAD Benchmark Suite")
  endif("${CMAKE_GENERATOR}" MATCHES "Xcode")
  if(MSVC)
    # slightly misuse the lines for MSVC, since we don't usually do the
    # install/benchmark routine there. (Benchmarks aren't currently supported
    # in MSVC anyway.)
    set(INSTALL_LINE "To build, launch Visual Studio and open ${CMAKE_BINARY_DIR}/BRLCAD.sln")
    set(BENCHMARK_LINE "Build the ALL_BUILD target.  To create an NSIS installer, build the PACKAGE target")
  endif(MSVC)

  set(dreport_src "
#define _CRT_SECURE_NO_WARNINGS 1

#include <time.h>
#include <stdio.h>
#include <string.h>

void printtime(long tdiff) {
  long d_mins, d_hrs, d_days;
  d_days = 0; d_hrs = 0; d_mins = 0;
  if (tdiff > 86400) { d_days = tdiff / 86400; tdiff = tdiff % 86400; }
  if (tdiff > 3600) { d_hrs = tdiff / 3600; tdiff = tdiff % 3600; }
  if (tdiff > 60) { d_mins = tdiff / 60; tdiff = tdiff % 60; }
  if (d_days > 0) { if (d_days == 1) { printf(\"%ld day \", d_days); } else { printf(\"%ld days \", d_days); } }
  if (d_hrs > 0) { if (d_hrs == 1) { printf(\"%ld hour \", d_hrs); } else { printf(\"%ld hours \", d_hrs); } }
  if (d_mins > 0) { if (d_mins == 1) { printf(\"%ld minute \", d_mins); } else { printf(\"%ld minutes \", d_mins); } }
  if (tdiff > 0) { if (tdiff == 1) { printf(\"%ld second \", tdiff); } else { printf(\"%ld seconds \", tdiff); } }
  if (tdiff == 0 && d_mins == 0 && d_hrs == 0 && d_days == 0) { printf(\"0 seconds \"); }
}

int main(int argc, const char **argv) {

  FILE *infp = NULL; time_t t = time(NULL); long start_time;
  if (argc < 3) return 1;
  if (strncmp(argv[1], \"final\", 5) == 0) {
    if (argc < 4) return 1;
    printf(\"Done.\\n\\nBRL-CAD Release ${BRLCAD_VERSION}, Build ${CONFIG_DATE}\\n\\nElapsed compilation time: \");
    infp = fopen(argv[2], \"r\"); (void)fscanf(infp, \"%ld\", &start_time); ; fclose(infp); printtime(((long)t) - start_time);
    printf(\"\\nElapsed time since configuration: \");
    infp = fopen(argv[3], \"r\"); (void)fscanf(infp, \"%ld\", &start_time); ; fclose(infp); printtime(((long)t) - start_time);
    printf(\"\\n---\\n${INSTALL_LINE}\\n${BENCHMARK_LINE}\\n\\n\");
    return 0;
  }
  printf(\"%s\", argv[1]);
  infp = fopen(argv[2], \"r\"); (void)fscanf(infp, \"%ld\", &start_time); ; fclose(infp); printtime(((long)t) - start_time);
  printf(\"\\n\");
  return 0;
}
")

  # Build the code so we can run it
  file(WRITE "${CMAKE_BINARY_DIR}/CMakeTmp/dreport.c" "${dreport_src}")
  try_compile(dreport_build "${CMAKE_BINARY_DIR}/CMakeTmp"
    SOURCES "${CMAKE_BINARY_DIR}/CMakeTmp/dreport.c"
    OUTPUT_VARIABLE FREPORT_BUILD_INFO
    COPY_FILE "${CMAKE_BINARY_DIR}/CMakeTmp/dreport${EXE_EXT}")
  if(NOT dreport_build)
    message(FATAL_ERROR "Could not build time delta reporting utility: ${FREPORT_BUILD_INFO}")
  endif(NOT dreport_build)
  file(REMOVE "${CMAKE_BINARY_DIR}/CMakeTmp/dreport.c")
  if(COMMAND distclean)
    distclean("${CMAKE_BINARY_DIR}/CMakeTmp/dreport")
  endif(COMMAND distclean)

endfunction(generate_timer_exes)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8
