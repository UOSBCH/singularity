
#ifndef RANK_INTERFACE_HPP
#define RANK_INTERFACE_HPP

#include "utils.hpp"

namespace singularity {
    
    class rank_interface
    {
    public:
        virtual std::shared_ptr<vector_t> process(
            const matrix_t& outlink_matrix,
            const vector_t& initial_vector
        ) = 0;
    };
}
#endif /* RANK_INTERFACE_HPP */
