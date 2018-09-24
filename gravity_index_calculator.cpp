#include "include/gravity_index_calculator.hpp"

using namespace singularity;

double_type gravity_index_calculator::calculate_index(money_t balance, double_type activity_index, double_type social_index)
{
    double_type stake_index = double_type( balance ) / double_type ( current_supply );
    
    return ( 1 - activity_weight - social_weight ) * stake_index + activity_weight * activity_index + social_weight * social_index;
}

money_t gravity_index_calculator::calculate_votes(money_t balance, double_type activity_index, double_type social_index)
{
    return  (money_t)(double_type) (calculate_index(balance, activity_index, social_index) * current_supply);
}

account_activity_index_map_t gravity_index_calculator::scale_activity_index(const account_activity_index_map_t& index_map)
{
    account_activity_index_map_t result;
    
    auto objects_count = double_type(index_map.size());
    
    if (objects_count == 0) {
        return result;
    }
    
    for (auto index: index_map) {
        result[index.first] = index.second * objects_count;
    }
    
    return result;
}
