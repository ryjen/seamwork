add_library(seamwork_standard_format_study INTERFACE)
target_compile_features(seamwork_standard_format_study INTERFACE cxx_std_23)
target_include_directories(seamwork_standard_format_study
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_executable(seamwork_standard_format_example example.cpp)
target_link_libraries(seamwork_standard_format_example PRIVATE seamwork_standard_format_study)
target_compile_features(seamwork_standard_format_example PRIVATE cxx_std_23)
set_target_properties(seamwork_standard_format_example PROPERTIES CXX_EXTENSIONS OFF)

add_executable(seamwork_standard_format_test tests.cpp)
target_link_libraries(seamwork_standard_format_test PRIVATE seamwork_standard_format_study)
target_compile_features(seamwork_standard_format_test PRIVATE cxx_std_23)
set_target_properties(seamwork_standard_format_test PROPERTIES CXX_EXTENSIONS OFF)

if(SEAMWORK_BUILD_TESTS)
    add_test(NAME seamwork_standard_format COMMAND seamwork_standard_format_test)
endif()

add_custom_target(seamwork_standard_format_check
    COMMAND $<TARGET_FILE:seamwork_standard_format_test>
    DEPENDS seamwork_standard_format_test
    USES_TERMINAL
)
