
#ifndef EMISSION_HPP
#define EMISSION_HPP

#include "utils.hpp"
#include "activity_index_calculator.hpp"

namespace singularity {

    struct emission_parameters_t
    {
        // Initial token supply
        uint64_t initial_supply = 100000000000000;
        // Maximal supply increment per year, in percents
        uint16_t year_emission_limit = 10;
        // Emission value koefficient
        uint64_t emission_scale = 100000000;
        // Emission event quantity per a year
        uint16_t emission_event_count_per_year = 12;
        // Delay emission koefficient, 0 < delay_koefficient < 1
        double delay_koefficient = 0.5;
    };
    
    class activity_period
    {
    public:
        activity_period();
        const matrix_t::size_type initial_size = 10000;
        void add_block(const std::vector<transaction_t>& transactions);
        double get_activity();
        void clear();
    private:
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
    };

    class emission_calculator
    {
    public:
        emission_calculator(emission_parameters_t parameters):parameters(parameters) {};
        uint64_t calculate(uint64_t total_emission, double activity);
    private:
        emission_parameters_t parameters;
    };
}

#endif /* EMISSION_HPP */
