add_executable(test_gravity_index_calculator "gravity_index_calculator.cpp")
add_executable(test_activity_index_calculator "activity_index_calculator.cpp")
add_executable(test_scan "scan.cpp")
add_executable(test_utils "utils.cpp")
add_executable(test_emission "emission.cpp")

target_link_libraries(test_gravity_index_calculator singularity )
target_link_libraries(test_activity_index_calculator singularity )
target_link_libraries(test_scan singularity )
target_link_libraries(test_utils singularity )
target_link_libraries(test_emission singularity )

add_test(NAME test_gravity_index_calculator COMMAND test_gravity_index_calculator)
add_test(NAME test_activity_index_calculator COMMAND test_activity_index_calculator)
add_test(NAME test_scan COMMAND test_scan)
add_test(NAME test_utils COMMAND test_utils)
add_test(NAME test_emission COMMAND test_emission)

