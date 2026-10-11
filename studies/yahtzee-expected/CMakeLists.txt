add_library(seamwork_yahtzee_expected INTERFACE)
target_compile_features(seamwork_yahtzee_expected INTERFACE cxx_std_23)
target_include_directories(seamwork_yahtzee_expected
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_executable(seamwork_yahtzee_expected_example example.cpp)
target_link_libraries(seamwork_yahtzee_expected_example PRIVATE seamwork_yahtzee_expected)
target_compile_features(seamwork_yahtzee_expected_example PRIVATE cxx_std_23)
set_target_properties(seamwork_yahtzee_expected_example PROPERTIES CXX_EXTENSIONS OFF)

add_executable(seamwork_yahtzee_expected_test tests.cpp)
target_link_libraries(seamwork_yahtzee_expected_test PRIVATE seamwork_yahtzee_expected)
target_compile_features(seamwork_yahtzee_expected_test PRIVATE cxx_std_23)
set_target_properties(seamwork_yahtzee_expected_test PROPERTIES CXX_EXTENSIONS OFF)

if(SEAMWORK_BUILD_TESTS)
    add_test(NAME seamwork_yahtzee_expected COMMAND seamwork_yahtzee_expected_test)
endif()

add_custom_target(seamwork_yahtzee_expected_check
    COMMAND $<TARGET_FILE:seamwork_yahtzee_expected_test>
    DEPENDS seamwork_yahtzee_expected_test
    USES_TERMINAL
)
