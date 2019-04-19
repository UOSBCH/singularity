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

account_activity_index_map_t gravity_index_calculator::scale_activity_index(const account_activity_index_map_t& index_map, double_type new_norm)
{
    account_activity_index_map_t result;
    
    double_type old_norm = 0;

    for (auto index: index_map) {
        old_norm += index.second;
    }
    
    if (old_norm == 0) {
        return result;
    }
    
    double_type scale = new_norm / old_norm;
    
    for (auto index: index_map) {
        result[index.first] = index.second * scale;
    }
    
    return result;
}

account_activity_index_map_t gravity_index_calculator::scale_activity_index_to_node_count(const account_activity_index_map_t& index_map)
{
    auto objects_count = double_type(index_map.size());
    
    if (objects_count == 0) {
        return account_activity_index_map_t();
    } else {
        return scale_activity_index(index_map, objects_count);
    }
}

account_activity_index_map_t gravity_index_calculator::scale_activity_index_to_1(const account_activity_index_map_t& index_map)
{
    return scale_activity_index(index_map, 1);
}
