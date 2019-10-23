
#ifndef EMISSION_HPP
#define EMISSION_HPP

#include "utils.hpp"
#include "activity_index_calculator.hpp"

namespace singularity {

    struct emission_parameters_t
    {
        // Initial token supply
        money_t initial_supply = 100000000000000;
        // Maximal supply increment per year, in percents
        double_type yearly_emission_percent = 10;
        // Emission value koefficient
        money_t activity_monetary_value = 100000000;
        // Emission period in seconds
        double_type emission_period_seconds = 1800;
        // Delay emission koefficient, 0 < delay_koefficient < 1
        double_type delay_koefficient = 0.5;
    };
    
    struct emission_state_t
    {
        money_t target_emission = 0;
        double_type last_activity = 0;
    };

    class activity_period_new
    {
    public:
        activity_period_new(uint32_t period_length, uint32_t period_count)
            :period_length(period_length), period_count(period_count)
        {
            if (period_length == 0 || period_count == 0) {
                throw validation_exception("Parameters period_length and period_count must be positive");
            }
            p_account_keepers = new std::vector<id_registry>(period_count);
        }
        virtual ~activity_period_new()
        {
            delete p_account_keepers;
        }
        void add_block(const std::vector<std::shared_ptr<relation_t> >& relations);
        double_type get_activity() const;
        unsigned int get_handled_block_count () const;
    private:
        activity_period_new(const activity_period_new& obj) {}
        uint32_t period_length;
        uint32_t period_count;
        std::vector<id_registry>* p_account_keepers;
        unsigned int handled_blocks_count = 0;
    };
    
    class emission_calculator
    {
    public:
        emission_calculator(
            emission_parameters_t parameters
        ): parameters_(parameters)
        {}
        double_type get_emission_limit(double_type current_total_supply) const;
        
        double_type get_target_emission(double_type current_activity, double_type max_activity) const;
        
        double_type get_resulting_emission(double_type target_emission, double_type emission_limit) const;

        double_type get_next_max_activity(double_type max_activity, double_type resulting_emission) const;
        
        void set_parameters(emission_parameters_t parameters)
        {
            parameters_ = parameters;
        }
        
        emission_parameters_t get_parameters() const
        {
            return parameters_;
        }
    private:
        emission_parameters_t parameters_;
    };
}

#endif /* EMISSION_HPP */
