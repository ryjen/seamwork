add_subdirectory(yahtzee-randomness)
add_subdirectory(yahtzee-expected)

# Release CI keeps legacy aggregate tests disabled, but Seamwork studies still
# need executable evidence. Each study contributes a check target here.
add_custom_target(seamwork_study_checks)
add_dependencies(
    seamwork_study_checks
    seamwork_yahtzee_randomness_check
    seamwork_yahtzee_expected_check
)
