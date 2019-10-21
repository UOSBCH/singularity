#include "include/page_rank_alt.hpp"
#include <boost/numeric/ublas/io.hpp>

using namespace boost;
using namespace boost::numeric::ublas;
using namespace singularity;

std::shared_ptr<vector_t> page_rank_alt::process(
        const matrix_t& outlink_matrix,
        const vector_t& initial_vector,
        const vector_t& weight_vector,
        const additional_matrices_vector& additional_matrices
) const
{
    return calculate_rank(outlink_matrix, additional_matrices, initial_vector, weight_vector);
}

double_type page_rank_alt::get_teleportation_weight() const
{
    return  double_type(1) - outlink_weight;
}

std::shared_ptr<vector_t> page_rank_alt::iterate(
        const matrix_t& outlink_matrix, 
        const additional_matrices_vector& additional_matrices,
        const vector_t& previous,
        const vector_t& teleportation
) const
{
    std::shared_ptr<vector_t> next(new vector_t(outlink_matrix.size1(), 0));
    
    matrix_tools::prod(*next, outlink_matrix, previous, num_threads);
    
    for (auto additional_matrix: additional_matrices) {
        *next += prod(*additional_matrix, previous) * outlink_weight;
    }
    
//     *next += teleportation;
    
    return next;
}

std::shared_ptr<vector_t> page_rank_alt::calculate_rank(
        const matrix_t& outlink_matrix, 
        const additional_matrices_vector& additional_matrices,
        const vector_t& initial_vector,
        const vector_t& weight_vector
) const
{
    std::shared_ptr<vector_t> next;
    std::shared_ptr<vector_t> result = std::make_shared<vector_t>(weight_vector.size(), 0);
    vector_t teleportation(weight_vector * (double_type(1) - outlink_weight));
    std::shared_ptr<vector_t> previous = std::make_shared<vector_t>(teleportation);
    
    *result += *previous;
    
    matrix_t outlink_matrix_weighted = outlink_matrix * outlink_weight;
    
    for (uint i = 0; i < MAX_ITERATIONS; i++) {
        next  = iterate(outlink_matrix_weighted, additional_matrices, *previous, teleportation);
        double_type norm = norm_1(*next);

        if (norm <= precision) {
            return result;
        } else {
            previous = next;
            *result += *next;
        }
    }
    
    return result;
}


