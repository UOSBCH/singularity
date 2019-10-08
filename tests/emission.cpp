#define BOOST_TEST_MODULE EMISSION
#include <boost/test/included/unit_test.hpp>
#include "../include/emission.hpp"
#include <stdlib.h>
#include <iostream>
#include <boost/numeric/ublas/io.hpp>

using namespace singularity;
using namespace boost;
using namespace boost::numeric::ublas;

class emission_step_t
{
public:
    emission_step_t(
        double_type activity, 
        double_type max_activity, 
        double_type target_emission,        
        double_type emission_value
    ):
        activity(activity), 
        max_activity(max_activity),
        target_emission(target_emission),
        emission_value(emission_value)
    {};
    double_type activity;
    double_type max_activity;
    double_type target_emission; 
    double_type emission_value;
};

std::vector<emission_step_t> get_emission_steps()
{
    return {
        emission_step_t(3, 1.48255, 300000, 148255),
        emission_step_t(5, 3.21329, 351744, 173074),
    };
};

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
    
    max_activity = 0;

    emission_limit = ec.get_emission_limit(current_supply);
    BOOST_CHECK_CLOSE(emission_limit, 797414.04289, 1e-3);
    
    for(auto step: get_emission_steps()) {
        target_emission = ec.get_target_emission(step.activity, max_activity);
        BOOST_CHECK_CLOSE(target_emission, step.target_emission, 1e-3);
        emission = ec.get_resulting_emission(target_emission, emission_limit);
        BOOST_CHECK_CLOSE(emission, step.emission_value, 1e-3);
        current_supply += emission;
        total_emission += emission;
        max_activity = ec.get_next_max_activity(max_activity, emission);
        BOOST_CHECK_CLOSE(max_activity, step.max_activity, 1e-3);
    }
}

BOOST_AUTO_TEST_SUITE_END()
