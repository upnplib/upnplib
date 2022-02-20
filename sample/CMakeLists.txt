# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-02-20

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(SAMPLE VERSION 0001
                  DESCRIPTION "Simple sample to use the library"
                  HOMEPAGE_URL "https://github.com/upnplib")


link_libraries(${UPNP_GMOCK_LIBRARIES}
               # On Linux we cannot link with the static pthreads library
               # because then we have also link with a static libc that's not
               # available by default.
               $<$<NOT:$<BOOL:${WIN32}>>:${UPNP_PTHREADS_SHARED_LIBRARY}>
               # On MS Windows we link with the static pthreads4w library to
               # avoid error prone managing access to its .dll file.
               $<$<BOOL:${WIN32}>:${UPNP_PTHREADS_STATIC_LIBRARY}>
)

IF (client)
	ADD_EXECUTABLE (tv_ctrlpt
		common/sample_util.c
		common/tv_ctrlpt.c
		linux/tv_ctrlpt_main.c
	)

	TARGET_INCLUDE_DIRECTORIES (tv_ctrlpt
		PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/common
		PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/common
	)

	TARGET_LINK_LIBRARIES (tv_ctrlpt
		upnp_shared
	)

	INSTALL (TARGETS tv_ctrlpt
		DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
ENDIF (client)

IF (client AND device)
	ADD_EXECUTABLE (tv_combo
		common/sample_util.c
		common/tv_ctrlpt.c
		common/tv_device.c
		linux/tv_combo_main.c
	)

	TARGET_INCLUDE_DIRECTORIES (tv_combo
		PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/common
		PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/common
	)

	TARGET_LINK_LIBRARIES (tv_combo
		upnp_shared
	)

	INSTALL (TARGETS tv_combo
		DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
ENDIF (client AND device)

IF (device)
	add_executable (tv_device
		${PROJECT_SOURCE_DIR}/common/sample_util.cpp
		${PROJECT_SOURCE_DIR}/common/tv_device.cpp
		${PROJECT_SOURCE_DIR}/linux/tv_device_main.cpp
	)

	target_include_directories (tv_device PRIVATE
		${PROJECT_SOURCE_DIR}/common
                ${UPNP_CORE_SOURCE_DIR}/inc
                ${PUPNP_IXML_SOURCE_DIR}/inc
                ${CMAKE_BINARY_DIR}/upnp/inc
	)

        target_compile_definitions(tv_device PRIVATE
                UPNP_HAVE_TOOLS
        )

	target_link_libraries (tv_device
		upnplib_shared
	)

	#install (TARGETS tv_device
        #      DESTINATION ${CMAKE_INSTALL_BINDIR}
	#)

	#install (DIRECTORY web/
	#	DESTINATION ${CMAKE_INSTALL_DATADIR}/upnp
	#)
ENDIF (device)
