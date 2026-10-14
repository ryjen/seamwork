find_package(Threads REQUIRED)

add_library(seamwork_structured_delayed_work_study INTERFACE)
target_compile_features(seamwork_structured_delayed_work_study INTERFACE cxx_std_23)
target_include_directories(seamwork_structured_delayed_work_study
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(
    seamwork_structured_delayed_work_study
    INTERFACE Threads::Threads
)

add_executable(seamwork_structured_delayed_work_example example.cpp)
target_link_libraries(
    seamwork_structured_delayed_work_example
    PRIVATE seamwork_structured_delayed_work_study
)
target_compile_features(seamwork_structured_delayed_work_example PRIVATE cxx_std_23)
set_target_properties(
    seamwork_structured_delayed_work_example
    PROPERTIES CXX_EXTENSIONS OFF
)

add_executable(seamwork_structured_delayed_work_test tests.cpp)
target_link_libraries(
    seamwork_structured_delayed_work_test
    PRIVATE seamwork_structured_delayed_work_study
)
target_compile_features(seamwork_structured_delayed_work_test PRIVATE cxx_std_23)
set_target_properties(
    seamwork_structured_delayed_work_test
    PROPERTIES CXX_EXTENSIONS OFF
)

if(SEAMWORK_BUILD_TESTS)
    add_test(
        NAME seamwork_structured_delayed_work
        COMMAND seamwork_structured_delayed_work_test
    )
    set_tests_properties(
        seamwork_structured_delayed_work PROPERTIES TIMEOUT 30
    )
endif()

add_custom_target(seamwork_structured_delayed_work_check
    COMMAND $<TARGET_FILE:seamwork_structured_delayed_work_test>
    DEPENDS seamwork_structured_delayed_work_test
    USES_TERMINAL
)
