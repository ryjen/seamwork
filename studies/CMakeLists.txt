add_subdirectory(yahtsee-randomness)
add_subdirectory(yahtsee-expected)
add_subdirectory(yahtsee-turn-state)
add_subdirectory(standard-format)
add_subdirectory(borrowed-frame)
add_subdirectory(transaction-lifetime)
add_subdirectory(authority-transport)
add_subdirectory(native-size-boundary)
add_subdirectory(percent-decoding)

# Each C++23 study supplies an executable check, independent of optional CTest
# registration, so CI can validate the implemented contract explicitly.
add_custom_target(seamwork_study_checks)
add_dependencies(
    seamwork_study_checks
    seamwork_yahtsee_randomness_check
    seamwork_yahtsee_expected_check
    seamwork_yahtsee_turn_state_check
    seamwork_standard_format_check
    seamwork_borrowed_frame_check
    seamwork_transaction_lifetime_check
    seamwork_authority_transport_check
    seamwork_native_size_boundary_check
    seamwork_percent_decoding_check
)
