include(FetchContent)
set(SAGE_TESTS "sage_tests")

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.18.0
)
FetchContent_MakeAvailable(googletest)

add_executable(
    ${SAGE_TESTS}
    # Targets
    core_test.cpp
    io/output_default_test.cpp
    #allocator/arena_test.cpp
    #allocator/pool_test.cpp
)

target_link_libraries(
    ${SAGE_TESTS}
    PRIVATE ${SAGE_LIB}
            GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(${SAGE_TESTS})
