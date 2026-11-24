# CMake 3.24+ honours this policy for URL downloads; older versions ignore it.
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)
find_package(GTest QUIET)

if(NOT GTest_FOUND)
    set(SIMCORE_GTEST_VERSION 1.15.2)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v${SIMCORE_GTEST_VERSION}.zip
        URL_HASH
            SHA256=f179ec217f9b3b3f3c6e8b02d3e7eda997b49e4ce26d6b235c9053bec9c0bf9f
    )
    set(gtest_force_shared_crt
        ON
        CACHE BOOL "" FORCE)
    set(INSTALL_GTEST
        OFF
        CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

add_executable(simcore_tests registry_test.cpp stepper_test.cpp event_bus_test.cpp)
target_link_libraries(simcore_tests PRIVATE simcore::simcore GTest::gtest_main)
target_compile_features(simcore_tests PRIVATE cxx_std_20)
set_target_properties(simcore_tests PROPERTIES CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(simcore_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(GoogleTest)
# PRE_TEST defers test discovery to ctest time. The default POST_BUILD mode runs
# the freshly built test executable during the build, which fails on machines
# whose security policy blocks executing newly compiled binaries.
gtest_discover_tests(simcore_tests DISCOVERY_MODE PRE_TEST)

# T5: the target is excluded from the normal build on purpose; ctest builds it
# and requires the compilation to fail with the Entity concept diagnostic.
add_executable(simcore_concept_violation EXCLUDE_FROM_ALL concept_violation.cpp)
target_link_libraries(simcore_concept_violation PRIVATE simcore::simcore)
target_compile_features(simcore_concept_violation PRIVATE cxx_std_20)

add_test(
    NAME simcore.concept.rejects_non_entity
    COMMAND
        ${CMAKE_COMMAND} -DBUILD_DIR=${CMAKE_BINARY_DIR}
        -DTARGET=simcore_concept_violation -DCONFIG=$<CONFIG> -P
        ${CMAKE_CURRENT_SOURCE_DIR}/expect_compile_failure.cmake)
