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

set(LIB_SUFFIX "")
if(CMAKE_SYSTEM_NAME MATCHES "Android")
    set(LIB_SUFFIX "/${CMAKE_ANDROID_ARCH_ABI}")
endif()

find_path(JsonBox_INCLUDE_DIR
    JsonBox.h
    PATH_SUFFIXES include
    PATHS ${JsonBox_ROOT}
    NO_CMAKE_FIND_ROOT_PATH
)

find_library(JsonBox_LIBRARY
    NAMES JsonBox
    PATH_SUFFIXES lib${LIB_SUFFIX}
    PATHS ${JsonBox_ROOT}
    NO_CMAKE_FIND_ROOT_PATH
)

find_path(JsonBox_INCLUDE_DIR
    JsonBox.h
    PATH_SUFFIXES include
    PATHS ${FIND_JsonBox_PATHS}
)

find_library(JsonBox_LIBRARY
    NAMES JsonBox
    PATH_SUFFIXES lib${LIB_SUFFIX}
    PATHS ${FIND_JsonBox_PATHS}
)

set(JsonBox_LIBRARIES ${JsonBox_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    JsonBox
    DEFAULT_MSG
    JsonBox_LIBRARY
    JsonBox_INCLUDE_DIR
)
