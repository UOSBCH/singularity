#define BOOST_TEST_MODULE ACTIVITY_INDEX_CALCULATOR
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



std::vector<std::shared_ptr<relation_t> > get_transactions(parameters_t params)
{
    money_t account_balance = 100000 * params.precision;
    
    time_t now = time(nullptr);
    
    return {
        std::make_shared<transaction_t> (2000000000, 0, "account-0", "account-1", now, account_balance, account_balance, 0),
        std::make_shared<transaction_t> (1000000000, 0, "account-1", "account-0", now, account_balance, account_balance, 0),
        std::make_shared<transaction_t> (3000000000, 0, "account-0", "account-2", now, account_balance, account_balance, 0),
        std::make_shared<transaction_t> (5000000000, 0, "account-2", "account-1", now, account_balance, account_balance, 0),
        std::make_shared<transaction_t> (7000000000, 0, "account-1", "account-2", now, account_balance, account_balance, 0)
    };
}


void add_random_transactions(activity_index_calculator& ic, uint32_t num_accounts, uint32_t num_blocks, uint32_t block_size, double_type max_amount)
{
    time_t now = time(nullptr);
    
    money_t account_balance = 100000 * ic.get_parameters().precision;
    
    for (uint32_t i = 0; i < num_blocks; i++) {
        std::vector<std::shared_ptr<relation_t> > transactions;
        
        for (uint32_t j = 0; j < block_size; j++) {
            std::string src_account  = "A" + std::to_string((int)std::floor(num_accounts * drand48()));
            std::string target_account  = "A" + std::to_string((int)std::floor(num_accounts * drand48()));
	    double_type amount_double = max_amount * double_type( drand48()) * double_type(ic.get_parameters().precision);
            money_t amount = (double) amount_double;
            if (src_account == target_account || amount < 10) {
                continue;
            }
            transactions.push_back( std::make_shared<transaction_t> ( 
                amount, 
                0, 
                src_account, 
                target_account, 
                now, 
                account_balance,
                account_balance,
                0
            ));
        }
        ic.add_block(transactions);
    }
}

std::map<std::string, double_type> account_rank_1 = {
    {"account-0", 0.201253},
    {"account-1", 0.249181},
    {"account-2", 0.549566}
};

void run_test(
    parameters_t params, 
    std::vector<std::shared_ptr<relation_t> > data, 
    std::map<std::string, double_type> expected_account_rank
)
{
    const double precision = 1e-3;
    auto p_calculator = rank_calculator_factory::create_calculator_for_transfer(params);
    p_calculator->add_block(data);
    auto r = p_calculator->calculate();
    
    auto p_account_index_map = r[node_type::ACCOUNT];
    
    double account_norm = 0;
    
    for(auto r: *p_account_index_map) {
        std::string name = r.first;
        double value1 = (double) r.second;
        double value2 = (double) expected_account_rank[name];
        account_norm += value1;
        BOOST_CHECK_CLOSE (value1, value2, precision);
    }

    BOOST_CHECK_CLOSE (account_norm, 1, precision);
};

BOOST_AUTO_TEST_SUITE( activity_index_calculator_test)

BOOST_AUTO_TEST_CASE( test1 )
{
    parameters_t params;
    
    params.transaction_amount_threshold = 10;
    params.account_amount_threshold = 10;

    auto relations = get_transactions(params);
    
    run_test(params, relations, account_rank_1);
}

BOOST_AUTO_TEST_CASE( test2 )
{
    parameters_t params;

    params.transaction_amount_threshold = 10;
    params.account_amount_threshold = 10;

    auto calculator = rank_calculator_factory::create_calculator_for_transfer(params);

    add_random_transactions(*calculator, 1000, 100, 100, 1000);

    auto r = calculator->calculate();
    
    auto ar = r[node_type::ACCOUNT];
    
    double_type r_sum = 0;
    
    for (auto it: *ar) {
        r_sum += it.second;
    }
  
    
    BOOST_CHECK_CLOSE(static_cast<double>(r_sum), 1, 1e-6);
    
    BOOST_CHECK_EQUAL(calculator->get_total_handled_block_count(), 100);
}

BOOST_AUTO_TEST_SUITE_END()
