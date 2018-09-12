#include "include/page_rank.hpp"
#include <boost/numeric/ublas/io.hpp>

using namespace boost;
using namespace boost::numeric::ublas;
using namespace singularity;

std::shared_ptr<vector_t> page_rank::process(
        const matrix_t& outlink_matrix,
        const vector_t& initial_vector,
        const node_type_map<sparce_vector_t>& outlink_vectors, 
        const node_type_map<sparce_vector_t>& mask_vectors            
) {
//     sparce_vector_t v = matrix_tools::calculate_correction_vector(outlink_matrix);
    
    return calculate_rank(outlink_matrix, outlink_vectors, mask_vectors, initial_vector);
}

std::shared_ptr<vector_t> page_rank::iterate(
        const matrix_t& outlink_matrix, 
        const node_type_map<sparce_vector_t>& outlink_vectors, 
        const node_type_map<sparce_vector_t>& mask_vectors,            
        const vector_t& previous,
        const vector_t& teleportation
) {
//     unsigned int num_accounts = outlink_matrix.size2();
    
    std::shared_ptr<vector_t> next(new vector_t(outlink_matrix.size1(), 0));
    
    matrix_tools::prod(*next, outlink_matrix, previous, parameters.num_threads);
    
//     vector_t masked_previous(num_accounts);
    
    for (auto outlink_vector_it: outlink_vectors) {
        auto node_type_id = outlink_vector_it.first;
        auto p_outlink_vector = outlink_vector_it.second;
        auto p_mask_vector = mask_vectors.at(node_type_id);
        
        *next += *p_mask_vector * ((1 - TELEPORTATION_WEIGHT) * inner_prod(*p_outlink_vector, previous));
    }
    
//     vector_t correction_vector(num_accounts, (1 - TELEPORTATION_WEIGHT) * inner_prod(outlink_vector, previous));
    
//     *next += correction_vector;
    *next += teleportation;
    
    return next;
}

std::shared_ptr<vector_t> page_rank::calculate_rank(
        const matrix_t& outlink_matrix, 
        const node_type_map<sparce_vector_t>& outlink_vectors, 
        const node_type_map<sparce_vector_t>& mask_vectors,            
        const vector_t& initial_vector
) {
    unsigned int num_accounts = outlink_matrix.size2();
//     double_type initialValue = 1.0/num_accounts;
    std::shared_ptr<vector_t> next;
    std::shared_ptr<vector_t> previous(new vector_t(initial_vector));
    vector_t teleportation(num_accounts, TELEPORTATION_WEIGHT * norm_1(initial_vector) / num_accounts);
    
    matrix_t outlink_matrix_weighted = outlink_matrix * (double_type(1) - TELEPORTATION_WEIGHT);
//     sparce_vector_t outlink_vector_weighted = outlink_vector * (double_type(1) - TELEPORTATION_WEIGHT);
    
    for (uint i = 0; i < MAX_ITERATIONS; i++) {
        next  = iterate(outlink_matrix_weighted, outlink_vectors, mask_vectors, *previous, teleportation);
        double_type norm = norm_1(*next - *previous);

        if (norm <= precision) {
            return next;
        } else {
            previous = next;
        }
    }
    
    return next;
}

