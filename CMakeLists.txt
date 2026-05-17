cmake_minimum_required(VERSION 3.10)
project(base-lib
    LANGUAGES C
)

add_library(base-lib STATIC)

set_property(TARGET base-lib PROPERTY C_STANDARD 11)

set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Lib)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src)
set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc)

target_include_directories(base-lib PUBLIC
    ${INC_DIR}
    ${LIB_DIR}/SipHash/Inc
)

target_sources(base-lib PRIVATE
    ${SRC_DIR}/Base_ArrayList.c
    ${SRC_DIR}/Base_Hash.c
    ${SRC_DIR}/Base_HashMap.c
    ${LIB_DIR}/SipHash/Src/siphash.c
    ${LIB_DIR}/SipHash/Src/halfsiphash.c
)

# ---- Tests ----
option(BUILD_TESTS "Build unit tests (Check + CTest)" OFF)

if(BUILD_TESTS)
    include(Test/Config.cmake)
endif()
