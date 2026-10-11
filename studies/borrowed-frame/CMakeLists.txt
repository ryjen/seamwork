add_library(seamwork_borrowed_frame_study INTERFACE)
target_compile_features(seamwork_borrowed_frame_study INTERFACE cxx_std_23)
target_include_directories(seamwork_borrowed_frame_study
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_executable(seamwork_borrowed_frame_example example.cpp)
target_link_libraries(seamwork_borrowed_frame_example PRIVATE seamwork_borrowed_frame_study)
target_compile_features(seamwork_borrowed_frame_example PRIVATE cxx_std_23)
set_target_properties(seamwork_borrowed_frame_example PROPERTIES CXX_EXTENSIONS OFF)

add_executable(seamwork_borrowed_frame_test tests.cpp)
target_link_libraries(seamwork_borrowed_frame_test PRIVATE seamwork_borrowed_frame_study)
target_compile_features(seamwork_borrowed_frame_test PRIVATE cxx_std_23)
set_target_properties(seamwork_borrowed_frame_test PROPERTIES CXX_EXTENSIONS OFF)

if(SEAMWORK_BUILD_TESTS)
    add_test(NAME seamwork_borrowed_frame COMMAND seamwork_borrowed_frame_test)
endif()

add_custom_target(seamwork_borrowed_frame_check
    COMMAND $<TARGET_FILE:seamwork_borrowed_frame_test>
    DEPENDS seamwork_borrowed_frame_test
    USES_TERMINAL
)
