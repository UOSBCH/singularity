
#ifndef RANK_CALCULATOR_FACTORY_HPP
#define RANK_CALCULATOR_FACTORY_HPP

#include "activity_index_calculator.hpp"
#include "social_index_calculator.hpp"
#include "ncd_aware_rank.hpp"
#include "page_rank_alt.hpp"

namespace singularity {
    
    class rank_calculator_factory
    {
    public:
        static std::shared_ptr<activity_index_calculator> create_calculator_for_transfer(parameters_t parameters)
        {
            auto calculator = std::make_shared<activity_index_calculator>(
                parameters, 
                true, 
                std::make_shared<ncd_aware_rank>(parameters)
            );
            
            calculator->set_filter(std::make_shared<transfer_filter>(parameters));
            
            return calculator;
        };
        static std::shared_ptr<social_index_calculator> create_calculator_for_social_network(parameters_t parameters)
        {
            return std::make_shared<social_index_calculator>(
                parameters, 
                true, 
                std::make_shared<page_rank_alt>(parameters.outlink_weight, parameters.num_threads, parameters.rank_calculation_precision),
                calculation_mode::PHANTOM_ACCOUNT
            );
        };
    };
    
}

#endif /* RANK_CALCULATOR_FACTORY_HPP */
