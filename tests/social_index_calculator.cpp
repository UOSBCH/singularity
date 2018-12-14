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

std::vector<std::shared_ptr<relation_t> > get_relations(parameters_t params)
{
    std::vector<std::shared_ptr<relation_t> > relations;

    relations.push_back( std::make_shared<ownwership_t>("account-0", "post-0", 0));
    relations.push_back( std::make_shared<ownwership_t>("account-1", "post-1", 0));
    relations.push_back( std::make_shared<ownwership_t>("account-2", "post-2", 0));
    relations.push_back( std::make_shared<upvote_t>("account-0", "post-1", 0));
    relations.push_back( std::make_shared<upvote_t>("account-1", "post-0", 0));
    relations.push_back( std::make_shared<upvote_t>("account-0", "post-2", 0));
    relations.push_back( std::make_shared<follow_t>("account-2", "account-1", 0));
    relations.push_back( std::make_shared<trust_t>("account-1", "account-2", 0));
    
    return relations;
}

std::vector<std::shared_ptr<relation_t> > get_relations_2(parameters_t params)
{
    std::vector<std::shared_ptr<relation_t> > relations;

    relations.push_back( std::make_shared<ownwership_t>("account-0", "post-0", 0));
    relations.push_back( std::make_shared<ownwership_t>("account-1", "post-1", 0));
    relations.push_back( std::make_shared<ownwership_t>("account-2", "post-2", 0));
    relations.push_back( std::make_shared<upvote_t>("account-0", "post-1", 0));
    relations.push_back( std::make_shared<upvote_t>("account-1", "post-0", 0));
    relations.push_back( std::make_shared<downvote_t>("account-0", "post-2", 0));
    relations.push_back( std::make_shared<follow_t>("account-2", "account-1", 0));
    relations.push_back( std::make_shared<trust_t>("account-1", "account-2", 0));
    
    return relations;
}

std::vector<std::shared_ptr<relation_t> > get_relations_3(parameters_t params)
{
    std::vector<std::shared_ptr<relation_t> > relations;

    relations.push_back( std::make_shared<ownwership_t>("account-0", "post-0", 0));
    relations.push_back( std::make_shared<ownwership_t>("account-1", "post-1", 0));
    relations.push_back( std::make_shared<ownwership_t>("account-2", "post-2", 0));
    relations.push_back( std::make_shared<ownwership_t>("account-2", "repost-2-1", 0));
    relations.push_back( std::make_shared<repost_t>("repost-2-1", "post-1", 0));
    relations.push_back( std::make_shared<upvote_t>("account-0", "repost-2-1", 0));
    relations.push_back( std::make_shared<upvote_t>("account-1", "post-0", 0));
    relations.push_back( std::make_shared<downvote_t>("account-0", "post-2", 0));
    relations.push_back( std::make_shared<follow_t>("account-2", "account-1", 0));
    relations.push_back( std::make_shared<trust_t>("account-1", "account-2", 0));
    
    return relations;
}

BOOST_AUTO_TEST_SUITE( social_index_calculator_test)

BOOST_AUTO_TEST_CASE( test1 )
{
    parameters_t params;
    params.outlink_weight = 0.9;
    
    auto calculator = rank_calculator_factory::create_calculator_for_social_network(params);

    std::vector<std::shared_ptr<relation_t> > relations = get_relations(params);
    
    calculator->add_block(relations);
    auto r = calculator->calculate();
    
    auto p_account_index_map = r[node_type::ACCOUNT];
    auto p_content_index_map = r[node_type::CONTENT];
    
    BOOST_CHECK_CLOSE((double) p_account_index_map->at("account-0"), 0.39758, 1e-3);
    BOOST_CHECK_CLOSE((double) p_account_index_map->at("account-1"), 0.30121, 1e-3);
    BOOST_CHECK_CLOSE((double) p_account_index_map->at("account-2"), 0.30121, 1e-3);
    BOOST_CHECK_CLOSE((double) p_content_index_map->at("post-0"), 0.30121, 1e-3);
    BOOST_CHECK_CLOSE((double) p_content_index_map->at("post-1"), 0.19879, 1e-3);
    BOOST_CHECK_CLOSE((double) p_content_index_map->at("post-2"), 0.19879, 1e-3);
    
    double_type account_sum = p_account_index_map->at("account-0") + p_account_index_map->at("account-1") + p_account_index_map->at("account-2");
    
    double_type content_sum = p_content_index_map->at("post-0") + p_content_index_map->at("post-1") + p_content_index_map->at("post-2");
    
    double_type total_sum = content_sum + account_sum;
    
    BOOST_CHECK_CLOSE((double) account_sum, 1, 1e-3);

//     BOOST_CHECK_CLOSE((double) content_sum, 1, 1e-3);

//     BOOST_CHECK_CLOSE((double) total_sum, 2, 1e-3);
    
    
    BOOST_CHECK_EQUAL(calculator->get_total_handled_block_count(), 1);
}

BOOST_AUTO_TEST_CASE( test11 )
{
    parameters_t params;
    params.outlink_weight = 0.9;
    
    auto calculator = rank_calculator_factory::create_calculator_for_social_network(params);

    std::vector<std::shared_ptr<relation_t> > relations = get_relations(params);
    
    std::map<std::string, double_type> stack_map;
    
    stack_map["account-0"] = 10000;
    stack_map["account-1"] = 1000;
    stack_map["account-2"] = 1000;
    
    calculator->add_stack_vector(stack_map);
    
    calculator->add_block(relations);
    auto r = calculator->calculate();

    auto p_account_index_map = r[node_type::ACCOUNT];
    auto p_content_index_map = r[node_type::CONTENT];
    
    BOOST_CHECK_CLOSE((double) p_account_index_map->at("account-0"), 0.2543470, 1e-3);
    BOOST_CHECK_CLOSE((double) p_account_index_map->at("account-1"), 0.3728265, 1e-3);
    BOOST_CHECK_CLOSE((double) p_account_index_map->at("account-2"), 0.3728265, 1e-3);
    BOOST_CHECK_CLOSE((double) p_content_index_map->at("post-0"), 0.3728265, 1e-3);
    BOOST_CHECK_CLOSE((double) p_content_index_map->at("post-1"), 0.1271735, 1e-3);
    BOOST_CHECK_CLOSE((double) p_content_index_map->at("post-2"), 0.1271735, 1e-3);
    
    double_type account_sum = p_account_index_map->at("account-0") + p_account_index_map->at("account-1") + p_account_index_map->at("account-2");
    
    double_type content_sum = p_content_index_map->at("post-0") + p_content_index_map->at("post-1") + p_content_index_map->at("post-2");
    
    double_type total_sum = content_sum + account_sum;
    
    BOOST_CHECK_CLOSE((double) account_sum, 1, 1e-3);

//     BOOST_CHECK_CLOSE((double) content_sum, 1, 1e-3);

//     BOOST_CHECK_CLOSE((double) total_sum, 2, 1e-3);
    
    
    BOOST_CHECK_EQUAL(calculator->get_total_handled_block_count(), 1);
}



BOOST_AUTO_TEST_CASE( test2 )
{
    parameters_t params;
    params.outlink_weight = 0.9;
    
    auto calculator = rank_calculator_factory::create_calculator_for_social_network(params);

    std::vector<std::shared_ptr<relation_t> > relations = get_relations_2(params);
    
    calculator->add_block(relations);
    auto r = calculator->calculate();
    
    auto p_account_index_map = r[node_type::ACCOUNT];
    auto p_content_index_map = r[node_type::CONTENT];
    
    BOOST_CHECK_CLOSE((double) p_account_index_map->at("account-0"), 0.47584, 1e-3);
    BOOST_CHECK_CLOSE((double) p_account_index_map->at("account-1"), 0.47584, 1e-3);
    BOOST_CHECK_CLOSE((double) p_account_index_map->at("account-2"), 0.0483133, 1e-3);
    BOOST_CHECK_CLOSE((double) p_content_index_map->at("post-0"), 0.47584, 1e-3);
    BOOST_CHECK_CLOSE((double) p_content_index_map->at("post-1"), 0.47584, 1e-3);
    BOOST_CHECK_CLOSE((double) p_content_index_map->at("post-2"), 0, 1e-3);
    
    double_type account_sum = p_account_index_map->at("account-0") + p_account_index_map->at("account-1") + p_account_index_map->at("account-2");
    
    double_type content_sum = p_content_index_map->at("post-0") + p_content_index_map->at("post-1") + p_content_index_map->at("post-2");
    
    double_type total_sum = content_sum + account_sum;
    
    BOOST_CHECK_CLOSE((double) account_sum, 1, 1e-3);

//     BOOST_CHECK_CLOSE((double) content_sum, 1, 1e-3);

//     BOOST_CHECK_CLOSE((double) total_sum, 2, 1e-3);
    
    
    BOOST_CHECK_EQUAL(calculator->get_total_handled_block_count(), 1);
}

BOOST_AUTO_TEST_CASE( test3 )
{
    parameters_t params;
    params.outlink_weight = 0.9;
    
    auto calculator = rank_calculator_factory::create_calculator_for_social_network(params);

    std::vector<std::shared_ptr<relation_t> > relations = get_relations_3(params);
    
    calculator->add_block(relations);
    auto r = calculator->calculate();
    
    auto p_account_index_map = r[node_type::ACCOUNT];
    auto p_content_index_map = r[node_type::CONTENT];
    
    BOOST_CHECK_CLOSE((double) p_account_index_map->at("account-0"), 0.47584, 1e-3);
    BOOST_CHECK_CLOSE((double) p_account_index_map->at("account-1"), 0.47584, 1e-3);
    BOOST_CHECK_CLOSE((double) p_account_index_map->at("account-2"), 0.0483133, 1e-3);
    BOOST_CHECK_CLOSE((double) p_content_index_map->at("post-0"), 0.47584, 1e-3);
    BOOST_CHECK_CLOSE((double) p_content_index_map->at("post-1"), 0.47584, 1e-3);
    BOOST_CHECK_CLOSE((double) p_content_index_map->at("post-2"), 0, 1e-3);
    
    double_type account_sum = p_account_index_map->at("account-0") + p_account_index_map->at("account-1") + p_account_index_map->at("account-2");
    
    double_type content_sum = p_content_index_map->at("post-0") + p_content_index_map->at("post-1") + p_content_index_map->at("post-2");
    
    double_type total_sum = content_sum + account_sum;
    
    BOOST_CHECK_CLOSE((double) account_sum, 1, 1e-3);

//     BOOST_CHECK_CLOSE((double) content_sum, 1, 1e-3);

//     BOOST_CHECK_CLOSE((double) total_sum, 2, 1e-3);
    
    
    BOOST_CHECK_EQUAL(calculator->get_total_handled_block_count(), 1);
}


BOOST_AUTO_TEST_SUITE_END()
