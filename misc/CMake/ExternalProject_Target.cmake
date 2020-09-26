# This is based on the logic generated by CMake for EXPORT, but customized for
# use with ExternalProject:
#
# https://cmake.org/cmake/help/latest/module/ExternalProject.html
#
# The goal is to create an imported target based on the ExternalProject
# information, and then append the necessary install logic to manage RPath
# settings in the external projects as if the external files were built by the
# main CMake project.

# TODO - need to rework this in a way that's compatible with the superbuild
# pattern.  Main BRL-CAD build won't have the ExternalProject_Add targets from
# the superbuild, so we need another way to handle getting the necessary
# info to the main build.  Maybe useful:
# https://gitlab.kitware.com/cmake/community/-/wikis/doc/tutorials/How-to-create-a-ProjectConfig.cmake-file

# The catch to this is that the external project outputs MUST be built in a way
# that is compatible with CMake's RPath handling assumptions.  See
# https://stackoverflow.com/questions/41175354/can-i-install-shared-imported-library
# for one of the issues surrounding this - file(RPATH_CHANGE) must be able to
# succeed, and it is up to the 3rd party build setups to prepare their outputs
# to be ready.  The key variable CMAKE_BUILD_RPATH comes from running the
# function cmake_set_rpath, which must be available.

# Custom patch utility to replace the build directory path with the install
# directory path in text files - make sure CMAKE_BINARY_DIR and
# CMAKE_INSTALL_PREFIX are finalized before generating this file!
configure_file(${BRLCAD_CMAKE_DIR}/buildpath_replace.cxx.in ${CMAKE_CURRENT_BINARY_DIR}/buildpath_replace.cxx)
add_executable(buildpath_replace ${CMAKE_CURRENT_BINARY_DIR}/buildpath_replace.cxx)

# Custom patch utility for external RPath preparation
configure_file(${BRLCAD_CMAKE_DIR}/rpath_replace.cxx.in ${CMAKE_CURRENT_BINARY_DIR}/rpath_replace.cxx @ONLY)
add_executable(rpath_replace ${CMAKE_CURRENT_BINARY_DIR}/rpath_replace.cxx)

function(ExternalProject_ByProducts extproj E_IMPORT_PREFIX)

  cmake_parse_arguments(E "FIXPATH" "" "" ${ARGN})

  if (EXTPROJ_VERBOSE)

    list(LENGTH E_UNPARSED_ARGUMENTS FCNT)
    if (E_FIXPATH)
      if (FCNT GREATER 1)
	message("${extproj}: Adding path adjustment and installation rules for ${FCNT} byproducts")
      else (FCNT GREATER 1)
	message("${extproj}: Adding path adjustment and installation rules for ${FCNT} byproduct")
      endif (FCNT GREATER 1)
    else (E_FIXPATH)
      if (FCNT GREATER 1)
	message("${extproj}: Adding install rules for ${FCNT} byproducts")
      else (FCNT GREATER 1)
	message("${extproj}: Adding install rules for ${FCNT} byproduct")
      endif (FCNT GREATER 1)
    endif (E_FIXPATH)

  endif (EXTPROJ_VERBOSE)

  foreach (bpf ${E_UNPARSED_ARGUMENTS})

    set(D_IMPORT_PREFIX "${E_IMPORT_PREFIX}")
    get_filename_component(BPF_DIR "${bpf}" DIRECTORY)
    if (BPF_DIR)
      set(D_IMPORT_PREFIX "${D_IMPORT_PREFIX}/${BPF_DIR}")
    endif (BPF_DIR)

    set(I_IMPORT_PREFIX ${CMAKE_BINARY_DIR}/${E_IMPORT_PREFIX})
    install(FILES "${I_IMPORT_PREFIX}/${bpf}" DESTINATION "${D_IMPORT_PREFIX}/")
    if (E_FIXPATH)
      # Note - proper quoting for install(CODE) is extremely important for CPack, see
      # https://stackoverflow.com/a/48487133
      install(CODE "execute_process(COMMAND ${CMAKE_BINARY_DIR}/${BIN_DIR}/buildpath_replace${CMAKE_EXECUTABLE_SUFFIX} \"\${CMAKE_INSTALL_PREFIX}/${E_IMPORT_PREFIX}/${bpf}\")")
    endif (E_FIXPATH)

    DISTCLEAN("${I_IMPORT_PREFIX}/${bpf}")

  endforeach (bpf ${E_UNPARSED_ARGUMENTS})

endfunction(ExternalProject_ByProducts)


function(ET_target_props etarg IN_IMPORT_PREFIX IN_LINK_TARGET)

  cmake_parse_arguments(ET "STATIC;EXEC" "LINK_TARGET_DEBUG" "" ${ARGN})

  if(NOT CMAKE_CONFIGURATION_TYPES)

    if(ET_STATIC)
      set(IMPORT_PREFIX "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
    elseif(ET_EXEC)
      set(IMPORT_PREFIX "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    else()
      set(IMPORT_PREFIX "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
    endif(ET_STATIC)
    if(IN_IMPORT_PREFIX)
      set(IMPORT_PREFIX "${IMPORT_PREFIX}/${IN_IMPORT_PREFIX}")
    endif(IN_IMPORT_PREFIX)

    set_property(TARGET ${etarg} APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
    set_target_properties(${etarg} PROPERTIES
      IMPORTED_LOCATION_NOCONFIG "${IMPORT_PREFIX}/${IN_LINK_TARGET}"
      IMPORTED_SONAME_NOCONFIG "${IN_LINK_TARGET}"
      )

  else(NOT CMAKE_CONFIGURATION_TYPES)

    foreach(CFG_TYPE ${CMAKE_CONFIGURATION_TYPES})
      string(TOUPPER "${CFG_TYPE}" CFG_TYPE_UPPER)

      # The config variables are the ones set in this mode, but everything is being targeted to
      # one consistent top-level layout.  Adjust accordingly.
      if(ET_STATIC)
	set(IMPORT_PREFIX "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}}/../../${LIB_DIR}")
      elseif(ET_EXEC)
	set(IMPORT_PREFIX "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}}/../../${BIN_DIR}")
      else()
	set(IMPORT_PREFIX "${CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}}/../../${LIB_DIR}")
      endif(ET_STATIC)

      if(IN_IMPORT_PREFIX)
	set(IMPORT_PREFIX "${IMPORT_PREFIX}/${IN_IMPORT_PREFIX}")
      endif(IN_IMPORT_PREFIX)

      if("${CFG_TYPE_UPPER}" STREQUAL "DEBUG")
	set(LINK_TARGET ${ET_LINK_TARGET_DEBUG})
      else("${CFG_TYPE_UPPER}" STREQUAL "DEBUG")
	set(LINK_TARGET ${IN_LINK_TARGET})
      endif("${CFG_TYPE_UPPER}" STREQUAL "DEBUG")

      set_target_properties(${etarg} PROPERTIES
	IMPORTED_LOCATION_${CFG_TYPE_UPPER} "${IMPORT_PREFIX}/${LINK_TARGET}"
	IMPORTED_SONAME_${CFG_TYPE_UPPER} "${LINK_TARGET}"
	)

      if(NOT ET_STATIC AND NOT ET_EXEC AND MSVC)
	# For Windows, IMPORTED_IMPLIB is important for shared libraries.
	# It is that property that will tell a toplevel target what to link against
	# when building - pointing out the dll isn't enough by itself.
	string(REPLACE "${CMAKE_SHARED_LIBRARY_SUFFIX}" ".lib" IMPLIB_FILE "${LINK_TARGET}")
	set_target_properties(${etarg} PROPERTIES
	  IMPORTED_IMPLIB_${CFG_TYPE_UPPER} "${IMPORT_PREFIX}/${IMPLIB_FILE}"
	  )
      endif(NOT ET_STATIC AND NOT ET_EXEC AND MSVC)

    endforeach(CFG_TYPE ${CMAKE_CONFIGURATION_TYPES})

    # For everything except Debug, use the Release version
    #set_target_properties(TARGET ${etarg} PROPERTIES
    #  MAP_IMPORTED_CONFIG_MINSIZEREL Release
    #  MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
    #  )

    endif(NOT CMAKE_CONFIGURATION_TYPES)

endfunction(ET_target_props)


# For a given path, calculate the $ORIGIN style path needed relative
# to CMAKE_INSTALL_PREFIX
function(ET_Origin_Path POPATH INIT_PATH)

  get_filename_component(CPATH "${INIT_PATH}" REALPATH)
  set(RELDIRS)
  set(FPATH)
  while (NOT "${CPATH}" STREQUAL "${CMAKE_INSTALL_PREFIX}")
    get_filename_component(CDIR "${CPATH}" NAME)
    get_filename_component(CPATH "${CPATH}" DIRECTORY)
    if (NOT "${RELDIRS}" STREQUAL "")
      set(RELDIRS "${CDIR}/${RELDIRS}")
      set(FPATH "../${FPATH}")
    else (NOT "${RELDIRS}" STREQUAL "")
      set(RELDIRS "${CDIR}")
      set(FPATH "../")
    endif (NOT "${RELDIRS}" STREQUAL "")
  endwhile()

  set(FPATH "${FPATH}${RELDIRS}")

  set(${POPATH} ${FPATH} PARENT_SCOPE)
endfunction(ET_Origin_Path)

# Mimic the magic of the CMake install(TARGETS) form of the install command.
# This is the key to treating external project build outputs as fully managed
# CMake outputs, and requires that the external project build in such a way
# that the rpath settings in the build outputs are compatible with this
# mechanism.
function(ET_RPath LIB_DIR OUTPUT_DIR SUB_DIR E_OUTPUT_FILE)
  get_filename_component(RRPATH "${CMAKE_INSTALL_PREFIX}/${LIB_DIR}/${SUB_DIR}" REALPATH)
  set(OPATH)
  ET_Origin_Path(OPATH "${RRPATH}")
  if (NOT APPLE)
    set(NEW_RPATH "$ENV{DESTDIR}${RRPATH}:$ORIGIN/${OPATH}")
  else (NOT APPLE)
    set(NEW_RPATH "$ENV{DESTDIR}${RRPATH}:@loader_path/${OPATH}")
  endif (NOT APPLE)
  if (NOT DEFINED CMAKE_BUILD_RPATH)
    message(FATAL_ERROR "ET_RPath run without CMAKE_BUILD_RPATH defined - run cmake_set_rpath before defining external projects.")
  endif (NOT DEFINED CMAKE_BUILD_RPATH)
  if (NOT "${SUB_DIR}" STREQUAL "")
    set(OFINAL "${SUB_DIR}/${E_OUTPUT_FILE}")
  else (NOT "${SUB_DIR}" STREQUAL "")
    set(OFINAL "${E_OUTPUT_FILE}")
  endif (NOT "${SUB_DIR}" STREQUAL "")
  # Note - proper quoting for install(CODE) is extremely important for CPack, see
  # https://stackoverflow.com/a/48487133
  install(CODE "
  file(RPATH_CHANGE
    FILE \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${OUTPUT_DIR}/${OFINAL}\"
    OLD_RPATH \"${CMAKE_BUILD_RPATH}\"
    NEW_RPATH \"${NEW_RPATH}\")
  ")
endfunction(ET_RPath)

function(ExternalProject_Target etarg extproj)

  cmake_parse_arguments(E "RPATH;EXEC" "SUBDIR;OUTPUT_FILE;LINK_TARGET;LINK_TARGET_DEBUG;STATIC_OUTPUT_FILE;STATIC_LINK_TARGET;STATIC_LINK_TARGET_DEBUG" "SYMLINKS;DEPS" ${ARGN})

  if(NOT TARGET ${extproj})
    message(FATAL_ERROR "${extprog} is not a target")
  endif(NOT TARGET ${extproj})

  # Protect against redefinition of already defined targets.
  if(TARGET ${etarg})
    message(FATAL_ERROR "Target ${etarg} is already defined\n")
  endif(TARGET ${etarg})
  if(E_STATIC AND TARGET ${etarg}-static)
    message(FATAL_ERROR "Target ${etarg}-static is already defined\n")
  endif(E_STATIC AND TARGET ${etarg}-static)

  if (EXTPROJ_VERBOSE)
    message("${extproj}: Adding target \"${etarg}\"")
  endif (EXTPROJ_VERBOSE)

  if (E_STATIC_OUTPUT_FILE)
    set(E_STATIC 1)
  endif (E_STATIC_OUTPUT_FILE)

  if (E_OUTPUT_FILE AND NOT E_EXEC)
    set(E_SHARED 1)
  endif (E_OUTPUT_FILE AND NOT E_EXEC)

  if (E_LINK_TARGET_DEBUG)
    set(LINK_TARGET_DEBUG "${E_LINK_TARGET_DEBUG}")
  endif (E_LINK_TARGET_DEBUG)

  if (E_STATIC_LINK_TARGET_DEBUG)
    set(STATIC LINK_TARGET_DEBUG "${E_STATIC_LINK_TARGET_DEBUG}")
  endif (E_STATIC_LINK_TARGET_DEBUG)

  # Create imported target - need to both make the target itself
  # and set the necessary properties.  See also
  # https://gitlab.kitware.com/cmake/community/wikis/doc/tutorials/Exporting-and-Importing-Targets

  # Because the outputs are not properly build target outputs of the primary
  # CMake project, we need to install as either FILES or PROGRAMS
  if (NOT E_EXEC)

    # Handle shared library
    if (E_SHARED)
      add_library(${etarg} SHARED IMPORTED GLOBAL)
      if (E_LINK_TARGET)
	ET_target_props(${etarg} "${E_IMPORT_PREFIX}" ${E_LINK_TARGET} LINK_TARGET_DEBUG "${LINK_TARGET_DEBUG}")
      else (E_LINK_TARGET)
	ET_target_props(${etarg} "${E_IMPORT_PREFIX}" ${E_OUTPUT_FILE} LINK_TARGET_DEBUG "${LINK_TARGET_DEBUG}")
      endif (E_LINK_TARGET)

      install(FILES "${CMAKE_BINARY_DIR}/${LIB_DIR}/${E_SUBDIR}/${E_OUTPUT_FILE}" DESTINATION ${LIB_DIR}/${E_SUBDIR})
      if (E_RPATH AND NOT MSVC)
	ET_RPath("${LIB_DIR}" "${LIB_DIR}" "${E_SUBDIR}" "${E_OUTPUT_FILE}")
      endif (E_RPATH AND NOT MSVC)
    endif (E_SHARED)

    # If we do have a static lib as well, handle that
    if (E_STATIC AND BUILD_STATIC_LIBS)
      add_library(${etarg}-static STATIC IMPORTED GLOBAL)
      if (E_STATIC_LINK_TARGET)
	ET_target_props(${etarg}-static "${E_IMPORT_PREFIX}" ${E_STATIC_LINK_TARGET} STATIC_LINK_TARGET_DEBUG "${STATIC_LINK_TARGET_DEBUG}" STATIC)
      else (E_STATIC_LINK_TARGET)
	ET_target_props(${etarg}-static "${E_IMPORT_PREFIX}" ${E_STATIC_OUTPUT_FILE} STATIC_LINK_TARGET_DEBUG "${STATIC_LINK_TARGET_DEBUG}" STATIC)
      endif (E_STATIC_LINK_TARGET)
      if (MSVC)
	install(FILES "${CMAKE_BINARY_DIR}/${BIN_DIR}/${E_SUBDIR}/${E_OUTPUT_FILE}" DESTINATION ${BIN_DIR}/${E_SUBDIR})
      else (MSVC)
	install(FILES "${CMAKE_BINARY_DIR}/${LIB_DIR}/${E_SUBDIR}/${E_OUTPUT_FILE}" DESTINATION ${LIB_DIR}/${E_SUBDIR})
      endif (MSVC)
    endif (E_STATIC AND BUILD_STATIC_LIBS)

  else (NOT E_EXEC)

    add_executable(${etarg} IMPORTED GLOBAL)
    ET_target_props(${etarg} "${E_IMPORT_PREFIX}" ${E_OUTPUT_FILE} EXEC)
    install(PROGRAMS "${CMAKE_BINARY_DIR}/${BIN_DIR}/${E_SUBDIR}/${E_OUTPUT_FILE}" DESTINATION ${BIN_DIR}/${E_SUBDIR})
    if (E_RPATH AND NOT MSVC)
      ET_RPath("${LIB_DIR}" "${BIN_DIR}" "${E_SUBDIR}" "${E_OUTPUT_FILE}")
    endif (E_RPATH AND NOT MSVC)

  endif (NOT E_EXEC)

  # Let CMake know there is a target dependency here, despite this being an import target
  add_dependencies(${etarg} ${extproj})

  # Add install rules for any symlinks the caller has listed
  if(E_SYMLINKS)
    foreach(slink ${E_SYMLINKS})
      install(FILES "${CMAKE_BINARY_DIR}/${LIB_DIR}/${E_SUBDIR}/${slink}" DESTINATION ${LIB_DIR}/${E_SUBDIR})
    endforeach(slink ${E_SYMLINKS})
  endif (E_SYMLINKS)

endfunction(ExternalProject_Target)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

