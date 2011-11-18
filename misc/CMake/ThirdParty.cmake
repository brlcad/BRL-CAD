#-----------------------------------------------------------------------------
MACRO(THIRD_PARTY lower dir)
	STRING(TOUPPER ${lower} upper)
	BUNDLE_OPTION(${CMAKE_PROJECT_NAME}_${upper} "")
	FOREACH(extraarg ${ARGN})
		IF(extraarg STREQUAL "NOSYS")
			IF(${CMAKE_PROJECT_NAME}_${upper} STREQUAL "SYSTEM (AUTO)")
				SET(${CMAKE_PROJECT_NAME}_${upper} "BUNDLED (AUTO)" CACHE STRING "NOSYS passed - locally modified copy of code" FORCE)
			ENDIF(${CMAKE_PROJECT_NAME}_${upper} STREQUAL "SYSTEM (AUTO)")
		ENDIF(extraarg STREQUAL "NOSYS")
	ENDFOREACH(extraarg ${ARGN})
	# Main search logic
	IF(${CMAKE_PROJECT_NAME}_${upper} STREQUAL "BUNDLED" OR ${CMAKE_PROJECT_NAME}_${upper} STREQUAL "BUNDLED (AUTO)")
		SET(${CMAKE_PROJECT_NAME}_${upper}_BUILD ON)
		SET(${upper}_LIBRARY "${lower}" CACHE STRING "set by THIRD_PARTY macro" FORCE)
	ELSE(${CMAKE_PROJECT_NAME}_${upper} STREQUAL "BUNDLED" OR ${CMAKE_PROJECT_NAME}_${upper} STREQUAL "BUNDLED (AUTO)")
		SET(${CMAKE_PROJECT_NAME}_${upper}_BUILD OFF)
		# Stash the previous results (if any) so we don't repeatedly call out the tests - only report
		# if something actually changes in subsequent runs.
		SET(${upper}_FOUND_STATUS ${${upper}_FOUND})
		# Initialize (or rather, uninitialize) variables in preparation for search
		SET(${upper}_FOUND "${upper}-NOTFOUND" CACHE STRING "${upper}_FOUND" FORCE)
		MARK_AS_ADVANCED(${upper}_FOUND)
		SET(${upper}_LIBRARY "${upper}-NOTFOUND" CACHE STRING "${upper}_LIBRARY" FORCE)
		SET(${upper}_INCLUDE_DIR "${upper}-NOTFOUND" CACHE STRING "${upper}_INCLUDE_DIR" FORCE)
		# Be quiet if we're doing this over
		IF("${${upper}_FOUND_STATUS}" MATCHES "${upper}-NOTFOUND")
			SET(${upper}_FIND_QUIETLY TRUE)
		ENDIF("${${upper}_FOUND_STATUS}" MATCHES "${upper}-NOTFOUND")
		# Include the Find module for the library in question
		IF(EXISTS ${${CMAKE_PROJECT_NAME}_CMAKE_DIR}/Find${upper}.cmake)
			INCLUDE(${${CMAKE_PROJECT_NAME}_CMAKE_DIR}/Find${upper}.cmake)
		ELSE(EXISTS ${${CMAKE_PROJECT_NAME}_CMAKE_DIR}/Find${upper}.cmake)
			INCLUDE(${CMAKE_ROOT}/Modules/Find${upper}.cmake)
		ENDIF(EXISTS ${${CMAKE_PROJECT_NAME}_CMAKE_DIR}/Find${upper}.cmake)
		# If we didn't find it, and options allow it, enable the local copy
		IF(NOT ${upper}_FOUND AND NOT ${CMAKE_PROJECT_NAME}_${upper} STREQUAL "SYSTEM")
			SET(${CMAKE_PROJECT_NAME}_${upper}_BUILD ON)
			SET(${CMAKE_PROJECT_NAME}_${upper} "BUNDLED (AUTO)" CACHE STRING "System version not found, using local" FORCE)
			SET(${upper}_LIBRARY "${lower}" CACHE STRING "set by THIRD_PARTY macro" FORCE)
		ELSE(NOT ${upper}_FOUND AND NOT ${CMAKE_PROJECT_NAME}_${upper} STREQUAL "SYSTEM")
			SET(${upper}_FOUND "TRUE" CACHE STRING "${upper}_FOUND" FORCE)
			SET(${CMAKE_PROJECT_NAME}_${upper} "SYSTEM (AUTO)" CACHE STRING "System version found" FORCE)
		ENDIF(NOT ${upper}_FOUND AND NOT ${CMAKE_PROJECT_NAME}_${upper} STREQUAL "SYSTEM")
		IF(${upper}_FOUND)
			# We have to remove any previously built output from enabled local copies of the
			# library in question, or the linker will get confused - a system lib was found and
			# system libraries are to be preferred with current options.  This is unfortunate in
			# that it may introduce extra build work just from trying configure options, but appears
			# to be essential to ensuring that the build "just works" each time.
			STRING(REGEX REPLACE "lib" "" rootname "${lower}")
			FILE(GLOB STALE_FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_SHARED_LIBRARY_PREFIX}${rootname}*${CMAKE_SHARED_LIBRARY_SUFFIX}*")
			FOREACH(stale_file ${STALE_FILES})
				EXEC_PROGRAM(
					${CMAKE_COMMAND} ARGS -E remove ${stale_file}
					OUTPUT_VARIABLE rm_out
					RETURN_VALUE rm_retval
					)
			ENDFOREACH(stale_file ${STALE_FILES})
			IF("${${upper}_FOUND_STATUS}" MATCHES "${upper}-NOTFOUND")
				MESSAGE("Reconfiguring to use system ${upper}")
			ENDIF("${${upper}_FOUND_STATUS}" MATCHES "${upper}-NOTFOUND")
		ENDIF(${upper}_FOUND)
	ENDIF(${CMAKE_PROJECT_NAME}_${upper} STREQUAL "BUNDLED" OR ${CMAKE_PROJECT_NAME}_${upper} STREQUAL "BUNDLED (AUTO)")
	IF(${CMAKE_PROJECT_NAME}_${upper}_BUILD)
		ADD_SUBDIRECTORY(${dir})
		SET(${upper}_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${dir};${CMAKE_CURRENT_BINARY_DIR}/${dir} CACHE STRING "set by THIRD_PARTY_SUBDIR macro" FORCE)
		IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${dir}.dist)
			INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/${dir}.dist)
			DISTCHECK_IGNORE(${dir} ${dir}_ignore_files)
		ELSE(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${dir}.dist)
			MESSAGE("Local build, but file ${CMAKE_CURRENT_SOURCE_DIR}/${dir}.dist not found")
		ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${dir}.dist)
	ELSE(${CMAKE_PROJECT_NAME}_${upper}_BUILD)
		DISTCHECK_IGNORE_ITEM(${dir})
	ENDIF(${CMAKE_PROJECT_NAME}_${upper}_BUILD)
	MARK_AS_ADVANCED(${upper}_LIBRARY)
	MARK_AS_ADVANCED(${upper}_INCLUDE_DIR)
ENDMACRO(THIRD_PARTY)

#-----------------------------------------------------------------------------
MACRO(THIRD_PARTY_EXECUTABLE lower dir)
	STRING(TOUPPER ${lower} upper)
	BUNDLE_OPTION(${CMAKE_PROJECT_NAME}_${upper} "")
	FOREACH(extraarg ${ARGN})
		IF(extraarg STREQUAL "NOSYS")
			IF(${CMAKE_PROJECT_NAME}_${upper} STREQUAL "SYSTEM (AUTO)")
				SET(${CMAKE_PROJECT_NAME}_${upper} "BUNDLED (AUTO)" CACHE STRING "NOSYS passed - locally modified copy of code" FORCE)
			ENDIF(${CMAKE_PROJECT_NAME}_${upper} STREQUAL "SYSTEM (AUTO)")
		ENDIF(extraarg STREQUAL "NOSYS")
	ENDFOREACH(extraarg ${ARGN})
	# Main search logic
	IF(${CMAKE_PROJECT_NAME}_${upper} STREQUAL "BUNDLED" OR ${CMAKE_PROJECT_NAME}_${upper} STREQUAL "BUNDLED (AUTO)")
		SET(${CMAKE_PROJECT_NAME}_${upper}_BUILD ON)
		SET(${upper}_EXECUTABLE "${lower}" CACHE STRING "set by THIRD_PARTY macro" FORCE)
		# Include the Find module for the exec in question - need macro routines
		IF(EXISTS ${${CMAKE_PROJECT_NAME}_CMAKE_DIR}/Find${upper}.cmake)
			INCLUDE(${${CMAKE_PROJECT_NAME}_CMAKE_DIR}/Find${upper}.cmake)
		ELSE(EXISTS ${${CMAKE_PROJECT_NAME}_CMAKE_DIR}/Find${upper}.cmake)
			INCLUDE(${CMAKE_ROOT}/Modules/Find${upper}.cmake)
		ENDIF(EXISTS ${${CMAKE_PROJECT_NAME}_CMAKE_DIR}/Find${upper}.cmake)
	ELSE(${CMAKE_PROJECT_NAME}_${upper} STREQUAL "BUNDLED" OR ${CMAKE_PROJECT_NAME}_${upper} STREQUAL "BUNDLED (AUTO)")
		SET(${CMAKE_PROJECT_NAME}_${upper}_BUILD OFF)
		# Stash the previous results (if any) so we don't repeatedly call out the tests - only report
		# if something actually changes in subsequent runs.
		SET(${upper}_FOUND_STATUS ${${upper}_FOUND})
		# Initialize (or rather, uninitialize) variables in preparation for search
		SET(${upper}_FOUND "${upper}-NOTFOUND" CACHE STRING "${upper}_FOUND" FORCE)
		MARK_AS_ADVANCED(${upper}_FOUND)
		SET(${upper}_EXECUTABLE "${upper}-NOTFOUND" CACHE STRING "${upper}_EXECUTABLE" FORCE)
		# Be quiet if we're doing this over
		IF("${${upper}_FOUND_STATUS}" MATCHES "${upper}-NOTFOUND")
			SET(${upper}_FIND_QUIETLY TRUE)
		ENDIF("${${upper}_FOUND_STATUS}" MATCHES "${upper}-NOTFOUND")
		# Include the Find module for the exec in question
		IF(EXISTS ${${CMAKE_PROJECT_NAME}_CMAKE_DIR}/Find${upper}.cmake)
			INCLUDE(${${CMAKE_PROJECT_NAME}_CMAKE_DIR}/Find${upper}.cmake)
		ELSE(EXISTS ${${CMAKE_PROJECT_NAME}_CMAKE_DIR}/Find${upper}.cmake)
			INCLUDE(${CMAKE_ROOT}/Modules/Find${upper}.cmake)
		ENDIF(EXISTS ${${CMAKE_PROJECT_NAME}_CMAKE_DIR}/Find${upper}.cmake)
		# If we didn't find it, and options allow it, enable the local copy
		IF(NOT ${upper}_FOUND AND NOT ${CMAKE_PROJECT_NAME}_${upper} STREQUAL "SYSTEM" AND NOT ${CMAKE_PROJECT_NAME}_${upper} STREQUAL "SYSTEM (AUTO)")
			SET(${CMAKE_PROJECT_NAME}_${upper}_BUILD ON)
			SET(${CMAKE_PROJECT_NAME}_${upper} "BUNDLED (AUTO)" CACHE STRING "System version not found, using local" FORCE)
			SET(${upper}_EXECUTABLE "${lower}" CACHE STRING "set by THIRD_PARTY macro" FORCE)
		ELSE(NOT ${upper}_FOUND AND NOT ${CMAKE_PROJECT_NAME}_${upper} STREQUAL "SYSTEM" AND NOT ${CMAKE_PROJECT_NAME}_${upper} STREQUAL "SYSTEM (AUTO)")
			SET(${upper}_FOUND "TRUE" CACHE STRING "${upper}_FOUND" FORCE)
			SET(${CMAKE_PROJECT_NAME}_${upper} "SYSTEM (AUTO)" CACHE STRING "System version found" FORCE)
		ENDIF(NOT ${upper}_FOUND AND NOT ${CMAKE_PROJECT_NAME}_${upper} STREQUAL "SYSTEM" AND NOT ${CMAKE_PROJECT_NAME}_${upper} STREQUAL "SYSTEM (AUTO)")
		IF(${upper}_FOUND)
			IF("${${upper}_FOUND_STATUS}" MATCHES "${upper}-NOTFOUND")
				MESSAGE("Reconfiguring to use system ${upper}")
			ENDIF("${${upper}_FOUND_STATUS}" MATCHES "${upper}-NOTFOUND")
		ENDIF(${upper}_FOUND)
	ENDIF(${CMAKE_PROJECT_NAME}_${upper} STREQUAL "BUNDLED" OR ${CMAKE_PROJECT_NAME}_${upper} STREQUAL "BUNDLED (AUTO)")
	IF(${CMAKE_PROJECT_NAME}_${upper}_BUILD)
		ADD_SUBDIRECTORY(${dir})
		IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${dir}.dist)
			INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/${dir}.dist)
			DISTCHECK_IGNORE(${dir} ${dir}_ignore_files)
		ELSE(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${dir}.dist)
			MESSAGE("Local build, but file ${CMAKE_CURRENT_SOURCE_DIR}/${dir}.dist not found")
		ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${dir}.dist)
	ELSE(${CMAKE_PROJECT_NAME}_${upper}_BUILD)
		DISTCHECK_IGNORE_ITEM(${dir})
	ENDIF(${CMAKE_PROJECT_NAME}_${upper}_BUILD)
	MARK_AS_ADVANCED(${upper}_EXECUTABLE)
ENDMACRO(THIRD_PARTY_EXECUTABLE)
