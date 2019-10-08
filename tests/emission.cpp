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
    time_t now = time(nullptr);
    
    return {
        transaction_t (200, 0, "account-0", "account-1", now, 0, 0, 0),
        transaction_t (100, 0, "account-1", "account-0", now, 0, 0, 0),
        transaction_t (300, 0, "account-0", "account-2", now, 0, 0, 0),
        transaction_t (500, 0, "account-2", "account-1", now, 0, 0, 0),
        transaction_t (700, 0, "account-1", "account-2", now, 0, 0, 0),
    };
}

std::vector<transaction_t> get_transactions2()
{
    time_t now = time(nullptr);
    
    return {
        transaction_t (200, 0, "account-0", "account-1", now, 0, 0, 0),
        transaction_t (100, 0, "account-1", "account-0", now, 0, 0, 0),
        transaction_t (300, 0, "account-0", "account-2", now, 0, 0, 0),
        transaction_t (500, 0, "account-2", "account-1", now, 0, 0, 0),
        transaction_t (700, 0, "account-1", "account-2", now, 0, 0, 0),
        transaction_t (800, 0, "account-1", "account-3", now, 0, 0, 0),
        transaction_t (100, 0, "account-2", "account-3", now, 0, 0, 0),
    };
}

BOOST_AUTO_TEST_SUITE( emission_test )

BOOST_AUTO_TEST_CASE( new_emission_test )
{
    emission_parameters_t params;
    
    double_type current_supply = 100000000;
    params.yearly_emission_percent = 10;
    params.activity_monetary_value = 100000;
    params.emission_period_seconds = double_type(365*24*3600)/12;
    params.delay_koefficient = 0.5;
    
    
    double_type current_activity, max_activity, emission_limit, target_emission, emission, total_emission;
    
    emission_calculator_new ec(params);
    
    current_activity = 3;
    max_activity = 0;

    emission_limit = ec.get_emission_limit(current_supply);
    BOOST_CHECK_CLOSE(emission_limit, 797414.04289, 1e-3);
    target_emission += ec.get_target_emission(current_activity, max_activity);
    BOOST_CHECK_CLOSE(target_emission, 300000, 1e-3);
    emission = ec.get_resulting_emission(target_emission, emission_limit);
    BOOST_CHECK_CLOSE(emission, 148255, 1e-3);
    
    current_supply += emission;
    total_emission += emission;
    max_activity = ec.get_next_max_activity(max_activity, emission);
    current_activity = 5;

    emission_limit = ec.get_emission_limit(current_supply);
    BOOST_CHECK_CLOSE(emission_limit, 798596.2526, 1e-3);
    target_emission += ec.get_target_emission(current_activity, max_activity);
    BOOST_CHECK_CLOSE(target_emission, 651744, 1e-3);
    emission = ec.get_resulting_emission(target_emission - total_emission, emission_limit);
    
    BOOST_CHECK_CLOSE(emission, 243724, 1e-3);
    
}

BOOST_AUTO_TEST_SUITE_END()
