#define BOOST_TEST_MODULE EMISSION
#include <boost/test/included/unit_test.hpp>
#include "../include/emission.hpp"
#include <stdlib.h>
#include <iostream>
#include <boost/numeric/ublas/io.hpp>

using namespace singularity;
using namespace boost;
using namespace boost::numeric::ublas;

std::vector<transaction_t> get_transactions1()
{
    std::vector<transaction_t> transactions;

    time_t now = time(nullptr);
    
    transactions.push_back( transaction_t (200, 0, "account-0", "account-1", now));
    transactions.push_back( transaction_t (100, 0, "account-1", "account-0", now));
    transactions.push_back( transaction_t (300, 0, "account-0", "account-2", now));
    transactions.push_back( transaction_t (500, 0, "account-2", "account-1", now));
    transactions.push_back( transaction_t (700, 0, "account-1", "account-2", now));
    
    return transactions;
}

std::vector<transaction_t> get_transactions2()
{
    std::vector<transaction_t> transactions;

    time_t now = time(nullptr);
    
    transactions.push_back( transaction_t (200, 0, "account-0", "account-1", now));
    transactions.push_back( transaction_t (100, 0, "account-1", "account-0", now));
    transactions.push_back( transaction_t (300, 0, "account-0", "account-2", now));
    transactions.push_back( transaction_t (500, 0, "account-2", "account-1", now));
    transactions.push_back( transaction_t (700, 0, "account-1", "account-2", now));
    transactions.push_back( transaction_t (800, 0, "account-1", "account-3", now));
    transactions.push_back( transaction_t (100, 0, "account-2", "account-3", now));
    
    return transactions;
}

BOOST_AUTO_TEST_SUITE( emission_test )

BOOST_AUTO_TEST_CASE( test1 )
{
    std::vector<transaction_t> transactions = get_transactions1();
    
    activity_period ap;
    
    ap.add_block(transactions);
    
    double activity = ap.get_activity();

    BOOST_CHECK_CLOSE(activity, 3, 1e-3);
}

BOOST_AUTO_TEST_CASE( test2 )
{
    emission_parameters_t params;

    emission_calculator em(params);
    
    uint64_t emission = em.calculate(200000000000, 5000);
    
    BOOST_CHECK_EQUAL(emission, 148262316574);
}

BOOST_AUTO_TEST_SUITE_END()
