#define BOOST_TEST_MODULE SOCIAL_INDEX_CALCULATOR
#include <boost/test/included/unit_test.hpp>
#include "../include/activity_index_calculator.hpp"
#include "../include/rank_calculator_factory.hpp"
#include <stdlib.h>
#include <iostream>
#include <boost/numeric/ublas/io.hpp>
#include <ctime>
#include <fstream>
#include <stdio.h>

using namespace singularity;
using namespace boost;
using namespace boost::numeric::ublas;

std::vector<std::shared_ptr<relation_t> > get_relations()
{
    return {
        std::make_shared<ownwership_t>("account-0", "post-0", 0),
        std::make_shared<ownwership_t>("account-1", "post-1", 0),
        std::make_shared<ownwership_t>("account-2", "post-2", 0),
        std::make_shared<upvote_t>("account-0", "post-1", 0),
        std::make_shared<upvote_t>("account-1", "post-0", 0),
        std::make_shared<upvote_t>("account-0", "post-2", 0),
        std::make_shared<follow_t>("account-2", "account-1", 0),
        std::make_shared<trust_t>("account-1", "account-2", 0)
    };
}

std::vector<std::shared_ptr<relation_t> > get_relations_2()
{
    return {
        std::make_shared<ownwership_t>("account-0", "post-0", 0),
        std::make_shared<ownwership_t>("account-1", "post-1", 0),
        std::make_shared<ownwership_t>("account-2", "post-2", 0),
        std::make_shared<upvote_t>("account-0", "post-1", 0),
        std::make_shared<upvote_t>("account-1", "post-0", 0),
        std::make_shared<downvote_t>("account-0", "post-2", 0),
        std::make_shared<follow_t>("account-2", "account-1", 0),
        std::make_shared<trust_t>("account-1", "account-2", 0)
    };
}

std::vector<std::shared_ptr<relation_t> > get_relations_3()
{
    return {
        std::make_shared<ownwership_t>("account-0", "post-0", 0),
        std::make_shared<ownwership_t>("account-1", "post-1", 0),
        std::make_shared<ownwership_t>("account-2", "post-2", 0),
        std::make_shared<ownwership_t>("account-2", "repost-2-1", 0),
        std::make_shared<repost_t>("repost-2-1", "post-1", 0),
        std::make_shared<upvote_t>("account-0", "repost-2-1", 0),
        std::make_shared<upvote_t>("account-1", "post-0", 0),
        std::make_shared<downvote_t>("account-0", "post-2", 0),
        std::make_shared<follow_t>("account-2", "account-1", 0),
        std::make_shared<trust_t>("account-1", "account-2", 0)
    };
}

std::vector<std::shared_ptr<relation_t> > get_relations_decayable()
{
    return {
        std::make_shared<ownwership_t>("account-0", "post-0", 0),
        std::make_shared<ownwership_t>("account-1", "post-1", 0),
        std::make_shared<ownwership_t>("account-2", "post-20", 0),
        std::make_shared<ownwership_t>("account-2", "post-21", 0),
        std::make_shared<upvote_t>("account-0", "post-1", 516000),
        std::make_shared<upvote_t>("account-1", "post-0", 0),
        std::make_shared<upvote_t>("account-0", "post-20", 0),
        std::make_shared<upvote_t>("account-0", "post-21", 516000)
    };    
}

std::map<std::string, double_type> account_rank_1 = {
    {"account-0", 0.357490},
    {"account-1", 0.3212549},
    {"account-2", 0.3212549}
};

std::map<std::string, double_type> content_rank_1 = {
    {"post-0", 0.3212549},
    {"post-1", 0.357490},
    {"post-2", 0.357490}
};


void run_test(
    parameters_t params, 
    std::vector<std::shared_ptr<relation_t> > data, 
    std::map<std::string, double_type> expected_account_rank,
    std::map<std::string, double_type> expected_content_rank
)
{
    const double precision = 1e-3;
    auto p_calculator = rank_calculator_factory::create_calculator_for_social_network(params);
    p_calculator->add_block(data);
    auto r = p_calculator->calculate();
    
    auto p_account_index_map = r[node_type::ACCOUNT];
    auto p_content_index_map = r[node_type::CONTENT];
    
    double account_norm = 0;
    double content_norm = 0;
    
    for(auto r: *p_account_index_map) {
        std::string name = r.first;
        double calculated = (double) r.second;
        double expected = (double) expected_account_rank[name];
        account_norm += calculated;
        BOOST_CHECK_CLOSE (calculated, expected, precision);
    }
    for(auto r: *p_content_index_map) {
        std::string name = r.first;
        double calculated = (double) r.second;
        double expected = (double) expected_content_rank[name];
        content_norm += calculated;
        BOOST_CHECK_CLOSE (calculated, expected, precision);
    }
    BOOST_CHECK_CLOSE (account_norm, 1, precision);
};

BOOST_AUTO_TEST_SUITE( social_index_calculator_test)

BOOST_AUTO_TEST_CASE( test1 )
{
    parameters_t params;
    params.outlink_weight = 0.9;
    
    auto relations = get_relations();
    
    run_test(params, relations, account_rank_1, content_rank_1);
}

BOOST_AUTO_TEST_SUITE_END()
