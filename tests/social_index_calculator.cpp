#define BOOST_TEST_MODULE SOCIAL_INDEX_CALCULATOR
#include <boost/test/included/unit_test.hpp>
#include "../include/social_index_calculator.hpp"
#include <stdlib.h>
#include <iostream>
#include <boost/numeric/ublas/io.hpp>
#include <ctime>
#include <fstream>
#include <stdio.h>

using namespace singularity;
using namespace boost;
using namespace boost::numeric::ublas;

std::vector<std::shared_ptr<relation_t> > get_relations(parameters_t params)
{
    std::vector<std::shared_ptr<relation_t> > relations;

    relations.push_back( std::make_shared<like_t>("account-0", "account-1"));
    relations.push_back( std::make_shared<like_t>("account-1", "account-0"));
    relations.push_back( std::make_shared<like_t>("account-0", "account-2"));
    relations.push_back( std::make_shared<follow_t>("account-2", "account-1"));
    relations.push_back( std::make_shared<trust_t>("account-1", "account-2"));
    
    return relations;
}

BOOST_AUTO_TEST_SUITE( social_index_calculator_test)

BOOST_AUTO_TEST_CASE( test1 )
{
    parameters_t params;
    
    social_index_calculator calculator(params);

    std::vector<std::shared_ptr<relation_t> > relations = get_relations(params);
    
    calculator.add_block(relations);
    account_activity_index_map_t r = calculator.calculate();
    
    BOOST_CHECK_CLOSE(r["account-0"], 0.1281639, 1e-3);
    BOOST_CHECK_CLOSE(r["account-1"], 0.4451742, 1e-3);
    BOOST_CHECK_CLOSE(r["account-2"], 0.4266618, 1e-3);
    
    BOOST_CHECK_EQUAL(calculator.get_total_handled_block_count(), 1);
}


BOOST_AUTO_TEST_SUITE_END()
