add_library(seamwork_yahtsee_expected INTERFACE)
target_compile_features(seamwork_yahtsee_expected INTERFACE cxx_std_23)
target_include_directories(seamwork_yahtsee_expected
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(seamwork_yahtsee_expected INTERFACE seamwork_yahtsee_domain)

add_executable(seamwork_yahtsee_expected_example example.cpp)
target_link_libraries(seamwork_yahtsee_expected_example PRIVATE seamwork_yahtsee_expected)
target_compile_features(seamwork_yahtsee_expected_example PRIVATE cxx_std_23)
set_target_properties(seamwork_yahtsee_expected_example PROPERTIES CXX_EXTENSIONS OFF)

add_executable(seamwork_yahtsee_expected_test tests.cpp)
target_link_libraries(seamwork_yahtsee_expected_test PRIVATE seamwork_yahtsee_expected)
target_compile_features(seamwork_yahtsee_expected_test PRIVATE cxx_std_23)
set_target_properties(seamwork_yahtsee_expected_test PROPERTIES CXX_EXTENSIONS OFF)

if(SEAMWORK_BUILD_TESTS)
    add_test(NAME seamwork_yahtsee_expected COMMAND seamwork_yahtsee_expected_test)
endif()

add_custom_target(seamwork_yahtsee_expected_check
    COMMAND $<TARGET_FILE:seamwork_yahtsee_expected_test>
    DEPENDS seamwork_yahtsee_expected_test
    USES_TERMINAL
)
