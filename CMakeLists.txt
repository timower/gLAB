cmake_minimum_required(VERSION 3.3)
project(gLAB)

# TODO: split into subdirectiries and files
set(gLAB_INCLUDE
    include/gLAB/gui.h
    include/gLAB/utils.h
    include/gLAB/window.h
    include/gLAB/application.h)
set(gLAB_SRC
        src/main.cpp
        src/gui.cpp
        src/window.cpp
		src/app.h
		src/app.cpp)

file(GLOB GUI_HEADERS include/gLAB/gui/*.h)

# IMGUI
file(GLOB IMGUI_SRC lib/imgui/*.cpp lib/imgui/*.h)

set(GLAD_SRC lib/glad/src/glad.c)

add_executable(gLAB ${gLAB_SRC} ${gLAB_INCLUDE} ${IMGUI_SRC} ${GUI_HEADERS} ${GLAD_SRC})

target_include_directories(gLAB PRIVATE lib/imgui)

target_include_directories(gLAB PRIVATE lib/glad/include)

target_compile_features(gLAB PRIVATE cxx_std_17)

target_include_directories(gLAB PRIVATE "src")
target_include_directories(gLAB PUBLIC "include")

target_compile_definitions(gLAB PRIVATE NOMINMAX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # using Clang
  target_compile_options(gLAB PRIVATE -fno-exceptions -pedantic -Wall -ggdb -fdiagnostics-color=always)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # using GCC
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb -O1")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -O1 -march=native -msse2 -msse4.1 -march=core-avx2")
  target_compile_options(gLAB PRIVATE -fno-exceptions -Wno-unknown-pragmas -pedantic -Wall -ggdb -fdiagnostics-color=always)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # using Visual Studio C++
  # TODO: fix
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()

  #TODO: release and debug config (both optimized..)
  #message(WARNING ${CMAKE_CONFIGURATION_TYPES})
endif()

# ODE:
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
#set(ODE_16BIT_INDICES "Use 16-bit indices for trimeshes (default is 32-bit)." OFF)
#set(ODE_NO_BUILTIN_THREADING_IMPL "Disable built-in multithreaded threading implementation." OFF)
#set(ODE_NO_THREADING_INTF "Disable threading interface support (external implementations cannot be assigned." OFF)
#set(ODE_OLD_TRIMESH "Use old OPCODE trimesh-trimesh collider." OFF)
set(ODE_WITH_DEMOS OFF CACHE BOOL "" FORCE)
#set(ODE_WITH_GIMPACT "Use GIMPACT for trimesh collisions (experimental)." OFF)
#set(ODE_WITH_LIBCCD "Use libccd for handling some collision tests absent in ODE." OFF)
#set(ODE_WITH_OPCODE "Use old OPCODE trimesh-trimesh collider." ON)
#set(ODE_WITH_OU "Use TLS for global caches (allows threaded collision checks for separated spaces)." OFF)
set(ODE_WITH_TESTS OFF CACHE BOOL "" FORCE)

#add_subdirectory(deps/odedevs-ode-6f44023660dc)
#target_include_directories(gLAB PRIVATE deps/odedevs-ode-6f44023660dc/include)

# GLFW:
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(lib/glfw)
target_include_directories(gLAB PRIVATE lib/glfw/include)

# BGFX:
#set( BGFX_BUILD_TOOLS      ON )
#set( BGFX_BUILD_EXAMPLES   OFF  )
#set( BGFX_INSTALL          OFF  )
#set( BGFX_INSTALL_EXAMPLES OFF )
#set( BGFX_CUSTOM_TARGETS   OFF  )
#set( BGFX_USE_OVR          OFF )
#set( BGFX_AMALGAMATED      ON )
#set( BX_AMALGAMATED        ON )
#set( BGFX_CONFIG_DEBUG     OFF )
#set( BGFX_USE_DEBUG_SUFFIX ON  )
#add_subdirectory(lib/bgfx.cmake)
#target_include_directories(gLAB PRIVATE lib/bgfx.cmake/bgfx/include)

# GLM
target_include_directories(gLAB PRIVATE lib/glm)

# tinygltf:
#target_include_directories(gLAB PRIVATE deps/tinygltf-devel)

# Assets:
#target_include_directories(gLAB PRIVATE assets/include)

# STB:
#target_include_directories(gLAB PRIVATE deps/stb-master)

# Fast noise:
# add_subdirectory(deps/FastNoiseSIMD-master)
# get_target_property(fastIncludes FastNoiseSIMD PUBLIC_HEADER)
# get_filename_component(fastDir ${fastIncludes} DIRECTORY)
# message(STATUS ${fastDir})
# target_include_directories(gLAB PRIVATE ${fastDir})

# Font ICONS:
target_include_directories(gLAB PRIVATE lib/IconFontCppHeaders)

target_link_libraries(gLAB glfw  ${OPENGL_LIBRARIES}) # ODE FastNoiseSIMD)
set_target_properties(gLAB PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/")
#target_include_directories(app PRIVATE lib/glfw/include)
#target_include_directories(app PRIVATE lib/imgui)
