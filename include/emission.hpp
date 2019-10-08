
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

    class activity_period_new
    {
    public:
        activity_period_new(uint32_t period_length, uint32_t period_count)
            :period_length(period_length), period_count(period_count)
        {
            p_account_keepers = new std::vector<account_keeper>(period_count);
        };
        virtual ~activity_period_new()
        {
            delete p_account_keepers;
        };
        void add_block(const std::vector<transaction_t>& transactions);
        double_type get_activity();
        unsigned int get_handled_block_count();
    private:
        uint32_t period_length;
        uint32_t period_count;
        std::vector<account_keeper>* p_account_keepers;
        unsigned int handled_blocks_count = 0;
    };
    
//     struct emission_parameters_t
//     {
//         double_type yearly_emission_percent;
//         double_type emission_period_seconds;
//         double_type activity_monetary_value;
//         double_type delay_koefficient;
//     };
    
    class emission_calculator_new 
    {
    public:
        emission_calculator_new(
            emission_parameters_t parameters
        ): _parameters(parameters)
        {};
        double_type get_emission_limit(double_type current_total_supply);
        
        double_type get_target_emission(double_type current_activity, double_type max_activity);
        
        double_type get_resulting_emission(double_type target_emission, double_type emission_limit);

        double_type get_next_max_activity(double_type max_activity, double_type resulting_emission);
        
        void set_parameters(emission_parameters_t parameters)
        {
            _parameters = parameters;
        };
        
        emission_parameters_t get_parameters()
        {
            return _parameters;
        };
    private:
        emission_parameters_t _parameters;
    };
}

#endif /* EMISSION_HPP */
