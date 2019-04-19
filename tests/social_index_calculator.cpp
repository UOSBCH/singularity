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

void run_test(
    parameters_t params, 
    std::vector<std::vector<std::shared_ptr<relation_t> > > blocks, 
    std::map<std::string, double_type> *p_stack,
    std::map<std::string, double_type> expected_account_rank,
    std::map<std::string, double_type> expected_content_rank,
    std::map<std::string, double_type> *p_priorities
)
{
    const double precision = 1e-2;
    auto p_calculator = rank_calculator_factory::create_calculator_for_social_network(params);
    
    for(auto block: blocks) {
        p_calculator->add_block(block);
    }
    if (p_stack) {
        p_calculator->add_stack_vector(*p_stack);
    }
    if (p_priorities) {
        p_calculator->set_priorities(*p_priorities);
    }
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
    
    auto trusts = p_calculator->get_last_intermediate_results().trust;
    auto priorities = p_calculator->get_last_intermediate_results().priority;
    
    for(auto v: trusts) {
        std::cout << "TRUSTS" << std::endl;
        std::cout << v.first << ":" << v.second << std::endl;
    }
    for(auto v: priorities) {
        std::cout << "PRIORITIES" << std::endl;
        std::cout << v.first << ":" << v.second << std::endl;
    }
    
    BOOST_CHECK_CLOSE (account_norm, 1, precision);
};

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

std::vector<std::shared_ptr<relation_t> > get_relations_4()
{
    return {
        std::make_shared<ownwership_t>("account-0", "post-0", 0),
        std::make_shared<ownwership_t>("account-1", "post-1", 0),
        std::make_shared<ownwership_t>("account-2", "post-2", 0),
        std::make_shared<ownwership_t>("account-3", "post-3", 0),
        std::make_shared<ownwership_t>("account-4", "post-4", 0),
        std::make_shared<ownwership_t>("account-5", "post-5", 0),
        std::make_shared<upvote_t>("account-0", "post-1", 0),
        std::make_shared<upvote_t>("account-1", "post-0", 0),
        std::make_shared<upvote_t>("account-0", "post-2", 0),
        std::make_shared<upvote_t>("account-3", "post-4", 0),
        std::make_shared<upvote_t>("account-4", "post-5", 0),
        std::make_shared<upvote_t>("account-5", "post-3", 0),
    };
}

std::vector<std::shared_ptr<relation_t> > get_trust_relations_4()
{
    return {
        std::make_shared<trust_t>("account-0", "account-1", 0),
        std::make_shared<trust_t>("account-0", "account-2", 0),
    };
}


std::vector<std::shared_ptr<relation_t> > get_relations_for_2_subnets()
{
    return {
        std::make_shared<ownwership_t>("account-10", "post-10-0", 0),
        std::make_shared<ownwership_t>("account-11", "post-11-0", 0),
        std::make_shared<ownwership_t>("account-12", "post-12-0", 0),
        std::make_shared<ownwership_t>("account-20", "post-20-0", 0),
        std::make_shared<ownwership_t>("account-21", "post-21-0", 0),
        std::make_shared<ownwership_t>("account-22", "post-22-0", 0),
        std::make_shared<upvote_t>("account-10", "post-11-0", 0),
        std::make_shared<upvote_t>("account-11", "post-10-0", 0),
        std::make_shared<upvote_t>("account-10", "post-12-0", 0),
        std::make_shared<upvote_t>("account-12", "post-11-0", 0),
        std::make_shared<upvote_t>("account-20", "post-22-0", 0),
        std::make_shared<upvote_t>("account-21", "post-20-0", 0),
        std::make_shared<upvote_t>("account-20", "post-22-0", 0),
        std::make_shared<upvote_t>("account-22", "post-21-0", 0),
    };
}

std::map<std::string, double_type> get_stack_1()
{
    return {
        {"account-0", 100000000},
        {"account-1", 0},
        {"account-2", 0},
        {"account-3", 0},
        {"account-4", 0},
        {"account-5", 0},
    };
}

std::map<std::string, double_type> account_rank_1 = {
    {"account-0", 0.35764},
    {"account-1", 0.32118},
    {"account-2", 0.32118}
};

std::map<std::string, double_type> content_rank_1 = {
    {"post-0", 0.32118},
    {"post-1", 0.35764},
    {"post-2", 0.35764}
};

std::map<std::string, double_type> account_rank_2 = {
    {"account-0", 0.50559},
    {"account-1", 0.179016},
    {"account-2", 0.179016},
    {"account-3", 0.045458},
    {"account-4", 0.045458},
    {"account-5", 0.045458},
};

std::map<std::string, double_type> content_rank_2 = {
    {"post-0", 0.179016},
    {"post-1", 0.505592},
    {"post-2", 0.505592},
    {"post-3", 0},
    {"post-4", 0.0454586},
    {"post-5", 0.0454586},
};

std::map<std::string, double_type> account_rank_3 = {
    {"account-0", 0.301789557},
    {"account-1", 0.27102264},
    {"account-2", 0.27102264},
    {"account-3", 0.05205505},
    {"account-4", 0.05205505},
    {"account-5", 0.05205505},
};

std::map<std::string, double_type> content_rank_3 = {
    {"post-0", 0.27102264},
    {"post-1", 0.301789557},
    {"post-2", 0.301789557},
    {"post-3", 0},
    {"post-4", 0.05205505},
    {"post-5", 0.05205505},
};


std::map<std::string, double_type> account_rank_2_subnets = {
    {"account-10", 0.077743},
    {"account-11", 0.088493},
    {"account-12", 0.060486},
    {"account-20", 0.257759},
    {"account-21", 0.257759},
    {"account-22", 0.257759},
};

std::map<std::string, double_type> content_rank_2_subnets = {
    {"post-10-0", 0.088493},
    {"post-11-0", 0.138229},
    {"post-12-0", 0.077742},
    {"post-20-0", 0.257759111},
    {"post-21-0", 0},
    {"post-22-0", 0.257759333},
};

std::map<std::string, double_type> priorities_2_subnets = {
    {"account-10", 0.033333},
    {"account-11", 0.033333},
    {"account-12", 0.033333},
    {"account-20", 0.300000},
    {"account-21", 0.300000},
    {"account-22", 0.300000},
};

BOOST_AUTO_TEST_SUITE( social_index_calculator_test)

BOOST_AUTO_TEST_CASE( test1 )
{
    parameters_t params;
    params.outlink_weight = 0.9;
    
    std::vector<std::vector<std::shared_ptr<relation_t> > > blocks;
    blocks.push_back(get_relations());
    
    run_test(params, blocks, nullptr, account_rank_1, content_rank_1, nullptr);
}

BOOST_AUTO_TEST_CASE( test2 )
{
    parameters_t params;
    params.outlink_weight = 0.9;
    params.weight_contribution = 0.7;
    
    std::vector<std::vector<std::shared_ptr<relation_t> > > blocks;
    blocks.push_back(get_relations_4());
    
    auto stack = get_stack_1();
    
    run_test(params, blocks, &stack, account_rank_2, content_rank_2, nullptr);
}

BOOST_AUTO_TEST_CASE( test3 )
{
    parameters_t params;
    params.outlink_weight = 0.9;
    params.weight_contribution = 0.7;
    
    std::vector<std::vector<std::shared_ptr<relation_t> > > blocks;
    blocks.push_back(get_relations_4());
    blocks.push_back(get_trust_relations_4());
    
    auto stack = get_stack_1();
    
    run_test(params, blocks, &stack, account_rank_3, content_rank_3, nullptr);
}

BOOST_AUTO_TEST_CASE( test4 )
{
    parameters_t params;
    params.outlink_weight = 0.9;
    params.weight_contribution = 0.7;
    
    std::vector<std::vector<std::shared_ptr<relation_t> > > blocks;
    blocks.push_back(get_relations_for_2_subnets());
    
    run_test(params, blocks, nullptr, account_rank_2_subnets, content_rank_2_subnets, &priorities_2_subnets);
}

BOOST_AUTO_TEST_SUITE_END()
