add_library(seamwork_percent_decoding_study INTERFACE)
target_compile_features(seamwork_percent_decoding_study INTERFACE cxx_std_23)
target_include_directories(seamwork_percent_decoding_study
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_executable(seamwork_percent_decoding_example example.cpp)
target_link_libraries(seamwork_percent_decoding_example PRIVATE seamwork_percent_decoding_study)
target_compile_features(seamwork_percent_decoding_example PRIVATE cxx_std_23)
set_target_properties(seamwork_percent_decoding_example PROPERTIES CXX_EXTENSIONS OFF)

add_executable(seamwork_percent_decoding_test tests.cpp)
target_link_libraries(seamwork_percent_decoding_test PRIVATE seamwork_percent_decoding_study)
target_compile_features(seamwork_percent_decoding_test PRIVATE cxx_std_23)
set_target_properties(seamwork_percent_decoding_test PROPERTIES CXX_EXTENSIONS OFF)

if(SEAMWORK_BUILD_TESTS)
    add_test(NAME seamwork_percent_decoding COMMAND seamwork_percent_decoding_test)
endif()

add_custom_target(seamwork_percent_decoding_check
    COMMAND $<TARGET_FILE:seamwork_percent_decoding_test>
    DEPENDS seamwork_percent_decoding_test
    USES_TERMINAL
)
