#include "include/gravity_index_calculator.hpp"

using namespace singularity;

double_type gravity_index_calculator::calculate_index(money_t balance, double_type activity_index, double_type social_index)
{
    return ( 1.0 - activity_weight - social_weight ) * double_type(balance) / double_type(current_supply) + activity_weight * activity_index + social_weight * social_index;
}

money_t gravity_index_calculator::calculate_votes(money_t balance, double_type activity_index, double_type social_index)
{
    return (money_t)(double_type) (calculate_index(balance, activity_index, social_index) * current_supply);
}
