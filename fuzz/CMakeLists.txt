add_library(seamwork_fuzz_options INTERFACE)
target_compile_options(
    seamwork_fuzz_options
    INTERFACE
        -fsanitize=fuzzer,address,undefined
        -fno-omit-frame-pointer
)
target_link_options(
    seamwork_fuzz_options
    INTERFACE
        -fsanitize=fuzzer,address,undefined
)

add_executable(seamwork_fuzz_borrowed_frame borrowed_frame_fuzz.cpp)
target_compile_features(seamwork_fuzz_borrowed_frame PRIVATE cxx_std_23)
target_link_libraries(
    seamwork_fuzz_borrowed_frame
    PRIVATE
        seamwork_borrowed_frame_study
        seamwork_fuzz_options
)
set_target_properties(
    seamwork_fuzz_borrowed_frame
    PROPERTIES CXX_EXTENSIONS OFF
)

add_executable(seamwork_fuzz_percent_decoding percent_decoding_fuzz.cpp)
target_compile_features(seamwork_fuzz_percent_decoding PRIVATE cxx_std_23)
target_link_libraries(
    seamwork_fuzz_percent_decoding
    PRIVATE
        seamwork_percent_decoding_study
        seamwork_fuzz_options
)
set_target_properties(
    seamwork_fuzz_percent_decoding
    PROPERTIES CXX_EXTENSIONS OFF
)
