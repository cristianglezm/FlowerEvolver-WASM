######
# Try to find EvoAI library and include path.
# Once done this will define
#
# EvoAI_FOUND
# EvoAI_INCLUDE_DIR
# EvoAI_LIBRARY
# EvoAI_LIBRARIES
# EvoAI_ROOT
######

set(FIND_EvoAI_PATHS
    ${EvoAI_ROOT}
    $ENV{EvoAI_ROOT}
    /usr/local
    /usr/local/include
    /usr
    /sw
    /opt/local
    /opt/csw
    /opt
)

if(NOT EvoAI_FIND_VERSION_MAJOR)
    set(EvoAI_FIND_VERSION_MAJOR 1)
endif()

set(LIB_SUFFIX "")
if(CMAKE_SYSTEM_NAME MATCHES "Android")
    set(LIB_SUFFIX "/${CMAKE_ANDROID_ARCH_ABI}")
endif()

if(EvoAI_BUILD_STATIC)
    set(EvoAI_LIBRARY_NAME_RELEASE "EvoAI-${EvoAI_FIND_VERSION_MAJOR}-s")
    set(EvoAI_LIBRARY_NAME_DEBUG "EvoAI-${EvoAI_FIND_VERSION_MAJOR}-s-d")
else()
    set(EvoAI_LIBRARY_NAME_RELEASE "EvoAI-${EvoAI_FIND_VERSION_MAJOR}")
    set(EvoAI_LIBRARY_NAME_DEBUG "EvoAI-${EvoAI_FIND_VERSION_MAJOR}-d")
endif()

find_path(EvoAI_INCLUDE_DIR
    EvoAI.hpp
    PATH_SUFFIXES include
    PATHS ${EvoAI_ROOT}
    NO_CMAKE_FIND_ROOT_PATH
)

find_library(EvoAI_LIBRARY_RELEASE
    NAMES ${EvoAI_LIBRARY_NAME_RELEASE}
    PATH_SUFFIXES lib${LIB_SUFFIX}
    PATHS ${EvoAI_ROOT}
    NO_CMAKE_FIND_ROOT_PATH
)

find_library(EvoAI_LIBRARY_DEBUG
    NAMES ${EvoAI_LIBRARY_NAME_DEBUG}
    PATH_SUFFIXES lib${LIB_SUFFIX}
    PATHS ${EvoAI_ROOT}
    NO_CMAKE_FIND_ROOT_PATH
)

find_path(EvoAI_INCLUDE_DIR
    EvoAI.hpp
    PATH_SUFFIXES include
    PATHS ${FIND_EvoAI_PATHS}
)

find_library(EvoAI_LIBRARY_RELEASE
    NAMES ${EvoAI_LIBRARY_NAME_RELEASE}
    PATH_SUFFIXES lib${LIB_SUFFIX}
    PATHS ${FIND_EvoAI_PATHS}
)

find_library(EvoAI_LIBRARY_DEBUG
    NAMES ${EvoAI_LIBRARY_NAME_DEBUG}
    PATH_SUFFIXES lib${LIB_SUFFIX}
    PATHS ${FIND_EvoAI_PATHS}
)

include(SelectLibraryConfigurations)
select_library_configurations(EvoAI)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    EvoAI
    DEFAULT_MSG
    EvoAI_LIBRARY
    EvoAI_INCLUDE_DIR
)
