add_executable(simcore_demo particle_demo.cpp)

target_link_libraries(simcore_demo PRIVATE simcore::simcore)
target_compile_features(simcore_demo PRIVATE cxx_std_20)

set_target_properties(simcore_demo PROPERTIES CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(simcore_demo PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(simcore_priority_demo priority_demo.cpp)
target_link_libraries(simcore_priority_demo PRIVATE simcore::simcore)
set_target_properties(simcore_priority_demo PROPERTIES CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(simcore_priority_demo PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(simcore_batch_demo batch_demo.cpp)
target_link_libraries(simcore_batch_demo PRIVATE simcore::simcore)
set_target_properties(simcore_batch_demo PROPERTIES CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(simcore_batch_demo PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(simcore_one_minute_demo one_minute_demo.cpp)
target_link_libraries(simcore_one_minute_demo PRIVATE simcore::simcore)
set_target_properties(simcore_one_minute_demo PROPERTIES CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(simcore_one_minute_demo PRIVATE -Wall -Wextra -Wpedantic)
endif()
