
#ifndef SOCIAL_INDEX_CALCULATOR_HPP
#define SOCIAL_INDEX_CALCULATOR_HPP

#include <mutex>
#include <ctime>
#include "utils.hpp"
#include "relations.hpp"
#include "rank_interface.hpp"
#include "filters.hpp"
#include <boost/optional.hpp>
#include "exporter.hpp"

namespace singularity {

    enum calculation_mode {SIMPLE, DIAGONAL, PHANTOM_ACCOUNT};
    
    class social_index_calculator 
    {
    public:
        const matrix_t::size_type initial_size = 2;
        const std::string reserved_account = "*";
        social_index_calculator(
            parameters_t parameters, 
            bool disable_negative_weights,
            std::shared_ptr<rank_interface> p_rank_calculator,
            calculation_mode mode
        ): parameters(parameters), disable_negative_weights(disable_negative_weights), p_rank_calculator(p_rank_calculator), mode(mode) {
            p_ownership_matrix = std::make_shared<matrix_t>(initial_size, initial_size);
            p_vote_matrix = std::make_shared<matrix_t>(initial_size, initial_size);
            p_repost_matrix = std::make_shared<matrix_t>(initial_size, initial_size);
            p_comment_matrix = std::make_shared<matrix_t>(initial_size, initial_size);
            p_decay_manager = std::make_shared<decay_manager_t>(parameters.decay_period, parameters.decay_koefficient);
            if (mode == calculation_mode::PHANTOM_ACCOUNT) {
                get_account_id(reserved_account, true);
            }
        }
        void add_block(const std::vector<std::shared_ptr<relation_t> >& transactions);
        void skip_blocks(unsigned int blocks_count);
        std::map<node_type, std::shared_ptr<account_activity_index_map_t> > calculate();
        unsigned int get_total_handled_block_count();
        void set_parameters(parameters_t params);
        parameters_t get_parameters();
        void set_filter(std::shared_ptr<filter_interface> filter)
        {
            p_filter = filter;
        };
        void add_stack_vector(const std::map<std::string, double_type>& stacks);
        void set_weights(const std::map<std::string, double_type>& weights);
        activity_index_detalization_t get_detalization() 
        {
            return detalization;
        };
        activity_index_detalization_t get_content_detalization() 
        {
            return content_detalization;
        };
    private:
        calculation_mode mode;
        friend class boost::serialization::access;
        parameters_t parameters;
        bool disable_negative_weights;
        
        exporter_t exporter;
        
        unsigned int total_handled_blocks_count = 0;
        unsigned int handled_blocks_count = 0;
        std::shared_ptr<matrix_t> p_ownership_matrix;
        std::shared_ptr<matrix_t> p_vote_matrix;
        std::shared_ptr<matrix_t> p_repost_matrix;
        std::shared_ptr<matrix_t> p_comment_matrix;

        account_id_map_t account_map;
        account_id_map_t content_map;
        
        std::map<std::string, double_type> stack_map;
        std::map<std::string, double_type> weight_map;
        
        uint64_t accounts_count = 0;
        uint64_t contents_count = 0;
        std::mutex accounts_lock;
        std::mutex weight_matrix_lock;
        
        std::shared_ptr<rank_interface> p_rank_calculator;
        std::shared_ptr<decay_manager_t> p_decay_manager;
        std::shared_ptr<filter_interface> p_filter;

        std::vector<std::shared_ptr<relation_t> > filter_block(const std::vector<std::shared_ptr<relation_t> >& block);
                
        std::map<node_type, std::shared_ptr<account_activity_index_map_t> > calculate_score(
            const vector_t& account_rank,
            const vector_t& content_rank
        );
        
        void collect_accounts(
            const std::vector<std::shared_ptr<relation_t> >& transactions
        );
        void calculate_outlink_matrix(
            matrix_t& o,
            matrix_t& weight_matrix,
            additional_matrices_vector& additional_matrices
        );
        void calculate_content_matrix(
            matrix_t& o,
            matrix_t& weight_matrix
        );
        void update_weight_matrix(
            const std::vector<std::shared_ptr<relation_t> >& transactions
        );
        void normalize_columns(matrix_t &m, additional_matrices_vector& additional_matrices);
        void collapse_matrix(matrix_t& out, const matrix_t& in1, const matrix_t& in2);
        vector_t create_default_initial_vector();
        void limit_values(matrix_t& m);
        void adjust_matrix_sizes();
        vector_t create_stack_vector();
        vector_t create_weight_vector();
        void set_diagonal_elements(matrix_t& m);
        void add_phantom_account_relations (matrix_t& m);
        
        boost::optional<account_id_map_t::mapped_type> get_account_id(std::string name, bool allow_create);
        boost::optional<account_id_map_t::mapped_type> get_content_id(std::string name, bool allow_create);
        
        activity_index_detalization_t detalization;
        activity_index_detalization_t content_detalization;
        void calculate_detalization(
            const matrix_t& outlink_matrix,
            const matrix_t& content_matrix,
            const vector_t& activity_index_vector,
            const vector_t& stack_vector,
            const vector_t& weight_vector,
            const additional_matrices_vector& additional_matrices
        );
    };
}

BOOST_CLASS_VERSION( singularity::social_index_calculator, 1 )

#endif /* SOCIAL_INDEX_CALCULATOR_HPP */

