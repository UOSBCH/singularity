#define BOOST_TEST_MODULE GRAVITY_INDEX_CALCULATOR

#include <boost/test/included/unit_test.hpp>
#include "../include/gravity_index_calculator.hpp"

using namespace singularity;
using namespace boost;

BOOST_AUTO_TEST_SUITE( gravity_index_calculator_test)

BOOST_AUTO_TEST_CASE( test1 ) 
{
    gravity_index_calculator c(0.1, 100000000000);
    
    double_type gravity_index; 
    money_t votes;
    
    gravity_index = c.calculate_index(30000000000, 0.2);
    votes = c.calculate_votes(30000000000, 0.2);
    
    BOOST_CHECK_CLOSE(gravity_index, 0.29, 1e-6);
    BOOST_CHECK_EQUAL(votes, 29000000000);

    gravity_index = c.calculate_index(40000000000, 0.1);
    votes = c.calculate_votes(40000000000, 0.1);
    
    BOOST_CHECK_CLOSE(gravity_index, 0.37, 1e-6);
    BOOST_CHECK_EQUAL(votes, 36999999999);
}

BOOST_AUTO_TEST_SUITE_END()

