set(TEST_DIR ${CMAKE_SOURCE_DIR}/Test)

enable_testing()

find_package(PkgConfig REQUIRED)
pkg_check_modules(CHECK REQUIRED check)

find_package(Threads REQUIRED)

target_compile_options(base-lib PRIVATE
    -Wall -Wextra
    $<$<CONFIG:Debug>:-O0 -g>
    $<$<CONFIG:Release>:-O3>
)

target_compile_definitions(base-lib PRIVATE
    -DTEST_ENV
)

function(make_test TEST_NAME)
    add_executable(${TEST_NAME}
        ${TEST_DIR}/main.c
        ${TEST_DIR}/TestRegistry.c
        ${ARGN}
    )

    target_include_directories(${TEST_NAME} PRIVATE
        ${TEST_DIR}
        ${CHECK_INCLUDE_DIRS}
        ${INC_DIR}
    )

    target_compile_options(${TEST_NAME} PRIVATE
        ${CHECK_CFLAGS_OTHER}
        -Wall -Wextra
    )

    target_compile_options(${TEST_NAME} PRIVATE
        $<$<CONFIG:Debug>:-O0 -g>
        $<$<CONFIG:Release>:-O3>
    )

    target_link_libraries(${TEST_NAME} PRIVATE
        base-lib
        ${CHECK_LIBRARIES}
        Threads::Threads
        m
    )

    target_compile_definitions(${TEST_NAME} PRIVATE
        -DTEST_NAME=${TEST_NAME}
        -DTEST_ENV
    )

    add_test(
        NAME ${TEST_NAME}
        COMMAND ${TEST_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endfunction()

make_test(test_hash_map
    ${TEST_DIR}/HashMap/HashMap_test.c
)

add_executable(base-bench
    ${TEST_DIR}/bench_main.c
)

target_include_directories(base-bench PRIVATE
    ${TEST_DIR}
    ${CHECK_INCLUDE_DIRS}
    ${INC_DIR}
)

target_compile_options(base-bench PRIVATE
    ${CHECK_CFLAGS_OTHER}
    -Wall -Wextra
    $<$<CONFIG:Debug>:-O0 -g>
    $<$<CONFIG:Release>:-O3>
)

target_compile_definitions(base-bench PRIVATE
    -DTEST_ENV
)

target_link_libraries(base-bench PRIVATE
    base-lib
    ${CHECK_LIBRARIES}
    Threads::Threads
    m
)