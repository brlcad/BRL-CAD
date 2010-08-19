# - Find Tcl/Tk includes and libraries.

# There are quite a number of potential compilcations when it comes to
# Tcl/Tk, particularly in cases where multiple versions of Tcl/Tk are
# present on a system and the case of OSX, which my have Tk built for
# either X11 or Aqua.  On Windows there may be Cygwin installs of
# Tcl/Tk as well. 
#
# Several "design philosophy" decisions have to be made - what to report
# when multiple instances of Tcl/Tk are available, how much control to
# allow users, how to expose those controls, etc.  Here are the rules
# this particular implementation of FindTCLTK will strive to express:
#
# 1. If a parent CMakeLists.txt defines a specific TCLTK_PREFIX 
#    directory, don't look for or return any settings using
#    other Tcl/Tk installations, even if nothing is found
#    below TCLTK_PREFIX and other installations are present.
#    Report NOTFOUND instead.
#
# 2. There will be three possible variables for controlling
#    which versions to report:
#
#    TCLTK_MIN_VERSION
#    TCLTK_MAX_VERSION
#    TCLTK_EXACT_VERSION
#
#    All of these will be expected to have the form:
#
#    TCLTK_MAJOR_VERSION.TCLTK_MINOR_VERSION.TCLTK_PATCH_VERSION
#
#    although the PATCH_VERSION will be optional.  If
#    no PATCH_VERSION is specified, any patch version will
#    be regarded as satisfying the criteria of any version
#    number test applied.  If no versions are specified it
#    is assumed any will do.  Higher versions are preferred
#    over lower, within the above constraints.
#
# 3. Tk provides the option to compile either for
#    the "native" graphics system (aqua, win32, etc.) or for
#    X11 (which is also the native graphics system on Unix-type
#    platforms other than Mac OSX.)  There are situations were
#    a program or developer may want to require a particular
#    windowing system.  If that is the case they can make use
#    of the following two options:
#
#    TCLTK_NATIVE_GRAPHICS
#    TCLTK_X11_GRAPHICS
#
#    If NATIVE_GRAPHICS is set to ON, a Tcl/Tk system is 
#    reported found only if the reported graphics system
#    matches that of the current platform.  If X11_GRAPHICS
#    is on, a match is reported only if the windowing system
#    reports X11.  If neither option is ON, the windowing
#    system is not a factor is deciding to report
#    FOUND or NOTFOUND.  If BOTH are ON (why??) NATIVE_GRAPHICS
#    will override the TCLTK_X11_GRAPHICS setting and set it
#    to OFF.
#
# 4. By default, if no prefix is specified, FindTCLTK will search
#    a list of directories and the system path for tcl libraries.
#    This list can be expanded by a parent CMakeLists.txt file
#    by specifying additional paths in this variable, which will
#    be checked before the system path - essentially, this lets
#    a configure process do a "soft set" of the TCL prefix:
#
#    TCLTK_SEARCH_ADDITIONAL_PATHS
#
# 5. On Apple platforms, there may be a "Framework" install of
#    Tcl/Tk. By default, FindTCLTK will start with this version
#    on OSX platforms if no TCLTK_PREFIX is specified, but will
#    move on to another installation if the Framework Tcl/Tk doesn't
#    satisfy other criteria.  If a developer wishes to REQUIRE a 
#    Framework build of Tcl/Tk and reject other installs even though
#    they may satisfy other criteria, they can enable the following 
#    option:
#
#    TCKTK_USE_FRAMEWORK_ONLY
#
# 6. If a developer needs ONLY Tcl, without the Tk graphics library,
#    they can disable the following option (on by default)
#  
#    TCLTK_REQUIRE_TK
#
# 7. Normally, FindTCLTK will assume that the intent is to compile
#    C code and will require headers.  If a developer needs Tcl/Tk
#    ONLY for the purposes of running tclsh or wish scripts and is
#    not planning to do any compiling, they can disable the following
#    option and FindTCLTK will report a Tcl/Tk installation without
#    headers as FOUND.
#
#    TCLTK_NEED_HEADERS
#
# The following "results" variables will be set (Tk variables only
# set when TCKTK_REQUIRE_TK is ON):
#
#   TCLTK_FOUND          = Tcl (and Tk) found (see TCKTK_REQUIRE_TK)
#   TCLTK_FOUND_VERSION  = Version of selected Tcl/TK
#   TCL_LIBRARY          = path to Tcl library
#   TCL_INCLUDE_PATH     = path to directory containing tcl.h
#   TK_LIBRARY           = path to Tk library
#   TK_INCLUDE_PATH      = path to directory containing tk.h
#   TCL_TCLSH            = full path to tclsh binary
#   TK_WISH              = full path wo wish binary

# Tcl/Tk tends to name things using version numbers, so we need a
# list of numbers to check 
SET(TCLTK_POSSIBLE_MAJOR_VERSIONS 8)
SET(TCLTK_POSSIBLE_MINOR_VERSIONS 6 5 4 3 2 1 0)

# Create the Tcl/Tk options if not already present
OPTION(TCLTK_NATIVE_GRAPHICS "Require native Tk graphics." OFF)
OPTION(TCLTK_X11_GRAPHICS "Require X11 Tk graphics." OFF)
OPTION(TCLTK_USE_FRAMEWORK_ONLY "Don't use any Tcl/Tk installation that isn't a Framework." OFF)
OPTION(TCLTK_REQUIRE_TK "Look for Tk installation, not just Tcl." ON)
OPTION(TCLTK_NEED_HEADERS "Don't report a found Tcl/Tk unless headers are present." ON)

# Set up the logic for determing the tk windowingsystem.  This requires
# running a small wish script and recovering the results from a file -
# wrap this logic in a macro

SET(tkwin_script "
set filename \"${CMAKE_BINARY_DIR}/CMakeTmp/TK_WINDOWINGSYSTEM\"
set fileId [open $filename \"w\"]
set windowingsystem [tk windowingsystem]
puts $fileId $windowingsystem
close $fileId
exit
")

SET(tkwin_scriptfile "${CMAKE_BINARY_DIR}/CMakeTmp/tk_windowingsystem.tcl")

MACRO(TKGRAPHICSSYSTEM wishcmd resultvar)
   SET(${resultvar} "wm-NOTFOUND")
   FILE(WRITE ${tkwin_scriptfile} ${tkwin_script})
   EXEC_PROGRAM(${wishcmd} ARGS ${tkwin_scriptfile} OUTPUT_VARIABLE EXECOUTPUT)
   FILE(READ ${CMAKE_BINARY_DIR}/CMakeTmp/TK_WINDOWINGSYSTEM ${resultvar})
   MESSAGE("TK graphics: ${${resultvar}}")
ENDMACRO()

#TKGRAPHICSSYSTEM(wish TK_GRAPHICSTYPE)

# For ActiveState's Tcl/Tk install on Windows, there are some specific
# paths that may be needed.  This is a macro-ized version of the paths
# found in CMake's FindTCL.cmake
MACRO(WIN32TCLTKPATHS vararray extension)
  IF(WIN32)
    GET_FILENAME_COMPONENT(
      ActiveTcl_CurrentVersion 
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActiveTcl;CurrentVersion]" 
      NAME)
      SET(${vararray} ${${vararray}}
        "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActiveTcl\\${ActiveTcl_CurrentVersion}]/${extension}")
      FOREACH(MAJORNUM ${TCLTK_POSSIBLE_MAJOR_VERSIONS})
        FOREACH(MINORNUM ${TCLTK_POSSIBLE_MINOR_VERSIONS})
         SET(${vararray} ${${vararray}} "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\${MAJORNUM}.${MINORNUM};Root]/${extension}")
        ENDFOREACH()
      ENDFOREACH()
   ENDIF(WIN32)
ENDMACRO(WIN32TCLTKPATHS)

# Need some routines to chop version numbers up
MACRO(SPLIT_TCL_VERSION_NUM versionnum)
     STRING(REGEX REPLACE "([0-9]*).[0-9]*.?[0-9]*" "\\1" ${versionnum}_MAJOR "${${versionnum}}")
     STRING(REGEX REPLACE "[0-9]*.([0-9]*).?[0-9]*" "\\1" ${versionnum}_MINOR "${${versionnum}}")
     STRING(REGEX REPLACE "[0-9]*.[0-9]*.?([0-9]*)" "\\1" ${versionnum}_PATCH "${${versionnum}}")
ENDMACRO()    

# If an exact version is set, peg min and max at it too so the comparisons will work
IF(TCLTK_EXACT_VERSION)
   SET(TCLTK_MIN_VERSION ${TCLTK_EXACT_VERSION})
   SET(TCLTK_MAX_VERSION ${TCLTK_EXACT_VERSION})
ENDIF(TCLTK_EXACT_VERSION)


MACRO(FIND_LIBRARY_VERSIONS targetname pathnames options)
   SPLIT_TCL_VERSION_NUM(TCLTK_MIN_VERSION)
   SPLIT_TCL_VERSION_NUM(TCLTK_MAX_VERSION)
   FOREACH(MAJORNUM ${TCLTK_POSSIBLE_MAJOR_VERSIONS})
      if(NOT MAJORNUM LESS TCLTK_MIN_VERSION_MAJOR)
      if(NOT MAJORNUM GREATER TCLTK_MAX_VERSION_MAJOR)
      FOREACH(MINORNUM ${TCLTK_POSSIBLE_MINOR_VERSIONS})
         if(NOT MINORNUM LESS TCLTK_MIN_VERSION_MINOR)
         if(NOT MINORNUM GREATER TCLTK_MAX_VERSION_MINOR)
         SET(TCLTK_${targetname}${MAJORNUM}${MINORNUM} TCLTK_${targetname}${MAJORNUM}${MINORNUM}-NOTFOUND)
         FOREACH(SPATH ${${pathnames}})
            FIND_LIBRARY(TCLTK_${targetname}${MAJORNUM}${MINORNUM} NAMES ${targetname}${MAJORNUM}.${MINORNUM} ${targetname}${MAJORNUM}${MINORNUM} PATHS ${SPATH} ${options})
            IF(NOT TCLTK_${targetname}${MAJORNUM}${MINORNUM} MATCHES "NOTFOUND$")
               # On repeat configures, CMAKE_INSTALL_PREFIX apparently gets added to the FIND_* paths.  This is a problem
               # if re-installing over a previous install so strip these out - don't use output from the previous build to 
	       # do the current build!
               IF(NOT TCLTK_${targetname}${MAJORNUM}${MINORNUM} MATCHES "^${CMAKE_INSTALL_PREFIX}")
                  SET(TCLTK_${targetname}${MAJORNUM}${MINORNUM}_LIST ${TCLTK_${targetname}${MAJORNUM}${MINORNUM}_LIST} ${TCLTK_${targetname}${MAJORNUM}${MINORNUM}})
               endif()
            endif()
            SET(TCLTK_${targetname}${MAJORNUM}${MINORNUM} TCLTK_${targetname}${MAJORNUM}${MINORNUM}-NOTFOUND)
         ENDFOREACH()
	 #MESSAGE("TCLTK_${targetname}${MAJORNUM}${MINORNUM}_LIST: ${TCLTK_${targetname}${MAJORNUM}${MINORNUM}_LIST}")
         endif()
         endif()
      ENDFOREACH()
      endif()
      endif()
   ENDFOREACH()
ENDMACRO()

MACRO(PULL_PREFERRED_VERSION  targetname targetvar)
   FOREACH(MAJORNUM ${TCLTK_POSSIBLE_MAJOR_VERSIONS})
      if(NOT MAJORNUM LESS TCLTK_MIN_VERSION_MAJOR)
      if(NOT MAJORNUM GREATER TCLTK_MAX_VERSION_MAJOR)
      if(${${targetvar}} MATCHES "NOTFOUND$")
      FOREACH(MINORNUM ${TCLTK_POSSIBLE_MINOR_VERSIONS})
         if(NOT MINORNUM LESS TCLTK_MIN_VERSION_MINOR)
         if(NOT MINORNUM GREATER TCLTK_MAX_VERSION_MINOR)
         if(${${targetvar}} MATCHES "NOTFOUND$")
	    LIST(LENGTH TCLTK_${targetname}${MAJORNUM}${MINORNUM}_LIST LISTLENGTH)
            IF(LISTLENGTH GREATER 0)
               LIST(GET TCLTK_${targetname}${MAJORNUM}${MINORNUM}_LIST 0 ${targetvar})
	       LIST(REMOVE_AT TCLTK_${targetname}${MAJORNUM}${MINORNUM}_LIST 0)
            endif()
         endif()
         endif()
         endif()
      ENDFOREACH()
      endif()
      endif()
      endif()
   ENDFOREACH()
ENDMACRO()

# Try to be a bit forgiving with the TCLTK prefix - if someone gives the
# full path to the lib directory, catch that by adding the parent path
# to the list to check
IF(TCLTK_PREFIX)
   SET(TCL_LIBRARY "NOTFOUND")
   SET(TCLTK_PREFIX_LIBDIRS ${TCLTK_PREFIX_LIBDIRS} ${TCLTK_PREFIX}/lib)
   SET(TCLTK_PREFIX_LIBDIRS ${TCLTK_PREFIX_LIBDIRS} ${TCLTK_PREFIX})
   GET_FILENAME_COMPONENT(TCLTK_LIB_PATH_PARENT "${TCLTK_PREFIX}" PATH)
   SET(TCLTK_PREFIX_LIBDIRS ${TCLTK_PREFIX_LIBDIRS} ${TCLTK_LIB_PATH_PARENT}/lib)
   SET(TCLTK_PREFIX_LIBDIRS ${TCLTK_PREFIX_LIBDIRS} ${TCLTK_LIB_PATH_PARENT})
   LIST(REMOVE_DUPLICATES TCLTK_PREFIX_LIBDIRS)
   MESSAGE("TCLTK_PREFIX_LIBDIRS: ${TCLTK_PREFIX_LIBDIRS}")
   FIND_LIBRARY_VERSIONS(tcl TCLTK_PREFIX_LIBDIRS NO_SYSTEM_PATH)
   PULL_PREFERRED_VERSION(tcl TCL_LIBRARY)
   MESSAGE("TCL_LIBRARY: ${TCL_LIBRARY}")
   GET_FILENAME_COMPONENT(TCLTK_LIB_PATH_PARENT "${TCL_LIBRARY}" PATH)
   MESSAGE("TCL_PATH: ${TCLTK_LIB_PATH_PARENT}")
     set(tclconffile "tclConfig.sh-NOTFOUND") 
     find_file(tclconffile tclConfig.sh PATHS ${TCLTK_LIB_PATH_PARENT})
     if(NOT tclconffile MATCHES "NOTFOUND$")
	MESSAGE("Found config file: ${tclconffile}")
        FILE(READ ${tclconffile} TCL_CONF_FILE)
#        MESSAGE("${tclconffile}:\n ${TCL_CONF_FILE}")
        STRING(REGEX REPLACE "\r?\n" ";" ENT "${TCL_CONF_FILE}")
        FOREACH(line ${ENT})
#           STRING(REGEX REPLACE "(tclConfig)" "\\1" INCLUDEDIR "${TCL_CONF_FILE}")
	   MESSAGE("INCLUDEDIR: ${line}")
	endforeach()
     endif()
ENDIF(TCLTK_PREFIX)

#SET(syspath "/usr")
#FIND_LIBRARY_VERSIONS(tcl syspath "")

#LIST(REMOVE_DUPLICATES TCL_FOUND_MAJOR_VERSIONS)
#LIST(REMOVE_DUPLICATES TCL_FOUND_MINOR_VERSIONS)

GET_FILENAME_COMPONENT(TCL_TCLSH_PATH_PARENT "${TCL_BIN_DIR}" PATH)
SET(TCLTK_POSSIBLE_PATHS ${TCLTK_POSSIBLE_PATHS} ${TCL_TCLSH_PATH_PARENT})

GET_FILENAME_COMPONENT(TCL_TCLSH_PATH_PARENT "${TCL_LIBRARY_DIR}" PATH)
SET(TCLTK_POSSIBLE_PATHS ${TCLTK_POSSIBLE_PATHS} ${TCL_TCLSH_PATH_PARENT})

GET_FILENAME_COMPONENT(TCL_TCLSH_PATH_PARENT "${TCL_INCLUDE_DIR}" PATH)
SET(TCLTK_POSSIBLE_PATHS ${TCLTK_POSSIBLE_PATHS} ${TCL_TCLSH_PATH_PARENT})

SET(TCLTK_POSSIBLE_PATHS ${TCLTK_POSSIBLE_PATHS} ${TCL_PREFIX})

FOREACH(MAJORNUM ${TCL_FOUND_MAJOR_VERSIONS})
   FOREACH(MINORNUM ${TCL_FOUND_MINOR_VERSIONS})
       SET(LISTITEM 0)
       LIST(LENGTH TCL_TCLSH${MAJORNUM}${MINORNUM}_LIST LISTLENGTH)
       IF(LISTLENGTH GREATER 0)
          WHILE (LISTLENGTH GREATER LISTITEM)
             LIST(GET TCL_TCLSH${MAJORNUM}${MINORNUM}_LIST ${LISTITEM} TCL_TCLSH)
             GET_FILENAME_COMPONENT(TCL_TCLSH_PATH "${TCL_TCLSH}" PATH)
             GET_FILENAME_COMPONENT(TCL_TCLSH_PATH_PARENT "${TCL_TCLSH_PATH}" PATH)
             IF (TCLTK_USE_TK)
		 # FIXME: There is a risk here of picking up a wish in a system path that we don't want to consider, but if
		 # the NO_SYSTEM_PATH option is added valid entires in our list will not find their corresponding wish
		 # even if it is there - looks like we're going to have to special case tclsh/wish found in system paths
                 FIND_PROGRAM(TCLTK_WISH${MAJORNUM}${MINORNUM} NAMES wish${MAJORNUM}.${MINORNUM} wish${MAJORNUM}${MINORNUM} PATHS ${TCL_TCLSH_PATH_PARENT}/bin})
                 IF(NOT TCLTK_WISH${MAJORNUM}${MINORNUM} MATCHES "NOTFOUND$")
                    SET(TCLTK_POSSIBLE_PATHS ${TCLTK_POSSIBLE_PATHS} ${TCL_TCLSH_PATH_PARENT})
		 endif()
             ELSE(TCLTK_USE_TK)
               SET(TCLTK_POSSIBLE_PATHS ${TCLTK_POSSIBLE_PATHS} ${TCL_TCLSH_PATH_PARENT})
             ENDIF(TCLTK_USE_TK)
	     math(EXPR LISTITEM "${LISTITEM} + 1")
	  endwhile()
       endif()
   ENDFOREACH()
ENDFOREACH()

#LIST(REMOVE_DUPLICATES TCLTK_POSSIBLE_PATHS)

FOREACH(SPATH ${TCLTK_POSSIBLE_PATHS})
     set(tclconffile "tclConfig.sh-NOTFOUND") 
     find_file(tclconffile tclConfig.sh PATHS ${SPATH}/lib)
     if(NOT tclconffile MATCHES "NOTFOUND$")
	MESSAGE("Found config file: ${tclconffile}")
     endif()
endforeach()

GET_FILENAME_COMPONENT(TCL_TCLSH_PATH "${TCL_TCLSH}" PATH)
GET_FILENAME_COMPONENT(TCL_TCLSH_PATH_PARENT "${TCL_TCLSH_PATH}" PATH)
STRING(REGEX REPLACE 
  "^.*tclsh([0-9]\\.*[0-9]).*$" "\\1" TCL_TCLSH_VERSION "${TCL_TCLSH}")
GET_FILENAME_COMPONENT(TK_WISH_PATH "${TK_WISH}" PATH)
GET_FILENAME_COMPONENT(TK_WISH_PATH_PARENT "${TK_WISH_PATH}" PATH)
STRING(REGEX REPLACE 
  "^.*wish([0-9]\\.*[0-9]).*$" "\\1" TK_WISH_VERSION "${TK_WISH}")

GET_FILENAME_COMPONENT(TCL_INCLUDE_PATH_PARENT "${TCL_INCLUDE_PATH}" PATH)
GET_FILENAME_COMPONENT(TK_INCLUDE_PATH_PARENT "${TK_INCLUDE_PATH}" PATH)

GET_FILENAME_COMPONENT(TCL_LIBRARY_PATH "${TCL_LIBRARY}" PATH)
GET_FILENAME_COMPONENT(TCL_LIBRARY_PATH_PARENT "${TCL_LIBRARY_PATH}" PATH)
STRING(REGEX REPLACE 
  "^.*tcl([0-9]\\.*[0-9]).*$" "\\1" TCL_LIBRARY_VERSION "${TCL_LIBRARY}")

GET_FILENAME_COMPONENT(TK_LIBRARY_PATH "${TK_LIBRARY}" PATH)
GET_FILENAME_COMPONENT(TK_LIBRARY_PATH_PARENT "${TK_LIBRARY_PATH}" PATH)
STRING(REGEX REPLACE 
  "^.*tk([0-9]\\.*[0-9]).*$" "\\1" TK_LIBRARY_VERSION "${TK_LIBRARY}")

SET(TCLTK_POSSIBLE_LIB_PATHS
#  "${TCL_INCLUDE_PATH_PARENT}/lib"
#  "${TK_INCLUDE_PATH_PARENT}/lib"
#  "${TCL_LIBRARY_PATH}"
#  "${TK_LIBRARY_PATH}"
#  "${TCL_TCLSH_PATH_PARENT}/lib"
#  "${TK_WISH_PATH_PARENT}/lib"
  /usr/lib 
  /usr/local/lib
  )
FIND_LIBRARY(TCL_LIBRARY
  NAMES 
  tcl   
  tcl${TK_LIBRARY_VERSION} tcl${TCL_TCLSH_VERSION} tcl${TK_WISH_VERSION}
  tcl86 tcl8.6 
  tcl85 tcl8.5 
  tcl84 tcl8.4 
  tcl83 tcl8.3 
  tcl82 tcl8.2 
  tcl80 tcl8.0
  PATHS ${TCLTK_POSSIBLE_LIB_PATHS}
  )

FIND_LIBRARY(TK_LIBRARY 
  NAMES 
  tk
  tk${TCL_LIBRARY_VERSION} tk${TCL_TCLSH_VERSION} tk${TK_WISH_VERSION}
  tk86 tk8.6
  tk85 tk8.5 
  tk84 tk8.4 
  tk83 tk8.3 
  tk82 tk8.2 
  tk80 tk8.0
  PATHS ${TCLTK_POSSIBLE_LIB_PATHS}
  )

SET(TCLTK_POSSIBLE_INCLUDE_PATHS
  "${TCL_LIBRARY_PATH_PARENT}/include"
  "${TK_LIBRARY_PATH_PARENT}/include"
  "${TCL_INCLUDE_PATH}"
  "${TK_INCLUDE_PATH}"
#  ${TCL_FRAMEWORK_INCLUDES} 
#  ${TK_FRAMEWORK_INCLUDES} 
  "${TCL_TCLSH_PATH_PARENT}/include"
  "${TK_WISH_PATH_PARENT}/include"
  /usr/include
  /usr/local/include
  /usr/include/tcl${TK_LIBRARY_VERSION}
  /usr/include/tcl${TCL_LIBRARY_VERSION}
  /usr/local/include/tcl${TK_LIBRARY_VERSION}
  /usr/local/include/tcl${TCL_LIBRARY_VERSION}
  /usr/include/tcl8.6
  /usr/include/tcl8.5
  /usr/include/tcl8.4
  /usr/include/tcl8.3
  /usr/include/tcl8.2
  /usr/include/tcl8.0
  /usr/local/include/tcl8.6
  /usr/local/include/tcl8.5
  /usr/local/include/tcl8.4
  /usr/local/include/tcl8.3
  /usr/local/include/tcl8.2
  /usr/local/include/tcl8.0
  /usr/include/tk${TK_LIBRARY_VERSION}
  /usr/include/tk${TCL_LIBRARY_VERSION}
  /usr/local/include/tk${TK_LIBRARY_VERSION}
  /usr/local/include/tk${TCL_LIBRARY_VERSION}
  /usr/include/tk8.6
  /usr/include/tk8.5
  /usr/include/tk8.4
  /usr/include/tk8.3
  /usr/include/tk8.2
  /usr/include/tk8.0
  /usr/local/include/tk8.6
  /usr/local/include/tk8.5
  /usr/local/include/tk8.4
  /usr/local/include/tk8.3
  /usr/local/include/tk8.2
  /usr/local/include/tk8.0
  )

IF(WIN32)
  SET(TCLTK_POSSIBLE_INCLUDE_PATHS ${TCLTK_POSSIBLE_INCLUDE_PATHS}
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActiveTcl\\${ActiveTcl_CurrentVersion}]/include"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.6;Root]/include"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.5;Root]/include"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.4;Root]/include"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.3;Root]/include"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.2;Root]/include"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.0;Root]/include"
    "$ENV{ProgramFiles}/Tcl/include"
    "C:/Program Files/Tcl/include"
    "C:/Tcl/include"
    )
ENDIF(WIN32)

FIND_PATH(TCL_INCLUDE_PATH 
  NAMES tcl.h
  HINTS ${TCLTK_POSSIBLE_INCLUDE_PATHS}
  )

FIND_PATH(TK_INCLUDE_PATH 
  NAMES tk.h
  HINTS ${TCLTK_POSSIBLE_INCLUDE_PATHS}
  )

# handle the QUIETLY and REQUIRED arguments and set TCL_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(TCL DEFAULT_MSG TCL_LIBRARY TCL_INCLUDE_PATH)
SET(TCLTK_FIND_REQUIRED ${TCL_FIND_REQUIRED})
SET(TCLTK_FIND_QUIETLY  ${TCL_FIND_QUIETLY})
FIND_PACKAGE_HANDLE_STANDARD_ARGS(TCLTK DEFAULT_MSG TCL_LIBRARY TCL_INCLUDE_PATH TK_LIBRARY TK_INCLUDE_PATH)
SET(TK_FIND_REQUIRED ${TCL_FIND_REQUIRED})
SET(TK_FIND_QUIETLY  ${TCL_FIND_QUIETLY})
FIND_PACKAGE_HANDLE_STANDARD_ARGS(TK DEFAULT_MSG TK_LIBRARY TK_INCLUDE_PATH)

MARK_AS_ADVANCED(
  TCL_INCLUDE_PATH
  TK_INCLUDE_PATH
  TCL_LIBRARY
  TK_LIBRARY
  )

MESSAGE("TCL_INCLUDE_PATH: ${TCL_INCLUDE_PATH}")
MESSAGE("TK_INCLUDE_PATH: ${TK_INCLUDE_PATH}")
MESSAGE("TCL_LIBRARY: ${TCL_LIBRARY}")
MESSAGE("TK_LIBRARY: ${TK_LIBRARY}")

