add_subdirectory(yahtsee-randomness)
add_subdirectory(yahtsee-expected)
add_subdirectory(yahtsee-turn-state)

# Each C++23 study supplies an executable check, independent of optional CTest
# registration, so CI can validate the implemented contract explicitly.
add_custom_target(seamwork_study_checks)
add_dependencies(
    seamwork_study_checks
    seamwork_yahtsee_randomness_check
    seamwork_yahtsee_expected_check
    seamwork_yahtsee_turn_state_check
)
