message(STATUS "Compiling with MinGW")
# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER /usr/bin/i686-w64-mingw32-cc)
SET(CMAKE_CXX_COMPILER /usr/bin/i686-w64-mingw32-c++)
SET(CMAKE_RC_COMPILER /usr/bin/i686-w64-mingw32-windres)

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH} /usr/i686-w64-mingw32/ ${CMAKE_CURRENT_SOURCE_DIR}/../winlib/ )

set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../winlibs/libs/)
include_directories(/${CMAKE_CURRENT_SOURCE_DIR}/../winlibs/include/)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
#set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
#set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)