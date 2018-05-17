#ifndef GRAVITY_INDEX_CALCULATOR_HPP
#define GRAVITY_INDEX_CALCULATOR_HPP

#include <cstdint>

namespace singularity {

class gravity_index_calculator
{
    public:
        gravity_index_calculator(double activity_weight, uint64_t current_supply):
            activity_weight(activity_weight),
            current_supply(current_supply) 
            {};
        double calculate_index(uint64_t balance, double activity_index);
        uint64_t calculate_votes(uint64_t balance, double activity_index);
        private: 
            double activity_weight;
            uint64_t current_supply;
        };
}

#endif /* GRAVITY_INDEX_CALCULATOR_HPP */

