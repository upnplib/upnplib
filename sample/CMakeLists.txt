# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-09-04

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(UPNPLIB_SAMPLE VERSION 0006
                       DESCRIPTION "Simple sample to use the library"
                       HOMEPAGE_URL "https://github.com/upnplib")


# Switches for this Subproject
#-----------------------------
# These switches are all handled only in this configuration file.

# Options selectable by user configuration
# To build all samples by default then use:
# cmake -S . -B build -D UPNPLIB_WITH_SAMPLES=YES
# To build only a single sample for testing and internal use you can do e.g.:
# cmake -S . -B build -D UPNPLIB_DEF_SAMPLES=YES -D UPNPLIB_DEF_SAMPLE_CLIENT=YES
if(UPNPLIB_WITH_SAMPLES)
    set(UPNPLIB_DEF_SAMPLE_DEVICE YES)
    set(UPNPLIB_DEF_SAMPLE_CLIENT YES)
    set(UPNPLIB_DEF_SAMPLE_COMBO YES)
endif()


link_libraries(
        ixml_shared
        pupnp_native_shared
        # On Linux we cannot link with the static pthreads library
        # because then we have also link with a static libc that's not
        # available by default.
        $<$<NOT:$<BOOL:${WIN32}>>:${UPNP_PTHREADS_SHARED_LIBRARY}>
        # On MS Windows we link with the static pthreads4w library to
        # avoid error prone managing access to its .dll file.
        $<$<BOOL:${WIN32}>:${UPNP_PTHREADS_STATIC_LIBRARY}>
)

IF (UPNPLIB_DEF_SAMPLE_CLIENT)
	add_executable (tv_ctrlpt
                ./common/sample_util.cpp
		./common/tv_ctrlpt.cpp
		./linux/tv_ctrlpt_main.cpp
	)
        target_include_directories(tv_ctrlpt
            PRIVATE
                ./common
                ${UPnPlib_SOURCE_DIR}/include
                ${PUPNP_UPNP_SOURCE_DIR}/inc
                ${PUPNP_IXML_SOURCE_DIR}/inc
                # if pthreads4w isn't installed this path is empty. The linker
                # doesn't find "pthread.h" and falls back to look at <pthread.h>.
                ${pthreads4w_SOURCE_DIR}
        )

	#INSTALL (TARGETS tv_ctrlpt
	#	DESTINATION ${CMAKE_INSTALL_BINDIR}
	#)
ENDIF()

# client and device
IF (UPNPLIB_DEF_SAMPLE_COMBO)
	add_executable (tv_combo
		./common/sample_util.cpp
		./common/tv_ctrlpt.cpp
		./common/tv_device.cpp
		./linux/tv_combo_main.cpp
	)
        target_include_directories(tv_combo
            PRIVATE
                ./common
                ${UPnPlib_SOURCE_DIR}/include
                ${PUPNP_UPNP_SOURCE_DIR}/inc
                ${PUPNP_IXML_SOURCE_DIR}/inc
                # if pthreads4w isn't installed this path is empty. The linker
                # doesn't find "pthread.h" and falls back to look at <pthread.h>.
                ${pthreads4w_SOURCE_DIR}
        )
        target_compile_definitions(tv_combo
            PRIVATE
                $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
                # This is to avoid to compile in 'upnpdebug' that has many
                # dependencies into the libary and would pull many other sources
                NDEBUG
        )

	#INSTALL (TARGETS tv_combo
	#	DESTINATION ${CMAKE_INSTALL_BINDIR}
	#)
ENDIF()

IF(UPNPLIB_DEF_SAMPLE_DEVICE)
	add_executable (tv_device
                ./common/sample_util.cpp
                ./common/tv_device.cpp
                ./linux/tv_device_main.cpp
        )
        target_include_directories(tv_device
            PRIVATE
                ./common
                ${UPnPlib_SOURCE_DIR}/include
                ${PUPNP_UPNP_SOURCE_DIR}/inc
                ${PUPNP_IXML_SOURCE_DIR}/inc
                # if pthreads4w isn't installed this path is empty. The linker
                # doesn't find "pthread.h" and falls back to look at <pthread.h>.
                ${pthreads4w_SOURCE_DIR}
        )
        target_compile_definitions(tv_device
            PRIVATE
                $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
                # This is to avoid to compile in 'upnpdebug' that has many
                # dependencies into the libary and would pull many other sources
                NDEBUG
        )

	#install (TARGETS tv_device
        #      DESTINATION ${CMAKE_INSTALL_BINDIR}
	#)

	#install (DIRECTORY web/
	#	DESTINATION ${CMAKE_INSTALL_DATADIR}/upnp
	#)
ENDIF()
