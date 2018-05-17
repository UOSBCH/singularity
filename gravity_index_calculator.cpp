#include "include/gravity_index_calculator.hpp"

using namespace singularity;

double gravity_index_calculator::calculate_index(uint64_t balance, double activity_index)
{
    double stake_index = (double) balance / current_supply;
    
    return ( 1 - activity_weight ) * stake_index + activity_weight * activity_index;
}

uint64_t gravity_index_calculator::calculate_votes(uint64_t balance, double activity_index)
{
    return current_supply * calculate_index(balance, activity_index);
}
