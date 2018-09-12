
#ifndef RANK_INTERFACE_HPP
#define RANK_INTERFACE_HPP

#include "utils.hpp"
#include "relations.hpp"

namespace singularity {
    
    class rank_interface
    {
    public:
        virtual std::shared_ptr<vector_t> process(
            const matrix_t& outlink_matrix,
            const vector_t& initial_vector,
            const node_type_map<sparce_vector_t>& outlink_vectors, 
            const node_type_map<sparce_vector_t>& mask_vectors
        ) = 0;
    };
}
#endif /* RANK_INTERFACE_HPP */
