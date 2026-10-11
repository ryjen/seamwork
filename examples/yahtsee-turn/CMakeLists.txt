add_library(seamwork_example_yahtsee_turn_lib INTERFACE)
target_compile_features(seamwork_example_yahtsee_turn_lib INTERFACE cxx_std_23)
target_include_directories(seamwork_example_yahtsee_turn_lib
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(seamwork_example_yahtsee_turn_lib
    INTERFACE
        seamwork_yahtsee_randomness
        seamwork_yahtsee_expected
        seamwork_yahtsee_turn_state
)

add_executable(seamwork_example_yahtsee_turn main.cpp)
target_link_libraries(seamwork_example_yahtsee_turn PRIVATE seamwork_example_yahtsee_turn_lib)
target_compile_features(seamwork_example_yahtsee_turn PRIVATE cxx_std_23)
set_target_properties(seamwork_example_yahtsee_turn PROPERTIES CXX_EXTENSIONS OFF)

add_executable(seamwork_example_yahtsee_turn_test tests.cpp)
target_link_libraries(seamwork_example_yahtsee_turn_test PRIVATE seamwork_example_yahtsee_turn_lib)
target_compile_features(seamwork_example_yahtsee_turn_test PRIVATE cxx_std_23)
set_target_properties(seamwork_example_yahtsee_turn_test PROPERTIES CXX_EXTENSIONS OFF)

if(SEAMWORK_BUILD_TESTS)
    add_test(NAME seamwork_example_yahtsee_turn COMMAND seamwork_example_yahtsee_turn_test)
endif()
