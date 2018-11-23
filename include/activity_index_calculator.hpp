
#ifndef ACTIVITY_INDEX_CALCULATOR_HPP
#define ACTIVITY_INDEX_CALCULATOR_HPP

#include <mutex>
#include <ctime>
#include "utils.hpp"
#include "relations.hpp"
#include "rank_interface.hpp"
#include "filters.hpp"

namespace singularity {
    
    typedef std::map<std::string, uint32_t> account_id_map_t;
    typedef std::map<std::string, double_type> account_activity_index_map_t;
    
    struct account_t {
        money_t amount;
        int height;
    };
    
    class activity_index_calculator 
    {
    public:
        const matrix_t::size_type initial_size = 10;
        activity_index_calculator(
            parameters_t parameters, 
            bool disable_negative_weights,
            std::shared_ptr<rank_interface> p_rank_calculator
        ): parameters(parameters), disable_negative_weights(disable_negative_weights), p_rank_calculator(p_rank_calculator) {
            p_weight_matrix = std::make_shared<matrix_t>(initial_size, initial_size);
            p_decay_manager = std::make_shared<decay_manager_t>(parameters.decay_period, parameters.decay_koefficient);
        }
        void add_block(const std::vector<std::shared_ptr<relation_t> >& transactions);
        void skip_blocks(unsigned int blocks_count);
        std::map<node_type, std::shared_ptr<account_activity_index_map_t> > calculate();
        void save_state_to_file(std::string filename);
        void load_state_from_file(std::string filename);
        unsigned int get_total_handled_block_count();
        void set_parameters(parameters_t params);
        parameters_t get_parameters();
        void set_filter(std::shared_ptr<filter_interface> filter)
        {
            p_filter = filter;
        };
    private:
        friend class boost::serialization::access;
        parameters_t parameters;
        bool disable_negative_weights;
        
        unsigned int total_handled_blocks_count = 0;
        unsigned int handled_blocks_count = 0;
        std::shared_ptr<matrix_t> p_weight_matrix;
        std::map<node_type, std::shared_ptr<account_id_map_t> > node_maps;
        uint64_t nodes_count = 0;
        std::mutex accounts_lock;
        std::mutex weight_matrix_lock;
        
        std::shared_ptr<rank_interface> p_rank_calculator;
        std::shared_ptr<decay_manager_t> p_decay_manager;
        std::shared_ptr<filter_interface> p_filter;

        std::vector<std::shared_ptr<relation_t> > filter_block(const std::vector<std::shared_ptr<relation_t> >& block);
                
        std::map<node_type, std::shared_ptr<account_activity_index_map_t> > calculate_score(
            const vector_t& rank
        );
        
        void collect_accounts(
            const std::vector<std::shared_ptr<relation_t> >& transactions
        );
        void calculate_outlink_matrix(
            matrix_t& o,
            matrix_t& weight_matrix,
            additional_matrices_vector& additional_matrices,
            const vector_t& initial_vector
        );
        void update_weight_matrix(
            matrix_t& weight_matrix,
            const std::vector<std::shared_ptr<relation_t> >& transactions
        );
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & BOOST_SERIALIZATION_NVP(total_handled_blocks_count);
            ar & BOOST_SERIALIZATION_NVP(handled_blocks_count);
//             ar & BOOST_SERIALIZATION_NVP(node_maps);
            ar & BOOST_SERIALIZATION_NVP(*p_weight_matrix);
        }
        void normalize_columns(matrix_t &m, additional_matrices_vector& additional_matrices, const vector_t& initial_vector);
        vector_t create_initial_vector();
    };
}

BOOST_CLASS_VERSION( singularity::activity_index_calculator, 1 )

#endif /* ACTIVITY_INDEX_CALCULATOR_HPP */

