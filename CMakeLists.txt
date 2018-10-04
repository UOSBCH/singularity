project( singularity )

cmake_minimum_required( VERSION 2.8 )

find_package( Boost 1.58 REQUIRED)

if(Boost_FOUND)
   include_directories(${Boost_INCLUDE_DIRS})
endif()

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

target_include_directories( singularity PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( FILES ${HEADERS} DESTINATION "include" )

add_subdirectory( tests )
# add_subdirectory( benchmarks )

target_link_libraries( singularity boost_serialization pthread )

enable_testing ()

execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/git_version.hpp.in
  ${CMAKE_CURRENT_SOURCE_DIR}/generated/git_version.hpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/generated)
