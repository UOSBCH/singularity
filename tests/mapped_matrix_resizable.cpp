
#define BOOST_TEST_MODULE MAPPED_MATRIX_RESIZABLE
#include <boost/test/included/unit_test.hpp>
#include "../include/utils.hpp"
#include <cstdlib>
#include <iostream>
#include <boost/numeric/ublas/io.hpp>

using namespace singularity;

BOOST_AUTO_TEST_SUITE( mapped_matrix_resizable )

BOOST_AUTO_TEST_CASE( resize )
{
    BOOST_TEST_MESSAGE( "resize" );
    
    matrix_t m1(2,2), m2(4,4);
    
    m1(0,0) = 1;
    m1(1,0) = 3;
    m1(1,1) = -2;

    m2(0,0) = 1;
    m2(1,0) = 3;
    m2(1,1) = -2;
    
    m1.resize(4, 4, true);
    
    BOOST_CHECK(boost::numeric::ublas::detail::equals(m1, m2, double_type(0.001), double_type(0.001)));
}

BOOST_AUTO_TEST_CASE( multiply_by_const )
{
    matrix_t m1(2,2), m2(2,2);
    
    m1(0,0) = 1;
    m1(1,0) = 3;
    m1(1,1) = -2;
    
    m2(0,0) = 3;
    m2(1,0) = 9;
    m2(1,1) = -6;
    
    m1 *= 3;
    
    BOOST_CHECK(boost::numeric::ublas::detail::equals(m1, m2, double_type(0.001), double_type(0.001)));
}

BOOST_AUTO_TEST_CASE( divide_by_const )
{
    matrix_t m1(2,2), m2(2,2);
    
    m1(0,0) = 1;
    m1(1,0) = 3;
    m1(1,1) = -2;
    
    m2(0,0) = 0.5;
    m2(1,0) = 1.5;
    m2(1,1) = -1;
    
    m1 /= 2;
    
    BOOST_CHECK(boost::numeric::ublas::detail::equals(m1, m2, double_type(0.001), double_type(0.001)));
}
BOOST_AUTO_TEST_CASE( increase_by_const )
{
    matrix_t m1(2,2), m2(2,2);
    
    m1(0,0) = 1;
    m1(1,0) = 3;
    m1(1,1) = -2;
    
    m2(0,0) = 4;
    m2(1,0) = 6;
    m2(1,1) = 1;
    
    m1 += 3;
    
    BOOST_CHECK(boost::numeric::ublas::detail::equals(m1, m2, double_type(0.001), double_type(0.001)));
}
BOOST_AUTO_TEST_CASE( decrease_by_const )
{
    matrix_t m1(2,2), m2(2,2);
    
    m1(0,0) = 1;
    m1(1,0) = 3;
    m1(1,1) = -2;
    
    m2(0,0) = -1;
    m2(1,0) = 1;
    m2(1,1) = -4;
    
    m1 -= 2;
    
    BOOST_CHECK(boost::numeric::ublas::detail::equals(m1, m2, double_type(0.001), double_type(0.001)));
}

BOOST_AUTO_TEST_SUITE_END()
        
