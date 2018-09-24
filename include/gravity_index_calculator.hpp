#ifndef GRAVITY_INDEX_CALCULATOR_HPP
#define GRAVITY_INDEX_CALCULATOR_HPP

#include <cstdint>
#include "utils.hpp"
#include "activity_index_calculator.hpp"

namespace singularity {

    class gravity_index_calculator
    {
    public:
        gravity_index_calculator(double_type activity_weight, double_type social_weight, money_t current_supply):
        activity_weight(activity_weight),
        social_weight(social_weight),
        current_supply(current_supply) 
        {};
        double_type calculate_index(money_t balance, double_type activity_index, double_type social_index);
        money_t calculate_votes(money_t balance, double_type activity_index, double_type social_index);
        account_activity_index_map_t scale_activity_index(const account_activity_index_map_t& index_map); 
        
    private: 
        double_type activity_weight;
        double_type social_weight;
        money_t current_supply;
    };
}

#endif /* GRAVITY_INDEX_CALCULATOR_HPP */

