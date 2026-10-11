find_package(Threads REQUIRED)

add_library(seamwork_spsc_queue_study INTERFACE)
target_compile_features(seamwork_spsc_queue_study INTERFACE cxx_std_23)
target_include_directories(
    seamwork_spsc_queue_study
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(
    seamwork_spsc_queue_study
    INTERFACE Threads::Threads
)

add_executable(seamwork_spsc_queue_example example.cpp)
target_link_libraries(
    seamwork_spsc_queue_example
    PRIVATE seamwork_spsc_queue_study
)
target_compile_features(seamwork_spsc_queue_example PRIVATE cxx_std_23)
set_target_properties(
    seamwork_spsc_queue_example
    PROPERTIES CXX_EXTENSIONS OFF
)

add_executable(seamwork_spsc_queue_test tests.cpp)
target_link_libraries(
    seamwork_spsc_queue_test
    PRIVATE seamwork_spsc_queue_study
)
target_compile_features(seamwork_spsc_queue_test PRIVATE cxx_std_23)
set_target_properties(
    seamwork_spsc_queue_test
    PROPERTIES CXX_EXTENSIONS OFF
)

if(SEAMWORK_BUILD_TESTS)
    add_test(NAME seamwork_spsc_queue COMMAND seamwork_spsc_queue_test)
endif()

add_custom_target(seamwork_spsc_queue_check
    COMMAND $<TARGET_FILE:seamwork_spsc_queue_test>
    DEPENDS seamwork_spsc_queue_test
    USES_TERMINAL
)
