cmake_minimum_required(VERSION 3.20)
project(SimpleNES LANGUAGES C CXX)

if(NOT WIN32)
    message(FATAL_ERROR "The rasterm frontend currently requires Windows Terminal.")
endif()

set(SIMPLENES_RASTERM_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../.." CACHE PATH
    "rasterm source directory used when rasterm::rasterm is not already available")

if(NOT TARGET rasterm::rasterm)
    if(EXISTS "${SIMPLENES_RASTERM_SOURCE}/CMakeLists.txt")
        set(RASTERM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(RASTERM_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
        set(RASTERM_BUILD_FUZZERS OFF CACHE BOOL "" FORCE)
        set(RASTERM_BUILD_SHARED_C_API OFF CACHE BOOL "" FORCE)
        add_subdirectory("${SIMPLENES_RASTERM_SOURCE}"
            "${CMAKE_CURRENT_BINARY_DIR}/rasterm" EXCLUDE_FROM_ALL)
    else()
        find_package(rasterm 1.2 CONFIG REQUIRED)
    endif()
endif()

file(GLOB SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/main.cpp"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/APU/*.cpp"
)
file(GLOB VENDOR_SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/vendor/miniaudio/*.c"
)

find_package(OpenCV CONFIG REQUIRED COMPONENTS core imgproc)

add_executable(SimpleNES ${SOURCES} ${VENDOR_SOURCES})
target_compile_features(SimpleNES PRIVATE cxx_std_20)
set_property(TARGET SimpleNES PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
target_include_directories(SimpleNES PRIVATE
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/vendor/miniaudio"
)
target_compile_definitions(SimpleNES PRIVATE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)
target_link_libraries(SimpleNES PRIVATE
    rasterm::rasterm
    opencv_core
    opencv_imgproc
    avrt
    ole32
    winmm
)

configure_file(keybindings.conf keybindings.conf COPYONLY)

function(define_file_basename_for_sources targetname)
    get_target_property(source_files "${targetname}" SOURCES)
    foreach(sourcefile ${source_files})
        get_filename_component(basename "${sourcefile}" NAME)
        set_property(SOURCE "${sourcefile}" APPEND PROPERTY
            COMPILE_DEFINITIONS "__FILENAME__=\"${basename}\"")
    endforeach()
endfunction()

define_file_basename_for_sources(SimpleNES)
install(TARGETS SimpleNES RUNTIME DESTINATION bin)
