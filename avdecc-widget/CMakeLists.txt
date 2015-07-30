cmake_minimum_required (VERSION 2.8) 
project (avdecc_gui)
enable_testing()

# Compiler-specific C++11 activation.
if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
    if (NOT (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7))
        message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.7 or greater.")
    endif ()
    set(CMAKE_CXX_FLAGS                "-Wall -std=c++11")
    set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g")
    set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    set(CMAKE_CXX_FLAGS                "-Wall -std=c++11 -DHAVE_SNPRINTF")
    set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g")
    set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
#else ()
#    message(FATAL_ERROR "Your C++ compiler does not support C++11.")
endif ()

# wxWidgets
find_package(wxWidgets COMPONENTS core base adv REQUIRED)
include(${wxWidgets_USE_FILE})

include_directories(include ../../avdecc-lib/controller/lib/include ../../jdksavdecc-c/include)

file(GLOB_RECURSE AVBGUI_INCLUDES "*.h" )
file(GLOB_RECURSE AVBGUI_SRC "*.cpp" )

set(AVDECCLIB_DIR ${PROJECT_BINARY_DIR}/../avdecc-lib/controller/lib/${CMAKE_CFG_INTDIR})
add_library(avdecc-lib_controller SHARED IMPORTED)
set_property(TARGET avdecc-lib_controller PROPERTY IMPORTED_LOCATION ${AVDECCLIB_DIR}/avdecc-lib_controller32.dll)
set_property(TARGET avdecc-lib_controller PROPERTY IMPORTED_IMPLIB ${AVDECCLIB_DIR}/avdecc-lib_controller32.lib)

if(APPLE)
  add_executable (avbgui ${AVBGUI_INCLUDES} ${AVBGUI_SRC})
elseif(UNIX)
  add_executable (avbgui ${AVBGUI_INCLUDES} ${AVBGUI_SRC})
  target_link_libraries(avbgui rt)
elseif(WIN32)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_executable (avbgui WIN32 ${AVBGUI_INCLUDES} ${AVBGUI_SRC})
  add_custom_command(TARGET avbgui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_BINARY_DIR}/../avdecc-lib/controller/lib/$<CONFIGURATION>"
    "$<TARGET_FILE_DIR:avbgui>")
endif()

target_link_libraries(avbgui avdecc-lib_controller32)
target_link_libraries(avbgui ${wxWidgets_LIBRARIES})


