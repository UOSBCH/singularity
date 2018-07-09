#include "include/gravity_index_calculator.hpp"

using namespace singularity;

double_type gravity_index_calculator::calculate_index(money_t balance, double_type activity_index)
{
    double_type stake_index = double_type( balance ) / double_type ( current_supply );
    
    return ( 1 - activity_weight ) * stake_index + activity_weight * activity_index;
}

money_t gravity_index_calculator::calculate_votes(money_t balance, double_type activity_index)
{
    return  (money_t) (calculate_index(balance, activity_index) * current_supply);
}
