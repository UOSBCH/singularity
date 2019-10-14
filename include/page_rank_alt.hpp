
#ifndef PAGE_RANKING_ALT_HPP
#define PAGE_RANKING_ALT_HPP

#include "rank_interface.hpp"
#include "utils.hpp"

namespace singularity {
    class page_rank_alt: public rank_interface
    {
    public:
        page_rank_alt(double_type outlink_weight, uint16_t num_threads, double_type precision)
            :outlink_weight(outlink_weight), 
            num_threads(num_threads),
            precision(precision)
            {}
        const uint32_t MAX_ITERATIONS = 1000;
        void set_outlink_weight(double_type a_outlink_weight) {
            outlink_weight = a_outlink_weight;
        }
        virtual std::shared_ptr<vector_t> process(
            const matrix_t& outlink_matrix,
            const vector_t& initial_vector,
            const vector_t& weight_vector,
            const additional_matrices_vector& additional_matrices
        ) const override;
    private:
        double_type outlink_weight;
        uint16_t num_threads;
        double_type precision;
        virtual std::shared_ptr<vector_t> calculate_rank(
            const matrix_t& outlink_matrix, 
            const additional_matrices_vector& additional_matrices,
            const vector_t& initial_vector,
            const vector_t& weight_vector
        ) const;
        virtual std::shared_ptr<vector_t> iterate(
            const matrix_t& outlink_matrix, 
            const additional_matrices_vector& additional_matrices,
            const vector_t& previous,
            const vector_t& teleportation
        ) const;
    };
}

#endif /* PAGE_RANKING_ALT_HPP */


