# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-02-21

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(SAMPLE VERSION 0002
                  DESCRIPTION "Simple sample to use the library"
                  HOMEPAGE_URL "https://github.com/upnplib")


include_directories(
        ${PROJECT_SOURCE_DIR}/common
        ${UPNP_CORE_SOURCE_DIR}/inc
        ${UPNP_CORE_SOURCE_DIR}/src/inc
        ${PUPNP_IXML_SOURCE_DIR}/inc
        ${CMAKE_BINARY_DIR}/upnp/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}
)

add_compile_definitions(
        PRIVATE UPNP_HAVE_TOOLS
)

link_libraries(${UPNP_GMOCK_LIBRARIES}
               ixml_shared
               upnplib_shared_win
               # On Linux we cannot link with the static pthreads library
               # because then we have also link with a static libc that's not
               # available by default.
               $<$<NOT:$<BOOL:${WIN32}>>:${UPNP_PTHREADS_SHARED_LIBRARY}>
               # On MS Windows we link with the static pthreads4w library to
               # avoid error prone managing access to its .dll file.
               $<$<BOOL:${WIN32}>:${UPNP_PTHREADS_STATIC_LIBRARY}>
)

IF (client)
	add_executable (tv_ctrlpt
                ${PROJECT_SOURCE_DIR}/common/sample_util.cpp
		${PROJECT_SOURCE_DIR}/common/tv_ctrlpt.cpp
		${PROJECT_SOURCE_DIR}/linux/tv_ctrlpt_main.cpp
	)

	#INSTALL (TARGETS tv_ctrlpt
	#	DESTINATION ${CMAKE_INSTALL_BINDIR}
	#)
ENDIF (client)

# client and device
IF (combo)
	add_executable (tv_combo
		${PROJECT_SOURCE_DIR}/common/sample_util.cpp
		${PROJECT_SOURCE_DIR}/common/tv_ctrlpt.cpp
		${PROJECT_SOURCE_DIR}/common/tv_device.cpp
		${PROJECT_SOURCE_DIR}/linux/tv_combo_main.cpp
	)
        target_compile_definitions(tv_combo
                # This is to avoid to compile in 'upnpdebug' that has many
                # dependencies into the libary and would pull many other sources
                PRIVATE NDEBUG
        )

	#INSTALL (TARGETS tv_combo
	#	DESTINATION ${CMAKE_INSTALL_BINDIR}
	#)
ENDIF (combo)

IF (device)
	add_executable (tv_device
		${PROJECT_SOURCE_DIR}/common/sample_util.cpp
		${PROJECT_SOURCE_DIR}/common/tv_device.cpp
		${PROJECT_SOURCE_DIR}/linux/tv_device_main.cpp
	)
        target_compile_definitions(tv_device
                # This is to avoid to compile in 'upnpdebug' that has many
                # dependencies into the libary and would pull many other sources
                PRIVATE NDEBUG
        )

	#install (TARGETS tv_device
        #      DESTINATION ${CMAKE_INSTALL_BINDIR}
	#)

	#install (DIRECTORY web/
	#	DESTINATION ${CMAKE_INSTALL_DATADIR}/upnp
	#)
ENDIF (device)
