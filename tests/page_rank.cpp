#define BOOST_TEST_MODULE PAGE_RANK
#include <boost/test/included/unit_test.hpp>
#include <boost/numeric/ublas/io.hpp>
#include "../include/singularity.hpp"
#include "../include/page_rank_alt.hpp"

using namespace singularity;
using namespace boost;
using namespace boost::numeric::ublas;

vector_t get_priority()
{
    vector_t p(5);
    
    p(0) = 1;
    p(1) = 10;
    p(2) = 3;
    p(3) = 1;
    p(4) = 1;
    
    return p;
}

vector_t get_result()
{
    vector_t p(5);
/*
1.6346
3.3067
4.0616
3.9386
3.0585
*/  
    p(0) = 1.6346;
    p(1) = 3.3067;
    p(2) = 4.0616;
    p(3) = 3.9386;
    p(4) = 3.0585;
    
    return p;
}

std::shared_ptr<vector_based_matrix<double_type> > get_am()
{
    vector_t lg(5), rg(5);
    
    lg(0) = 0.2;
    lg(1) = 0.2;
    lg(2) = 0.2;
    lg(3) = 0.2;
    lg(4) = 0.2;
    
    rg(0) = 0;
    rg(1) = 0;
    rg(2) = 0;
    rg(3) = 0;
    rg(4) = 1;
    
    return std::make_shared<vector_based_matrix<double_type> >(lg, rg);
}

matrix_t get_m()
{
    matrix_t m(5,5);
    
/*    
0	0	0	3	1
5	0	0	0	1
2	1	0	0	1
0	0	1	0	1
1	0	0	7	1    
*/
    m(0, 3) = 3;
    m(1, 0) = 5;
    m(2, 0) = 2;
    m(2, 1) = 1;
    m(3, 2) = 1;
    m(4, 0) = 1;
    m(4, 3) = 7;
    
    
    return m;
}

void normalize(matrix_t& m)
{
    vector_t sum(m.size2(), 0);
    
    for(auto i=m.cbegin1(); i != m.cend1(); i++) {
       for(auto j=i.cbegin(); j != i.cend(); j++)  {
            sum(j.index2()) += *j;
        }
    }
    
    std::cout << sum << std::endl;

    for(auto i=m.begin1(); i != m.end1(); i++) {
       for(auto j=i.begin(); j != i.end(); j++)  {
           if(sum(j.index2()) > 0) {
              *j = *j / sum(j.index2());
           }
        }
    }
}

void page_rank_check(rank_interface& pr, double_type precision)
{
    auto p = get_priority();
    
    auto am = get_am();
    
    additional_matrices_vector amv;
    
    amv.push_back(am);
    
    auto m = get_m();

    normalize(m);

    auto result = pr.process(m, p, p, amv);
    
    auto etalon = get_result();
    
    BOOST_CHECK (boost::numeric::ublas::detail::equals(*result, etalon, precision, precision));
};

BOOST_AUTO_TEST_SUITE( page_rank_test)

BOOST_AUTO_TEST_CASE( test1 )
{
    double_type precision(0.001);

    page_rank pr(0.8, 1, precision);
    
    page_rank_check(pr, precision);
};

BOOST_AUTO_TEST_CASE( test2 )
{
    double_type precision(0.001);
    
    page_rank_alt pr(0.8, 1, precision);
    
    page_rank_check(pr, precision);
}


BOOST_AUTO_TEST_SUITE_END()
