
#ifndef SOCIAL_INDEX_CALCULATOR_HPP
#define SOCIAL_INDEX_CALCULATOR_HPP

#include <mutex>
#include <ctime>
#include "utils.hpp"
#include "relations.hpp"

namespace singularity {
    
    typedef std::map<std::string, unsigned int> account_id_map_t;
    typedef std::map<std::string, double_type> account_activity_index_map_t;
    
    
    class social_index_calculator 
    {
    public:
        const matrix_t::size_type initial_size = 10000;
        social_index_calculator(parameters_t parameters);
        void add_block(const std::vector<std::shared_ptr<relation_t> >& relations);
        void skip_blocks(unsigned int blocks_count);
        account_activity_index_map_t calculate();
        unsigned int get_total_handled_block_count();
        void set_parameters(parameters_t params);
        parameters_t get_parameters();
    private:
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
            const std::vector<std::shared_ptr<relation_t> >& relations
        );
        void calculate_outlink_matrix(
            matrix_t& o,
            matrix_t& weight_matrix
        );
        void update_weight_matrix(
            matrix_t& weight_matrix,
            account_id_map_t& account_id_map,
            const std::vector<std::shared_ptr<relation_t> >& relations
        );
    };
}

BOOST_CLASS_VERSION( singularity::social_index_calculator, 1 )

#endif /* SOCIAL_INDEX_CALCULATOR_HPP */

