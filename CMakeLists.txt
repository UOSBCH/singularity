project( singularity )

cmake_minimum_required( VERSION 2.8 )

find_package( Boost 1.58 EXACT REQUIRED)

file(GLOB HEADERS "include/*.hpp")

set(SOURCES ncd_aware_rank.cpp
            page_rank.cpp
            activity_index_calculator.cpp
            scan.cpp
            utils.cpp
            emission.cpp
            gravity_index_calculator.cpp
   )

set( CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -Wall" )

add_library( singularity ${SOURCES} ${HEADERS} )

install( TARGETS
   singularity

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
install( FILES ${HEADERS} DESTINATION "include" )

add_subdirectory( tests )
# add_subdirectory( benchmarks )

target_link_libraries( singularity boost_serialization pthread )

enable_testing ()
