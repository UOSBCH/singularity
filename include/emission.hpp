
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
        uint16_t year_emission_limit = 10;
        // Emission value koefficient
        money_t emission_scale = 100000000;
        // Emission event quantity per a year
        uint32_t emission_event_count_per_year = 12;
        // Delay emission koefficient, 0 < delay_koefficient < 1
        double_type delay_koefficient = 0.5;
    };
    
    struct emission_state_t
    {
        money_t target_emission = 0;
        double_type last_activity = 0;
    };
    
    class activity_period
    {
    public:
        activity_period();
        const matrix_t::size_type initial_size = 10000;
        void add_block(const std::vector<transaction_t>& transactions);
        double_type get_activity();
        void clear();
        unsigned int get_handled_block_count();
        void save_state_to_file(std::string filename);
        void load_state_from_file(std::string filename);
    private:
        friend class boost::serialization::access;
        unsigned int handled_blocks_count = 0;
        account_id_map_t account_map;
        std::mutex accounts_lock;
        std::mutex weight_matrix_lock;
        std::shared_ptr<matrix_t> p_weight_matrix;
        void collect_accounts(
            account_id_map_t& account_id_map,
            const std::vector<transaction_t>& transactions
        );
        byte_matrix_t calculate_link_matrix(
            matrix_t::size_type size,
            matrix_t& weight_matrix
        );
        void update_weight_matrix(
            matrix_t& weight_matrix,
            account_id_map_t& account_id_map,
            const std::vector<transaction_t>& transactions
        );
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & BOOST_SERIALIZATION_NVP(handled_blocks_count);
            ar & BOOST_SERIALIZATION_NVP(account_map);
            ar & BOOST_SERIALIZATION_NVP(*p_weight_matrix);
        }
    };

    class emission_calculator
    {
    public:
        emission_calculator(emission_parameters_t parameters, emission_state_t emission_state)
        :parameters(parameters), 
        emission_state(emission_state) 
        {};
        money_t calculate(money_t total_emission, activity_period& period);
        emission_state_t get_emission_state();
        emission_parameters_t get_parameters();
        void set_parameters(emission_parameters_t emission_parameters);
    private:
        emission_parameters_t parameters;
        emission_state_t emission_state;
    };
    
    class emission_calculator_new 
    {
    public:
        double_type get_emission_limit(double_type current_total_supply, double_type yearly_emission_percent, double_type emission_period_seconds);
        
        double_type get_target_emission(double_type current_activity, double_type max_activity, double_type activity_monetary_value);
        
        double_type get_resulting_emission(double_type target_emission, double_type emission_limit, double_type delay_koefficient);

        double_type get_next_max_activity(double_type max_activity, double_type resulting_emission, double_type activity_monetary_value);
    };
}

#endif /* EMISSION_HPP */
