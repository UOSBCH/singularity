#define BOOST_TEST_MODULE UTILS
#include <boost/test/included/unit_test.hpp>
#include "../include/utils.hpp"
#include <cstdlib>
#include <iostream>
#include <boost/numeric/ublas/io.hpp>
#include "../include/vector_based_matrix.hpp"
#include <boost/numeric/ublas/vector_sparse.hpp>

using namespace singularity;

BOOST_AUTO_TEST_SUITE( utils)

BOOST_AUTO_TEST_CASE( normalize_columns )
{
    BOOST_TEST_MESSAGE( "normalize_columns" );
    
    matrix_t m1(3,3), m2(3,3);
    
    m1(0,0) = 1;
    m1(1,0) = 3;

    m2(0,0) = 0.25;
    m2(1,0) = 0.75;
    
    matrix_tools::normalize_columns(m1);
    
    BOOST_CHECK(boost::numeric::ublas::detail::equals(m1,m2, double_type(0.001), double_type(0.001)));
}

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
    
    std::shared_ptr<matrix_t> p_m1_resized = matrix_tools::resize(m1, 4, 4);
    matrix_t m1_resized = m1;
    m1_resized.resize(4, 4, true);
    
    BOOST_CHECK(boost::numeric::ublas::detail::equals(m1_resized, m2, double_type(0.001), double_type(0.001)));
    BOOST_CHECK(boost::numeric::ublas::detail::equals(*p_m1_resized, m2, double_type(0.001), double_type(0.001)));
}

BOOST_AUTO_TEST_CASE( split_range_1 )
{
    BOOST_TEST_MESSAGE( "split_range_1" );
    
    range_t r(0, 20);
    
    std::vector<range_t> result = matrix_tools::split_range(r, 3);
    
    BOOST_CHECK_EQUAL(result.size(), 3);
    BOOST_CHECK_EQUAL(result[0].size(), 7);
    BOOST_CHECK_EQUAL(result[1].size(), 7);
    BOOST_CHECK_EQUAL(result[2].size(), 6);
}

BOOST_AUTO_TEST_CASE( split_range_2 )
{
    BOOST_TEST_MESSAGE( "split_range_2" );
    
    range_t r(0, 3);
    
    std::vector<range_t> result = matrix_tools::split_range(r, 10);
    
    BOOST_CHECK_EQUAL(result.size(), 3);
    BOOST_CHECK_EQUAL(result[0].size(), 1);
    BOOST_CHECK_EQUAL(result[1].size(), 1);
    BOOST_CHECK_EQUAL(result[2].size(), 1);
}

BOOST_AUTO_TEST_CASE( split_range_3 )
{
    BOOST_TEST_MESSAGE( "split_range_3" );
    
    range_t r(0, 10);
    
    std::vector<range_t> result = matrix_tools::split_range(r, 2);
    
    BOOST_CHECK_EQUAL(result.size(), 2);
    BOOST_CHECK_EQUAL(result[0].size(), 5);
    BOOST_CHECK_EQUAL(result[1].size(), 5);
}

BOOST_AUTO_TEST_CASE( partial_prod )
{
    BOOST_TEST_MESSAGE( "partial_prod" );

    vector_t v(4), r(4);
    matrix_t m(4, 4);
    
    v(0) = 1;
    v(1) = 2;
    v(2) = -1;
    v(3) = 1;
    
    m(0,1) = 1;
    m(1,0) = 1;
    m(2,3) = 1;
    m(3,2) = 1;
    
    r = v;
    
    matrix_tools::partial_prod(r,m,v, range_t(0,2));
    
    BOOST_CHECK_CLOSE(r[0], 2, 0.001);
    BOOST_CHECK_CLOSE(r[1], 1, 0.001);
    BOOST_CHECK_CLOSE(r[2], -1, 0.001);
    BOOST_CHECK_CLOSE(r[3], 1, 0.001);
}

BOOST_AUTO_TEST_CASE( derived_matrix )
{
    boost::numeric::ublas::mapped_vector<int16_t> left(5), right(4);
        
    left(0) = 1;
    left(4) = -1;
    
    right(0) = 1;
    right(1) = 2;
    right(2) = 3;
    right(3) = 4;

    boost::numeric::ublas::vector_based_matrix<int16_t> m(left, right);
    
    std::cout << m << std::endl;
    
    m *= 2;
    
    std::cout << m << std::endl;

    boost::numeric::ublas::vector<int16_t> v(4, 1);
    
    auto r = boost::numeric::ublas::prod(m, v);
    
    std::cout << r << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()
        
