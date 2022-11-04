# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-11-04

cmake_minimum_required(VERSION 3.18)
include(../../../cmake/project-header.cmake)

project(PUPNP_IXML_TEST VERSION 0005
                   DESCRIPTION "IXML library tests"
                   HOMEPAGE_URL "https://github.com/upnplib")


#file (GLOB XML_FILES
#	${PROJECT_SOURCE_DIR}/testdata/*.xml
#)

# ixml test executable linked with shared library
#------------------------------------------------
add_executable (test_ixml_shared
	test_document.cpp
)
target_include_directories(test_ixml_shared
    PRIVATE
        ${PUPNP_SOURCE_DIR}/upnp/inc
        ${PUPNP_IXML_SOURCE_DIR}/inc
)
target_compile_definitions (test_ixml_shared
    PRIVATE
        $<$<BOOL:${script_support}>:IXML_HAVE_SCRIPTSUPPORT>
        $<$<CONFIG:Debug>:DEBUG>
        UPNPLIB_SHARED
)
target_link_libraries (test_ixml_shared
    PRIVATE
        ixml_shared
)

#add_test (NAME test_ixml
#	COMMAND test_ixml_shared ${XML_FILES}
#)

#set_tests_properties (test_ixml PROPERTIES
#	ENVIRONMENT "PATH=$<TARGET_FILE_DIR:ixml_shared>\;%PATH%"
#)


# ixml test executable linked with static library
#------------------------------------------------
add_executable (test_ixml_static
	test_document.cpp
)
target_compile_definitions (test_ixml_static
    PRIVATE
        $<$<BOOL:${script_support}>:IXML_HAVE_SCRIPTSUPPORT>
        $<$<CONFIG:Debug>:DEBUG>
)
target_include_directories(test_ixml_static
    PRIVATE
        ${PUPNP_SOURCE_DIR}/upnp/inc
        ${PUPNP_IXML_SOURCE_DIR}/inc
)
target_link_libraries(test_ixml_static
    PRIVATE
        ixml_static
)

#add_test (NAME test_ixml_static
#	COMMAND test_ixml_static ${XML_FILES}
#)
