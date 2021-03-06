
#ifndef ACTIVITY_INDEX_CALCULATOR_HPP
#define ACTIVITY_INDEX_CALCULATOR_HPP

#include <mutex>
#include <ctime>
#include "utils.hpp"
#include "relations.hpp"
#include "rank_interface.hpp"
#include "filters.hpp"
#include "validator.hpp"
#include "abstract_index_calculator.hpp"

namespace singularity {
    class activity_index_calculator: public abstract_index_calculator
    {
    public:
        const matrix_t::size_type initial_size = 10;
        activity_index_calculator(
            parameters_t parameters, 
            bool disable_negative_weights,
            std::shared_ptr<rank_interface> p_rank_calculator
        ): parameters(parameters), disable_negative_weights(disable_negative_weights), p_rank_calculator(p_rank_calculator) {
            validator.validate(parameters);
            p_weight_matrix = std::make_shared<matrix_t>(initial_size, initial_size);
            p_decay_manager = std::make_shared<decay_manager_t>(parameters.decay_period, parameters.decay_koefficient);
        }
        virtual void add_block(const std::vector<std::shared_ptr<relation_t> >& transactions) override;
        void skip_blocks(unsigned int blocks_count);
        virtual std::map<node_type, std::shared_ptr<account_activity_index_map_t> > calculate() override;
        unsigned int get_total_handled_block_count();
        void set_parameters(parameters_t params);
        parameters_t get_parameters();
        void set_filter(std::shared_ptr<filter_interface> filter)
        {
            p_filter = filter;
        };
    private:
        parameters_t parameters;
        parameters_validator_t validator;
        bool disable_negative_weights;
        
        unsigned int total_handled_blocks_count = 0;
        unsigned int handled_blocks_count = 0;
        std::shared_ptr<matrix_t> p_weight_matrix;
        account_id_map_t account_map;
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
            const std::vector<std::shared_ptr<relation_t> >& relations
        );
        boost::optional<account_id_map_t::mapped_type> get_account_id(std::string name, bool allow_create);
        
        void calculate_outlink_matrix(
            matrix_t& o,
            matrix_t& weight_matrix,
            additional_matrices_vector& additional_matrices
        );
        void update_weight_matrix(
            const std::vector<std::shared_ptr<relation_t> >& transactions
        );
        void normalize_columns(matrix_t &m, additional_matrices_vector& additional_matrices);
        vector_t create_initial_vector();
    };
}

BOOST_CLASS_VERSION( singularity::activity_index_calculator, 1 )

#endif /* ACTIVITY_INDEX_CALCULATOR_HPP */

