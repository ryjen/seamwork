add_library(seamwork_yahtsee_domain INTERFACE)
target_compile_features(seamwork_yahtsee_domain INTERFACE cxx_std_23)
target_include_directories(seamwork_yahtsee_domain
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_executable(seamwork_yahtsee_domain_test tests.cpp)
target_link_libraries(seamwork_yahtsee_domain_test PRIVATE seamwork_yahtsee_domain)
target_compile_features(seamwork_yahtsee_domain_test PRIVATE cxx_std_23)
set_target_properties(seamwork_yahtsee_domain_test PROPERTIES CXX_EXTENSIONS OFF)

if(SEAMWORK_BUILD_TESTS)
    add_test(NAME seamwork_yahtsee_domain COMMAND seamwork_yahtsee_domain_test)
endif()
