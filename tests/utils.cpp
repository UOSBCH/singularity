#define BOOST_TEST_MODULE UTILS
#include "../include/mapped_matrix_resizable.hpp"
#include <boost/test/included/unit_test.hpp>
#include "../include/utils.hpp"
#include <cstdlib>
#include <iostream>
#include <boost/numeric/ublas/io.hpp>
#include "../include/vector_based_matrix.hpp"
#include <boost/numeric/ublas/vector_sparse.hpp>

using namespace singularity;
using namespace boost::numeric::ublas;

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
    
    BOOST_CHECK(detail::equals(m1,m2, double_type(0.001), double_type(0.001)));
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
    
    BOOST_CHECK(detail::equals(m1_resized, m2, double_type(0.001), double_type(0.001)));
    BOOST_CHECK(detail::equals(*p_m1_resized, m2, double_type(0.001), double_type(0.001)));
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
    
    BOOST_CHECK_CLOSE((double) r[0], 2, 0.001);
    BOOST_CHECK_CLOSE((double) r[1], 1, 0.001);
    BOOST_CHECK_CLOSE((double) r[2], -1, 0.001);
    BOOST_CHECK_CLOSE((double) r[3], 1, 0.001);
}

BOOST_AUTO_TEST_CASE( derived_matrix )
{
    mapped_vector<int16_t> left(5), right(4);
        
    left(0) = 1;
    left(4) = -1;
    
    right(0) = 1;
    right(1) = 2;
    right(2) = 3;
    right(3) = 4;

    vector_based_matrix<int16_t> m(left, right);
    
    m *= 2;

    vector<int16_t> v(4, 1);
    
    auto r = prod(m, v);
}

BOOST_AUTO_TEST_CASE( matrix_prod )
{
    BOOST_TEST_MESSAGE( "matrix_prod" );

    matrix_t expected(3,3), out(3, 3), in1(3,3), in2(3,3);
    
    expected(0,0) = 1;
    expected(1,1) = 1;
    expected(2,2) = 1;
    
    in1(0,0) = 1;
    in1(0,1) = 1;
    in1(0,2) = 1;
    in1(1,0) = 0;
    in1(1,1) = 2;
    in1(1,2) = 3;
    in1(2,0) = 2;
    in1(2,1) = 1;
    in1(2,2) = 0;

    in2(0,0) = 3;
    in2(0,1) = -1;
    in2(0,2) = -1;
    in2(1,0) = -6;
    in2(1,1) = 2;
    in2(1,2) = 3;
    in2(2,0) = 4;
    in2(2,1) = -1;
    in2(2,2) = -2;
    
    matrix_tools::prod(out, in1, in2);
    
    BOOST_CHECK(boost::numeric::ublas::detail::equals(expected, out, double_type(0.001), double_type(0.001)));
}

BOOST_AUTO_TEST_CASE( prod )
{
    matrix_t m(3,3);
    m(0,0) = 1;
    m(0,1) = 1;
    m(0,2) = 1;
    m(1,0) = 0;
    m(1,1) = 2;
    m(1,2) = 3;
    m(2,0) = 2;
    m(2,1) = 1;
    m(2,2) = 0;
    
    vector<double_type> in(3);
    
    in(0) = 1;
    in(1) = 1;
    in(2) = 2;

    vector<double_type> expected(3);

    expected(0) = 4;
    expected(1) = 8;
    expected(2) = 3;
    
    vector<double_type> out = boost::numeric::ublas::prod(m, in);

    BOOST_CHECK(detail::equals(expected, out, double_type(0.001), double_type(0.001)));
}

BOOST_AUTO_TEST_CASE( collapse )
{
    matrix_t left(3,4);
    left(0,0) = 1;
    left(0,1) = 1;
    left(0,3) = 1;
    left(1,2) = 1;

    matrix_t right(4,3);
    
    right(0,0) = 0.8;
    right(1,0) = 0.3;
    right(2,0) = 0.9;
    right(3,0) = 0.1;

    right(0,1) = 0.2;
    right(1,1) = 0.1;
    right(2,1) = 0.3;
    right(3,1) = 0.7;

    right(0,2) = 0.1;
    right(1,2) = 1;
    right(2,2) = 0.5;
    right(3,2) = 0.1;
    
    std::shared_ptr<matrix_t> p_result = boost::numeric::ublas::collapse(left, right);
    
    matrix_t expected(3, 3);
    
    expected(0,0) = 0.8;
    expected(0,1) = 0.7;
    expected(0,2) = 1;

    expected(1,0) = 0.9;
    expected(1,1) = 0.3;
    expected(1,2) = 0.5;
    
    double_type diff = norm_1(expected - *p_result);
    
    BOOST_CHECK_CLOSE(diff, double_type(0), double_type(0.001));
}


BOOST_AUTO_TEST_SUITE_END()
        
