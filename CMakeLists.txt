project( singularity )

cmake_minimum_required( VERSION 2.8 )

if ( Boost_INCLUDE_DIR )
MESSAGE("BOOST ROOT SET")
set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")
set(BOOST_COMPONENTS)
list(APPEND BOOST_COMPONENTS thread
                             date_time
                             filesystem
                             system
                             chrono
                             program_options
                             unit_test_framework
                             locale
			     serialization)

find_package(Boost 1.67 REQUIRED COMPONENTS ${BOOST_COMPONENTS})
set( Boost_USE_STATIC_LIBS ON CACHE STRING "ON or OFF" )
else()
find_package( Boost 1.58 REQUIRED )
endif()

file(GLOB HEADERS "include/*.hpp")

set(SOURCES ncd_aware_rank.cpp
            page_rank.cpp
            activity_index_calculator.cpp
            social_index_calculator.cpp
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

if ( Boost_INCLUDE_DIR )
target_include_directories( singularity PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" ${Boost_INCLUDE_DIR} )
else()
target_include_directories( singularity PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
endif()

install( FILES ${HEADERS} DESTINATION "include" )

add_subdirectory( tests )
# add_subdirectory( benchmarks )
if ( Boost_INCLUDE_DIR )
target_link_libraries( singularity pthread ${Boost_LIBRARIES} )
else()
target_link_libraries( singularity boost_serialization pthread )
endif()
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
