
#ifndef ACTIVITY_INDEX_CALCULATOR_HPP
#define ACTIVITY_INDEX_CALCULATOR_HPP

#include <mutex>
#include <ctime>
#include "utils.hpp"

namespace singularity {
    
    typedef std::map<std::string, unsigned int> account_id_map_t;
    typedef std::map<std::string, double> account_activity_index_map_t;
    
    struct account_t {
        double amount;
        int height;
    };

    struct transaction_t {
        double amount;
        double comission;
        std::string source_account;
        std::string target_account;
        time_t timestamp;
        transaction_t (double amount, double comission, std::string source_account, std::string target_account, time_t timestamp) :
            amount(amount), comission(comission), source_account(source_account), target_account(target_account), timestamp(timestamp) { }
    };
    
    class activity_index_calculator 
    {
    public:
        const matrix_t::size_type initial_size = 10000;
        activity_index_calculator(parameters_t parameters);
        void add_block(const std::vector<transaction_t>& transactions);
        void skip_blocks(unsigned int blocks_count);
        account_activity_index_map_t calculate();
        bool check_account( account_t account);
        bool check_transaction( transaction_t transaction);
        void save_state_to_file(std::string filename);
        void load_state_from_file(std::string filename);
        unsigned int get_total_handled_block_count();
    private:
        friend class boost::serialization::access;
        parameters_t parameters;
        
        unsigned int total_handled_blocks_count = 0;
        unsigned int handled_blocks_count = 0;
        std::shared_ptr<matrix_t> p_weight_matrix;
        account_id_map_t account_map;
        std::mutex accounts_lock;
        std::mutex weight_matrix_lock;
                
        account_activity_index_map_t calculate_score(
            const account_id_map_t& account_id_map,
            const vector_t& rank
        );
        void collect_accounts(
            account_id_map_t& account_id_map,
            const std::vector<transaction_t>& transactions
        );
        void calculate_outlink_matrix(
            matrix_t& o,
            matrix_t& weight_matrix
        );
        void update_weight_matrix(
            matrix_t& weight_matrix,
            account_id_map_t& account_id_map,
            const std::vector<transaction_t>& transactions
        );
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & BOOST_SERIALIZATION_NVP(total_handled_blocks_count);
            ar & BOOST_SERIALIZATION_NVP(handled_blocks_count);
            ar & BOOST_SERIALIZATION_NVP(account_map);
            ar & BOOST_SERIALIZATION_NVP(*p_weight_matrix);
        }
    };
}

BOOST_CLASS_VERSION( singularity::activity_index_calculator, 1 )

#endif /* ACTIVITY_INDEX_CALCULATOR_HPP */

