cmake_minimum_required (VERSION 3.5.1)

# project title
project(HE-lutEval-CKKS)
set(CMAKE_CXX_STANDARD 17)
# find package
find_package(OpenFHE REQUIRED)
option( BUILD_STATIC "Set to ON to include static versions of the library" OFF)

# setup c++
SET(CMAKE_CXX_FLAGS ${OpenFHE_CXX_FLAGS} )
SET(CMAKE_BUILD_TYPE "Release")
# SET(CMAKE_BUILD_TYPE "Debug") // in PST 2025
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -Wno-error=unused-variable -Wno-error=unused-result -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -Wno-error=unused-variable -Wno-error=maybe-uninitialized -Wno-error=unused-result")

# include dictionary openfhe  and head file
include_directories( ${OpenFHE_INCLUDE} )
include_directories( ${OpenFHE_INCLUDE}/third-party/include )
include_directories( ${OpenFHE_INCLUDE}/core )
include_directories( ${OpenFHE_INCLUDE}/pke )
include_directories( ${OpenFHE_INCLUDE}/binfhe )
include_directories( ${CMAKE_SOURCE_DIR}/include)


link_directories( ${OpenFHE_LIBDIR} )
if(BUILD_STATIC)
    set( CMAKE_EXE_LINKER_FLAGS "${OpenFHE_EXE_LINKER_FLAGS} -static")
    link_libraries( ${OpenFHE_STATIC_LIBRARIES} )
else()
    set( CMAKE_EXE_LINKER_FLAGS ${OpenFHE_EXE_LINKER_FLAGS} )
    link_libraries( ${OpenFHE_SHARED_LIBRARIES} )
endif()

# setup openfhe
link_directories( ${OpenFHE_LIBDIR} )
if(BUILD_STATIC)
    set( CMAKE_EXE_LINKER_FLAGS "${OpenFHE_EXE_LINKER_FLAGS} -static")
    link_libraries( ${OpenFHE_STATIC_LIBRARIES} )
else()
    set( CMAKE_EXE_LINKER_FLAGS ${OpenFHE_EXE_LINKER_FLAGS} )
    link_libraries( ${OpenFHE_SHARED_LIBRARIES} )
endif()

# setip output path
function(setup_property TARGETS)
    message("Setup")
    foreach(target IN LISTS TARGETS)
        message("Setting ${target}...")
        set_target_properties("${target}"
                PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
        )
        target_compile_features("${target}" PUBLIC cxx_std_17)
        target_compile_options("${target}"
                PUBLIC
                -Wall
                $<$<CONFIG:Debug>:-O0 -g3>           # -D CMAKE_BUILD_TYPE=Debug
                $<$<CONFIG:Release>:-O3>             # -D CMAKE_BUILD_TYPE=Release
                $<$<CONFIG:MinSizeRel>:-Os>          # -D CMAKE_BUILD_TYPE=MinSizeRel
                $<$<CONFIG:RelWithDebInfo>:-O3 -g3>  # -D CMAKE_BUILD_TYPE=RelWithDebInfo
        )
        target_link_options("${target}" PUBLIC -Wall)
        target_include_directories("${target}"
                PRIVATE ${CMAKE_SOURCE_DIR}/src
                PRIVATE ${CMAKE_SOURCE_DIR}/include
                PRIVATE ${CMAKE_SOURCE_DIR}/include/proposedMethod
        )
    endforeach()
endfunction()


file(GLOB common_sources src/proposedMethod/*.cpp)

add_executable(gen_tables
        src/gen_tables.cpp
        ${common_sources}
)
add_executable(gen_keys
        src/gen_keys.cpp
        ${common_sources}
)
# baseline
add_executable(main_Li
        src/main_Li.cpp
        ${common_sources}
)

# proposedMethod
add_executable(main_oneinput
        src/main_oneinput.cpp
        ${common_sources}
)
add_executable(main_twoinput
        src/main_twoinput.cpp
        ${common_sources}
)
add_executable(main_threeinput
        src/main_threeinput.cpp
        ${common_sources}
)

target_link_libraries(gen_keys
        stdc++fs
)
target_link_libraries(gen_tables
        stdc++fs
)
target_link_libraries(main_Li
        stdc++fs
)
target_link_libraries(main_oneinput
        stdc++fs
)
target_link_libraries(main_twoinput
        stdc++fs
)
target_link_libraries(main_threeinput
        stdc++fs
)


set(TARGETS
        gen_keys
        gen_tables
        main_Li
        main_oneinput
        main_twoinput
        main_threeinput
)

setup_property("${TARGETS}")
