
#ifndef NCD_AWARE_RANKING_HPP
#define NCD_AWARE_RANKING_HPP

#include "rank_interface.hpp"
#include "scan.hpp"

namespace singularity {
    class ncd_aware_rank: public rank_interface
    {
    public:
        ncd_aware_rank(parameters_t parameters):parameters(parameters) {}
        const uint32_t MAX_ITERATIONS = 1000;
        virtual std::shared_ptr<vector_t> process(
            const matrix_t& outlink_matrix,
            const vector_t& initial_vector,
            const vector_t& weight_vector,
            const additional_matrices_vector& additional_matrices
        ) const override;
        virtual double_type get_teleportation_weight() const override;
        virtual Graph create_graph(const matrix_t& m) const;
    private:
        parameters_t parameters;
        virtual std::shared_ptr<vector_t> calculate_rank(
            const matrix_t& outlink_matrix, 
            const additional_matrices_vector& additional_matrices,
            const matrix_t& interlevel_matrix_s, 
            const matrix_t& intelevel_matrix_l,
            const vector_t& initial_vector
        ) const;
        virtual std::shared_ptr<matrix_t> create_interlevel_matrix_s(
            const Graph& g
        ) const;
        virtual std::shared_ptr<matrix_t> create_interlevel_matrix_l(
            const Graph& g, 
            const matrix_t& outlink_matrix
        ) const;
        virtual std::shared_ptr<vector_t> iterate(
            const matrix_t& outlink_matrix, 
            const additional_matrices_vector& additional_matrices,
            const matrix_t& interlevel_matrix_s, 
            const matrix_t& interlevel_matrix_l, 
            const vector_t& previous,
            const vector_t& teleportation
        ) const;
    };
}

#endif /* NCD_AWARE_RANKING_HPP */

