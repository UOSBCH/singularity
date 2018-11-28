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
    
    transactions.push_back( transaction_t (200, 0, "account-0", "account-1", now, 0, 0, 0));
    transactions.push_back( transaction_t (100, 0, "account-1", "account-0", now, 0, 0, 0));
    transactions.push_back( transaction_t (300, 0, "account-0", "account-2", now, 0, 0, 0));
    transactions.push_back( transaction_t (500, 0, "account-2", "account-1", now, 0, 0, 0));
    transactions.push_back( transaction_t (700, 0, "account-1", "account-2", now, 0, 0, 0));
    
    return transactions;
}

std::vector<transaction_t> get_transactions2()
{
    std::vector<transaction_t> transactions;

    time_t now = time(nullptr);
    
    transactions.push_back( transaction_t (200, 0, "account-0", "account-1", now, 0, 0, 0));
    transactions.push_back( transaction_t (100, 0, "account-1", "account-0", now, 0, 0, 0));
    transactions.push_back( transaction_t (300, 0, "account-0", "account-2", now, 0, 0, 0));
    transactions.push_back( transaction_t (500, 0, "account-2", "account-1", now, 0, 0, 0));
    transactions.push_back( transaction_t (700, 0, "account-1", "account-2", now, 0, 0, 0));
    transactions.push_back( transaction_t (800, 0, "account-1", "account-3", now, 0, 0, 0));
    transactions.push_back( transaction_t (100, 0, "account-2", "account-3", now, 0, 0, 0));
    
    return transactions;
}

BOOST_AUTO_TEST_SUITE( emission_test )

BOOST_AUTO_TEST_CASE( test1 )
{
    std::vector<transaction_t> transactions = get_transactions1();
    
    activity_period ap;
    
    ap.add_block(transactions);
    
    double_type activity = ap.get_activity();

    BOOST_CHECK_CLOSE(activity, 3, 1e-3);
    
    emission_parameters_t params;
    
    params.initial_supply = 100000000;
    params.emission_event_count_per_year = 12;
    params.emission_scale = 1000000;
    params.year_emission_limit = 10;
    
    emission_state_t state;
    
    emission_calculator ec(params, state);
    money_t total_emission = 0;
    
    money_t emission = ec.calculate(total_emission, ap);
    
    total_emission += emission;
    
    BOOST_CHECK_EQUAL(emission, 761201);
    
    ap.clear();
    
    ap.add_block(get_transactions2());
    activity = ap.get_activity();

    BOOST_CHECK_CLOSE(activity, 5, 1e-3);

    emission = ec.calculate(total_emission, ap);

    total_emission += emission;
    
    BOOST_CHECK_EQUAL(emission, 795305);

    ap.clear();
    
    ap.add_block(get_transactions1());
    activity = ap.get_activity();

    BOOST_CHECK_CLOSE(activity, 3, 1e-3);

    emission = ec.calculate(total_emission, ap);

    total_emission += emission;
    
    BOOST_CHECK_EQUAL(emission, 0);
}

BOOST_AUTO_TEST_CASE( test2 )
{
    std::vector<transaction_t> transactions = get_transactions1();
    
    activity_period ap;
    
    ap.add_block(transactions);
    
    double_type activity = ap.get_activity();

    BOOST_CHECK_CLOSE(activity, 3, 1e-3);
    
    emission_parameters_t params;
    
    params.initial_supply = 100000000;
    params.emission_event_count_per_year = 12;
    params.emission_scale = 100000;
    params.year_emission_limit = 10;
    
    emission_state_t state;
    
    emission_calculator ec(params, state);
    money_t total_emission = 0;
    
    money_t emission = ec.calculate(total_emission, ap);
    
    total_emission += emission;
    
    BOOST_CHECK_EQUAL(emission, 148255);
    
    ap.clear();
    
    ap.add_block(get_transactions2());
    activity = ap.get_activity();

    BOOST_CHECK_CLOSE(activity, 5, 1e-3);

    emission = ec.calculate(total_emission, ap);

    total_emission += emission;
    
    BOOST_CHECK_EQUAL(emission, 173083);
}

BOOST_AUTO_TEST_CASE( test3 )
{
    activity_period ap;

    ap.add_block(get_transactions1());
    
    char buffer[] = "/tmp/grv_apXXXXXX\0";
    
    mkstemp(buffer);
    std::string filename(buffer);
    
    ap.save_state_to_file(filename);
    
    activity_period ap2;
    
    ap2.load_state_from_file(filename);
    
    remove(filename.c_str());

    BOOST_CHECK_EQUAL(ap.get_handled_block_count(), ap2.get_handled_block_count());

    BOOST_CHECK_EQUAL(ap.get_activity(), ap2.get_activity());
}

BOOST_AUTO_TEST_CASE( new_emission_test )
{
    double_type current_supply = 100000000;
    double_type emission_event_count_per_year = 12;
    double_type year_emission_limit = 10;
    double_type emission_scale = 100000;
    double_type emission_period = double_type(365*24*3600)/emission_event_count_per_year;
    double_type delay_koefficient = 0.5;
    
    double_type current_activity, max_activity, emission_limit, target_emission, emission;
    
    emission_calculator_new ec;
    
    current_activity = 3;
    max_activity = 0;

    emission_limit = ec.get_emission_limit(current_supply, year_emission_limit, emission_period);
    BOOST_CHECK_CLOSE(emission_limit, 797414.04289, 1e-3);
    target_emission = ec.get_target_emission(current_activity, max_activity, emission_scale);
    BOOST_CHECK_CLOSE(target_emission, 300000, 1e-3);
    emission = ec.get_resulting_emission(target_emission, emission_limit, delay_koefficient);
    BOOST_CHECK_CLOSE(emission, 148255, 1e-3);
    
    current_supply += emission;
    max_activity = current_activity;
    current_activity = 5;

    emission_limit = ec.get_emission_limit(current_supply, year_emission_limit, emission_period);
    BOOST_CHECK_CLOSE(emission_limit, 798596.2526, 1e-3);
    target_emission = ec.get_target_emission(current_activity, max_activity, emission_scale);
    BOOST_CHECK_CLOSE(target_emission, 200000, 1e-3);
    emission = ec.get_resulting_emission(target_emission, emission_limit, delay_koefficient);
    
    BOOST_CHECK_CLOSE(emission, 99480.5915, 1e-3);
//    BOOST_CHECK_CLOSE(emission, 173083, 1e-3);
    
}


BOOST_AUTO_TEST_SUITE_END()
