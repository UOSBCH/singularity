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
    std::vector<std::shared_ptr<relation_t> > transactions;

    money_t account_balance = 100000 * params.precision;
    
    time_t now = time(nullptr);
    
    transactions.push_back(std::make_shared<transaction_t> (2000000000, 0, "account-0", "account-1", now, account_balance, account_balance, 0));
    transactions.push_back(std::make_shared<transaction_t> (1000000000, 0, "account-1", "account-0", now, account_balance, account_balance, 0));
    transactions.push_back(std::make_shared<transaction_t> (3000000000, 0, "account-0", "account-2", now, account_balance, account_balance, 0));
    transactions.push_back(std::make_shared<transaction_t> (5000000000, 0, "account-2", "account-1", now, account_balance, account_balance, 0));
    transactions.push_back(std::make_shared<transaction_t> (7000000000, 0, "account-1", "account-2", now, account_balance, account_balance, 0));

//     transactions.push_back( transaction_t (1000000000, 0, "account-1", "account-0", now, account_balance, account_balance));
//     transactions.push_back( transaction_t (3000000000, 0, "account-0", "account-2", now, account_balance, account_balance));
//     transactions.push_back( transaction_t (5000000000, 0, "account-2", "account-1", now, account_balance, account_balance));
//     transactions.push_back( transaction_t (7000000000, 0, "account-1", "account-2", now, account_balance, account_balance));
    
    return transactions;
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

BOOST_AUTO_TEST_SUITE( activity_index_calculator_test)

BOOST_AUTO_TEST_CASE( test1 )
{
    parameters_t params;
    
    params.transaction_amount_threshold = 10;

    auto calculator = rank_calculator_factory::create_calculator_for_transfer(params);

    auto transactions = get_transactions(params);
    
    calculator->add_block(transactions);
    auto r = calculator->calculate();
    auto ar = r[node_type::ACCOUNT];
    
    BOOST_CHECK_CLOSE(static_cast<double>(ar->at("account-0")), 0.201253 /*0.220436*/, 1e-3);
    BOOST_CHECK_CLOSE(static_cast<double>(ar->at("account-1")), 0.249181 /*0.259471*/, 1e-3);
    BOOST_CHECK_CLOSE(static_cast<double>(ar->at("account-2")), 0.549566 /*0.520092*/, 1e-3);
    
    BOOST_CHECK_EQUAL(calculator->get_total_handled_block_count(), 1);
}

BOOST_AUTO_TEST_CASE( test2 )
{
    parameters_t params;

    params.transaction_amount_threshold = 10;

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


// BOOST_AUTO_TEST_CASE( test3 )
// {
//     parameters_t params;
// 
//     auto calculator = rank_calculator_factory::create_calculator_for_transfer(params);
// 
//     add_random_transactions(*calculator, 100, 10, 100, 1000);
//     
//     auto r = calculator->calculate();
//     auto ar = r[node_type::ACCOUNT];
//     
//     char buffer[] = "/tmp/grvXXXXXX\0";
//     
//     mkstemp(buffer);
//     std::string filename(buffer);
//     
//     calculator->save_state_to_file(filename);
//     
//     auto calculator2 = rank_calculator_factory::create_calculator_for_transfer(params);
//     
//     calculator2->load_state_from_file(filename);
//     auto r2 = calculator2->calculate();
//     auto ar2 = r2[node_type::ACCOUNT];
//     
//     remove(filename.c_str());
// 
//     BOOST_CHECK_EQUAL(calculator->get_total_handled_block_count(), calculator2->get_total_handled_block_count());
// 
//     std::string r_str, r2_str;
//     for (auto i = ar->cbegin(); i != ar->cend(); i++) {
//         r_str += i->first + "=" + to_string(i->second) + ";";
//     }
//     for (auto i2 = ar2->cbegin(); i2 != ar2->cend(); i2++) {
//         r2_str += i2->first + "=" + to_string(i2->second) + ";";
//     }
// 
//     BOOST_CHECK_EQUAL(r_str, r2_str);
// }


BOOST_AUTO_TEST_SUITE_END()
