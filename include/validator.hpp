
#ifndef VALIDATOR_HPP
#define VALIDATOR_HPP

#include "utils.hpp"

namespace singularity {

    class parameters_validator_t
    {
        public:
        void validate(const parameters_t& parameters)
        {
            if (parameters.outlink_weight <= 0) {
                throw validation_exception("The parameter outlink_weight must be positive");
            }
            
            if (parameters.interlevel_weight < 0) {
                throw validation_exception("The parameter interlevel_weight must be not negative");
            }
            
            if (parameters.outlink_weight + parameters.interlevel_weight >= 1) {
                throw validation_exception("Sum of parameters outlink_weight and interlevel_weight must be less than 1");
            }
            
            if (parameters.stack_contribution < 0) {
                throw validation_exception("The parameter stack_contribution must be not nefative");
            }
            
            if (parameters.weight_contribution < 0) {
                throw validation_exception("The parameter weight_contribution must be not negative");
            }
            
            if (parameters.stack_contribution + parameters.weight_contribution >= 1) {
                throw validation_exception("Sum of parameters stack_contribution and weight_contribution must be less than 1");
            }

            if (parameters.rank_calculation_precision <= 0) {
                throw validation_exception("The parameter rank_calculation_precision must be positive");
            }

            if (parameters.token_usd_rate <= 0) {
                throw validation_exception("The parameter token_usd_rate must be positive");
            }

            if (parameters.decay_koefficient <= 0 || parameters.decay_koefficient >= 1) {
                throw validation_exception("Parameter decay_koefficient must be between 0 and 1");
            }

            if (parameters.clustering_e <= 0) {
                throw validation_exception("The parameter clustering_e must be positive");
            }
        }
    };
};
#endif /* VALIDATOR_HPP */
