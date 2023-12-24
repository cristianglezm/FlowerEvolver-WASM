#
# Try to find JsonBox library and include path.
# Once done this will define
#
# JsonBox_FOUND
# JsonBox_INCLUDE_DIR
# JsonBox_LIBRARIES
# JsonBox_LIBRARY
# JsonBox_ROOT
#

set(FIND_JsonBox_PATHS
    ${JsonBox_ROOT}
    $ENV{JsonBox_ROOT}
    /usr/local
    /usr/local/include
    /usr
    /sw
    /opt/local
    /opt/csw
    /opt)

if(JsonBox_ROOT)
    SET(LIB_SUFFIX "/")
    if(CMAKE_SYSTEM_NAME MATCHES "Android")
        SET(LIB_SUFFIX "/${CMAKE_ANDROID_ARCH_ABI}")
    endif(CMAKE_SYSTEM_NAME MATCHES "Android")
	SET(JsonBox_INCLUDE_DIR "${JsonBox_ROOT}/include")
    SET(JsonBox_LIBRARY "${JsonBox_ROOT}/lib${LIB_SUFFIX}/libJsonBox.a")
    SET(JsonBox_LIBRARIES ${JsonBox_LIBRARY})
	SET(JsonBox_FOUND 1)
endif(JsonBox_ROOT)

find_path(JsonBox_INCLUDE_DIR include/JsonBox.h
          PATH_SUFFIXES include
          PATHS ${FIND_JsonBox_PATHS})

find_library(JsonBox_LIBRARY
		NAMES JsonBox JsonBox.a
		PATH_SUFFIXES lib
		PATHS ${FIND_JsonBox_PATHS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JsonBox DEFAULT_MSG JsonBox_LIBRARY JsonBox_INCLUDE_DIR)

set(JsonBox_INCLUDE_DIR "${JsonBox_INCLUDE_DIR}")
set(JsonBox_LIBRARIES "${JsonBox_LIBRARY}")

if(NOT JsonBox_FOUND)
	set(FIND_JsonBox_ERROR "Could NOT find JsonBox")
	if(JsonBox_FIND_REQUIRED)
		message(FATAL_ERROR ${FIND_JsonBox_ERROR})
	endif()
endif()
