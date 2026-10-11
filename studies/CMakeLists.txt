add_subdirectory(yahtzee-randomness)
add_subdirectory(yahtzee-expected)

# Each C++23 study supplies an executable check, independent of optional CTest
# registration, so CI can validate the implemented contract explicitly.
add_custom_target(seamwork_study_checks)
add_dependencies(
    seamwork_study_checks
    seamwork_yahtzee_randomness_check
    seamwork_yahtzee_expected_check
)
