set(TEST_DIR ${CMAKE_SOURCE_DIR}/Test)

enable_testing()

find_package(PkgConfig REQUIRED)
pkg_check_modules(CHECK REQUIRED check)

find_package(Threads REQUIRED)

add_executable(test-std
    ${TEST_DIR}/main.c
    ${TEST_DIR}/TestRegistry.c
    ${TEST_DIR}/HashMap/HashMap_test.c
)

set_property(TARGET test-std PROPERTY C_STANDARD 11)

target_include_directories(test-std PRIVATE
    ${CHECK_INCLUDE_DIRS}
    ${INC_DIR}/
    ${TEST_DIR}/
)

set(C_FLAGS ${CHECK_CFLAGS_OTHER} -Wall -Wextra -O0 -g -fdiagnostics-color=auto)

target_compile_definitions(std-lib PRIVATE -DTEST_ENV)

target_compile_options(test-std PRIVATE ${C_FLAGS})
target_compile_options(std-lib PRIVATE ${C_FLAGS})

target_link_libraries(test-std PRIVATE
    std-lib
    ${CHECK_LIBRARIES}
    Threads::Threads
    m
)

# Register with CTest
add_test(NAME test-std COMMAND test-std)