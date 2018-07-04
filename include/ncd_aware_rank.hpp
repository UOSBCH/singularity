
#ifndef NCD_AWARE_RANKING_HPP
#define NCD_AWARE_RANKING_HPP

#include "scan.hpp"

namespace singularity {
    class ncd_aware_rank
    {
    public:
        ncd_aware_rank(parameters_t parameters):parameters(parameters) {};
        const uint32_t MAX_ITERATIONS = 1000;
        std::shared_ptr<vector_t> process(
            const matrix_t& outlink_matrix
        );
    private:
        parameters_t parameters;
        double_type const precision = 0.01;
        std::shared_ptr<vector_t> calculate_ncd_aware_rank(
            const matrix_t& outlink_matrix, 
            const sparce_vector_t& outlink_vector, 
            const matrix_t& interlevel_matrix_s, 
            const matrix_t& intelevel_matrix_l
        );
        std::shared_ptr<matrix_t> create_interlevel_matrix_s(const Graph& g);
        std::shared_ptr<matrix_t> create_interlevel_matrix_l(
            const Graph& g, 
            const matrix_t& outlink_matrix
        );
        std::shared_ptr<vector_t> iterate(
            const matrix_t& outlink_matrix, 
            const sparce_vector_t& outlink_vector, 
            const matrix_t& interlevel_matrix_s, 
            const matrix_t& interlevel_matrix_l, 
            const vector_t& previous,
            const vector_t& teleportation
        );
        Graph create_graph(const matrix_t& m);
    };
}

#endif /* NCD_AWARE_RANKING_HPP */

