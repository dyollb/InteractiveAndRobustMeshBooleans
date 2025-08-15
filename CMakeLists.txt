cmake_minimum_required(VERSION 3.21)

# Set the target architecture.
# All modern x86/x64 processors support AVX2.
# Older x86/x64 processors may support SSE2 but not AVX2.
# Very old x86/x64 processors, or non x86/x64
# processors, do not support any of the two.
set(ENABLE_SSE2 True)
set(ENABLE_AVX2 True)


# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set the project name
project(mesh_booleans)

set(TBB_TEST OFF CACHE BOOL " " FORCE)
set(TBB_EXAMPLES OFF CACHE BOOL " " FORCE)
set(TBB_INSTALL OFF CACHE BOOL " " FORCE)
add_subdirectory(arrangements/external/oneTBB EXCLUDE_FROM_ALL)


# add the executable
add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC
    ./
    code/
    arrangements/code/
    arrangements/external/Indirect_Predicates/include/
)

set(cinolib_DIR ${PROJECT_SOURCE_DIR}/arrangements/external/Cinolib)
set(CINOLIB_USES_OPENGL_GLFW_IMGUI ON)
set(CINOLIB_USES_SHEWCHUK_PREDICATES ON)

find_package(cinolib REQUIRED)

target_link_libraries(${PROJECT_NAME} cinolib)
target_link_libraries(${PROJECT_NAME} tbb)
target_compile_definitions(${PROJECT_NAME} PUBLIC TBB_PARALLEL=1)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/arrangements/external/abseil-cpp/)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/arrangements/external/oneTBB/)


add_executable(${PROJECT_NAME}_rotation EXCLUDE_FROM_ALL main-rotation.cpp)

target_include_directories(${PROJECT_NAME}_rotation PUBLIC
    ./
    code/
    arrangements/code/
    arrangements/external/Indirect_Predicates/include/
)

target_link_libraries(${PROJECT_NAME}_rotation cinolib)
target_link_libraries(${PROJECT_NAME}_rotation tbb)
target_compile_definitions(${PROJECT_NAME}_rotation PUBLIC TBB_PARALLEL=1)
target_include_directories(${PROJECT_NAME}_rotation PUBLIC ${PROJECT_SOURCE_DIR}/arrangements/external/abseil-cpp/)
target_include_directories(${PROJECT_NAME}_rotation PUBLIC ${PROJECT_SOURCE_DIR}/arrangements/external/oneTBB/)

add_executable(${PROJECT_NAME}_arap EXCLUDE_FROM_ALL main-arap.cpp)

target_include_directories(${PROJECT_NAME}_arap PUBLIC
    ./
    code/
    arrangements/code/
    arrangements/external/Indirect_Predicates/include/
)

target_link_libraries(${PROJECT_NAME}_arap cinolib)
target_link_libraries(${PROJECT_NAME}_arap tbb)
target_compile_definitions(${PROJECT_NAME}_arap PUBLIC TBB_PARALLEL=1)
target_include_directories(${PROJECT_NAME}_arap PUBLIC ${PROJECT_SOURCE_DIR}/arrangements/external/abseil-cpp/)
target_include_directories(${PROJECT_NAME}_arap PUBLIC ${PROJECT_SOURCE_DIR}/arrangements/external/oneTBB/)

add_executable(${PROJECT_NAME}_stencil EXCLUDE_FROM_ALL main-stencil.cpp)

target_include_directories(${PROJECT_NAME}_stencil PUBLIC
    ./
    code/
    arrangements/code/
    arrangements/external/Indirect_Predicates/include/
)

target_link_libraries(${PROJECT_NAME}_stencil cinolib)
target_link_libraries(${PROJECT_NAME}_stencil tbb)
target_compile_definitions(${PROJECT_NAME}_stencil PUBLIC TBB_PARALLEL=1)
target_include_directories(${PROJECT_NAME}_stencil PUBLIC ${PROJECT_SOURCE_DIR}/arrangements/external/abseil-cpp/)
target_include_directories(${PROJECT_NAME}_stencil PUBLIC ${PROJECT_SOURCE_DIR}/arrangements/external/oneTBB/)


add_executable(${PROJECT_NAME}_inputcheck EXCLUDE_FROM_ALL main-inputcheck.cpp)

target_include_directories(${PROJECT_NAME}_inputcheck PUBLIC
        ./
        code/
        arrangements/code/
        arrangements/external/Indirect_Predicates/include/
)

target_link_libraries(${PROJECT_NAME}_inputcheck cinolib)
target_link_libraries(${PROJECT_NAME}_inputcheck tbb)
target_compile_definitions(${PROJECT_NAME}_inputcheck PUBLIC TBB_PARALLEL=1)
target_include_directories(${PROJECT_NAME}_inputcheck PUBLIC ${PROJECT_SOURCE_DIR}/arrangements/external/abseil-cpp/)
target_include_directories(${PROJECT_NAME}_inputcheck PUBLIC ${PROJECT_SOURCE_DIR}/arrangements/external/oneTBB/)

option(MESH_BOOLEANS_BUILD_SHARED "Build mesh_booleans library as shared instead of static" ON)
if(MESH_BOOLEANS_BUILD_SHARED)
    add_library(${PROJECT_NAME}_lib SHARED code/boolean_api.cpp)
else()
    add_library(${PROJECT_NAME}_lib STATIC code/boolean_api.cpp)
endif()

target_include_directories(${PROJECT_NAME}_lib PRIVATE
    ./
    code/
    arrangements/code/
    arrangements/external/Indirect_Predicates/include/
)

target_link_libraries(${PROJECT_NAME}_lib cinolib)
target_link_libraries(${PROJECT_NAME}_lib tbb)
target_compile_definitions(${PROJECT_NAME}_lib PUBLIC TBB_PARALLEL=1)
if(MESH_BOOLEANS_BUILD_SHARED)
    target_compile_definitions(${PROJECT_NAME}_lib PRIVATE MESH_BOOLEANS_EXPORTS)
endif()
target_include_directories(${PROJECT_NAME}_lib PUBLIC ${PROJECT_SOURCE_DIR}/arrangements/external/abseil-cpp/)
target_include_directories(${PROJECT_NAME}_lib PUBLIC ${PROJECT_SOURCE_DIR}/arrangements/external/oneTBB/)

# Set shared library properties
set_target_properties(${PROJECT_NAME}_lib PROPERTIES
    OUTPUT_NAME "mesh_booleans"
    VERSION 1.0.0
    SOVERSION 1
)

# Only set visibility and Windows properties for shared libraries
if(MESH_BOOLEANS_BUILD_SHARED)
    set_target_properties(${PROJECT_NAME}_lib PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES
    )
    
    # On Windows, set the DLL output directory
    if(WIN32)
        set_target_properties(${PROJECT_NAME}_lib PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        )
    endif()
endif()

set(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}_lib)
foreach(target IN LISTS TARGETS)
        # Compiler-specific options
        if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
                # grant IEEE 754 compliance
                target_compile_options(${target} PUBLIC "/fp:strict")
                # use intrinsic functions
                target_compile_options(${target} PUBLIC "/Oi")
                # set target architecture
                if(ENABLE_AVX2)
                        target_compile_options(${target} PUBLIC "/arch:AVX2")
                elseif(ENABLE_SSE2)
                        target_compile_options(${target} PUBLIC "/arch:SSE2")
                endif()
                # reserve enough stack size
                target_link_options(${target} PUBLIC "/STACK:8421376")
                # turn off annoying warnings
                target_compile_options(${target} PUBLIC "/D _CRT_SECURE_NO_WARNINGS")
        else()
                # set standard optimization level
                target_compile_options(${target} PUBLIC -O2)
                # reserve enough stack size
                target_compile_options(${target} PUBLIC -Wl,-z,stacksize=8421376)
                # grant IEEE 754 compliance
                target_compile_options(${target} PUBLIC -frounding-math)
                # set target architecture
                if(ENABLE_AVX2)
                        set_property(TARGET ${target} PROPERTY ISPC_INSTRUCTION_SETS avx2-i32x4)
                elseif(ENABLE_SSE2)
                        target_compile_options(${target} PUBLIC "-msse2")
                endif()
        endif()
endforeach()

# Install configuration
include(GNUInstallDirs)

# Install shared library
install(TARGETS ${PROJECT_NAME}_lib
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install headers
install(FILES 
    code/boolean_api.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mesh_booleans
)

# Install executables
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install license file
install(FILES 
    LICENSE
    README.md
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/mesh_booleans
)