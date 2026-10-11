add_library(seamwork_yahtsee_randomness INTERFACE)
target_compile_features(seamwork_yahtsee_randomness INTERFACE cxx_std_23)
target_include_directories(seamwork_yahtsee_randomness
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_executable(seamwork_yahtsee_randomness_example example.cpp)
target_link_libraries(seamwork_yahtsee_randomness_example
    PRIVATE
        seamwork_yahtsee_randomness
)
target_compile_features(seamwork_yahtsee_randomness_example PRIVATE cxx_std_23)
set_target_properties(seamwork_yahtsee_randomness_example PROPERTIES CXX_EXTENSIONS OFF)

# Compile deterministic study tests as executable evidence even when test
# registration is disabled.
add_executable(seamwork_yahtsee_randomness_test tests.cpp)
target_link_libraries(seamwork_yahtsee_randomness_test
    PRIVATE
        seamwork_yahtsee_randomness
)
target_compile_features(seamwork_yahtsee_randomness_test PRIVATE cxx_std_23)
set_target_properties(seamwork_yahtsee_randomness_test PROPERTIES CXX_EXTENSIONS OFF)

if(SEAMWORK_BUILD_TESTS)
    add_test(
        NAME seamwork_yahtsee_randomness
        COMMAND seamwork_yahtsee_randomness_test
    )
endif()

add_custom_target(seamwork_yahtsee_randomness_check
    COMMAND $<TARGET_FILE:seamwork_yahtsee_randomness_test>
    DEPENDS seamwork_yahtsee_randomness_test
    USES_TERMINAL
)
