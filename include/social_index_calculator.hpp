
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
#include "validator.hpp"

namespace singularity {

    enum calculation_mode {SIMPLE, DIAGONAL, PHANTOM_ACCOUNT};
    
    struct intermediate_results_t 
    {
        account_activity_index_map_t default_initial;
        account_activity_index_map_t trust;
        account_activity_index_map_t priority;
        account_activity_index_map_t stack;
        account_activity_index_map_t initial;
        account_activity_index_map_t activity_index;
        account_activity_index_map_t activity_index_significant;
        account_activity_index_map_t activity_index_norm;
        account_activity_index_map_t activity_index_norm_excluding_phantom;
    };
    
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
            validator.validate(parameters);
            p_ownership_matrix = std::make_shared<matrix_t>(initial_size, initial_size);
            p_vote_matrix = std::make_shared<matrix_t>(initial_size, initial_size);
            p_repost_matrix = std::make_shared<matrix_t>(initial_size, initial_size);
            p_comment_matrix = std::make_shared<matrix_t>(initial_size, initial_size);
            p_trust_matrix = std::make_shared<matrix_t>(initial_size, initial_size);
            p_decay_manager = std::make_shared<decay_manager_t>(parameters.decay_period, parameters.decay_koefficient);
            if (mode == calculation_mode::PHANTOM_ACCOUNT) {
                get_account_id(reserved_account, true);
            }
        }
        void add_block(const std::vector<std::shared_ptr<relation_t> >& relations);
        void skip_blocks(unsigned int blocks_count);
        std::map<node_type, std::shared_ptr<account_activity_index_map_t> > calculate();
        std::shared_ptr<vector_t> calculate_priority_vector();
        unsigned int get_total_handled_block_count();
        void set_parameters(parameters_t params);
        parameters_t get_parameters();
        void set_filter(std::shared_ptr<filter_interface> filter)
        {
            p_filter = filter;
        };
        void add_stack_vector(const std::map<std::string, double_type>& stacks);
        void set_priorities(const std::map<std::string, double_type>& priorities);
        activity_index_detalization_t get_account_rank_detalization() 
        {
            return account_rank_detalization;
        };
        activity_index_detalization_t get_account_priority_detalization() 
        {
            return account_priority_detalization;
        };
        activity_index_detalization_t get_content_rank_detalization() 
        {
            return content_rank_detalization;
        };
        account_activity_index_map_t vector2map(vector_t& v);
        intermediate_results_t get_last_intermediate_results()
        {
            return last_intermediate_results;
        };
    private:
        parameters_t parameters;
        parameters_validator_t validator;
        bool disable_negative_weights;
        std::shared_ptr<rank_interface> p_rank_calculator;
        calculation_mode mode;
        
        exporter_t exporter;
        
        unsigned int total_handled_blocks_count = 0;
        unsigned int handled_blocks_count = 0;
        std::shared_ptr<matrix_t> p_ownership_matrix;
        std::shared_ptr<matrix_t> p_vote_matrix;
        std::shared_ptr<matrix_t> p_repost_matrix;
        std::shared_ptr<matrix_t> p_comment_matrix;
        std::shared_ptr<matrix_t> p_trust_matrix;

        account_id_map_t account_map;
        account_id_map_t content_map;
        
        std::map<std::string, double_type> stack_map;
        std::map<std::string, double_type> priority_map;
        
        uint64_t accounts_count = 0;
        uint64_t contents_count = 0;
        std::mutex accounts_lock;
        std::mutex weight_matrix_lock;
        
        std::shared_ptr<decay_manager_t> p_decay_manager;
        std::shared_ptr<filter_interface> p_filter;

        std::vector<std::shared_ptr<relation_t> > filter_block(const std::vector<std::shared_ptr<relation_t> >& block);
                
        std::map<node_type, std::shared_ptr<account_activity_index_map_t> > calculate_score(
            const vector_t& account_rank,
            const vector_t& content_rank
        );
        
        void collect_accounts(
            const std::vector<std::shared_ptr<relation_t> >& relations
        );
        void calculate_outlink_matrix(
            matrix_t& o,
            matrix_t& weight_matrix,
            additional_matrices_vector& additional_matrices,
            const vector_t& weight_vector
        );
        void calculate_content_matrix(
            matrix_t& o,
            matrix_t& weight_matrix
        );
        void update_weight_matrix(
            const std::vector<std::shared_ptr<relation_t> >& relations
        );
        void normalize_columns(matrix_t &m, additional_matrices_vector& additional_matrices, const vector_t& weight_vector);
        vector_t create_default_initial_vector();
        void limit_values(matrix_t& m);
        void adjust_matrix_sizes();
        vector_t create_stack_vector();
        vector_t create_priority_vector();
        void set_diagonal_elements(matrix_t& m);
        void add_phantom_account_relations (matrix_t& m);
        
        boost::optional<account_id_map_t::mapped_type> get_account_id(std::string name, bool allow_create);
        boost::optional<account_id_map_t::mapped_type> get_content_id(std::string name, bool allow_create);
        
        activity_index_detalization_t account_rank_detalization;
        activity_index_detalization_t account_priority_detalization;
        activity_index_detalization_t content_rank_detalization;
        void calculate_detalization(
            activity_index_detalization_t& detalization,
            double_type outlink_weight,
            double_type normalization_koefficient,
            const matrix_t& outlink_matrix,
            const vector_t& activity_index_vector,
            const vector_t& base_vector,
            const additional_matrices_vector& additional_matrices
        );
        
        void calculate_content_detalization (
            activity_index_detalization_t& detalization,
            const matrix_t& outlink_matrix, 
            const vector_t& activity_index_vector
        ); 
        
        intermediate_results_t last_intermediate_results;
    };
};

BOOST_CLASS_VERSION( singularity::social_index_calculator, 1 )

#endif /* SOCIAL_INDEX_CALCULATOR_HPP */

