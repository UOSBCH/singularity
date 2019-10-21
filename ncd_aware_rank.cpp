#include "include/ncd_aware_rank.hpp"
#include <boost/numeric/ublas/io.hpp>

using namespace boost;
using namespace boost::numeric::ublas;
using namespace singularity;

std::shared_ptr<vector_t> ncd_aware_rank::process(
        const matrix_t& outlink_matrix,
        const vector_t& initial_vector,
        const vector_t& weight_vector,
        const additional_matrices_vector& additional_matrices
) const {
//     sparce_vector_t v = matrix_tools::calculate_correction_vector(outlink_matrix);
    Graph g = create_graph(outlink_matrix);
    scan scan(parameters.clustering_e, parameters.clustering_m);
    scan.process(g);
    std::shared_ptr<matrix_t> ms = create_interlevel_matrix_s(g);
    std::shared_ptr<matrix_t> ml = create_interlevel_matrix_l(g, outlink_matrix);
    
    return calculate_rank(outlink_matrix, additional_matrices, *ms, *ml, initial_vector);
}

double_type ncd_aware_rank::get_teleportation_weight() const
{
    return double_type(1) - parameters.outlink_weight - parameters.interlevel_weight;
}

std::shared_ptr<vector_t> ncd_aware_rank::iterate(
        const matrix_t& outlink_matrix, 
        const additional_matrices_vector& additional_matrices,
        const matrix_t& interlevel_matrix_s, 
        const matrix_t& interlevel_matrix_l, 
        const vector_t& previous,
        const vector_t& teleportation
) const {
//     unsigned int num_accounts = outlink_matrix.size2();
    vector_t tmp(interlevel_matrix_l.size1(), 0); 
    matrix_tools::prod(tmp, interlevel_matrix_l, previous, parameters.num_threads);
    
    vector_t tmp2(interlevel_matrix_s.size1(), 0);
    std::shared_ptr<vector_t> next(new vector_t(outlink_matrix.size1(), 0));
    
    matrix_tools::prod(*next, outlink_matrix, previous, parameters.num_threads);
    matrix_tools::prod(tmp2, interlevel_matrix_s, tmp, parameters.num_threads);
    
    *next += tmp2;

    for (auto additional_matrix: additional_matrices) {
        *next += prod(*additional_matrix, previous) * parameters.outlink_weight;
    }
    
//     vector_t correction_vector(num_accounts, inner_prod(outlink_vector, previous));
    
//     *next += correction_vector;
    *next += teleportation;
    
    return next;
}

std::shared_ptr<vector_t> ncd_aware_rank::calculate_rank(
        const matrix_t& outlink_matrix, 
        const additional_matrices_vector& additional_matrices,
        const matrix_t& interlevel_matrix_s, 
        const matrix_t& interlevel_matrix_l,
        const vector_t& initial_vector
) const {
//     unsigned int num_accounts = outlink_matrix.size2();
//     double_type initialValue = 1.0/num_accounts;
    std::shared_ptr<vector_t> next;
    std::shared_ptr<vector_t> previous = std::make_shared<vector_t>(initial_vector);
    vector_t teleportation = (*previous) * (1.0 - parameters.outlink_weight - parameters.interlevel_weight) ;
    
    matrix_t outlink_matrix_weighted = outlink_matrix * parameters.outlink_weight;
    matrix_t interlevel_matrix_s_weighted = interlevel_matrix_s * parameters.interlevel_weight;
//     sparce_vector_t outlink_vector_weighted = outlink_vector * parameters.outlink_weight;
    
    for (uint i = 0; i < MAX_ITERATIONS; i++) {
        next  = iterate(outlink_matrix_weighted, additional_matrices, interlevel_matrix_s_weighted, interlevel_matrix_l, *previous, teleportation);
        double_type norm = norm_1(*next - *previous);
        if (norm <= parameters.rank_calculation_precision) {
            return next;
        } else {
            previous = next;
        }
    }
    
    return next;
}


std::shared_ptr<matrix_t> ncd_aware_rank::create_interlevel_matrix_s(
    const Graph& g
) const
{
    Graph::vertex_iterator current, end;
    
    unsigned int num_clasters = get_property(g, graph_num_clusters);
    
    std::shared_ptr<matrix_t> S(new matrix_t(num_vertices(g), num_clasters));
    
    tie(current, end) = vertices(g);
    
    for ( ; current != end; current++) {
        unsigned int index = get(vertex_index, g, *current);
        unsigned int cluster_id = get(vertex_cluster_id, g, *current);
        (*S)(index, cluster_id) = 1;
    }

    matrix_tools::normalize_columns(*S);
    
    return S;
}

std::shared_ptr<matrix_t> ncd_aware_rank::create_interlevel_matrix_l(
        const Graph& g, 
        const matrix_t& outlink_matrix
) const
{
    unsigned int num_clusters = get_property(g, graph_num_clusters);

    Graph::vertex_iterator start, end;
    
    tie(start, end) = vertices(g);
    
    std::shared_ptr<matrix_t> L(new matrix_t(num_clusters, num_vertices(g)));
    
    for (matrix_t::const_iterator1 i = outlink_matrix.begin1(); i != outlink_matrix.end1(); i++)
    {
        Graph::vertex_descriptor vertex = start[i.index1()];
        unsigned int clusterId = get(vertex_cluster_id, g, vertex);
        (*L)(clusterId, i.index1()) = 1;
        for (matrix_t::const_iterator2 j = i.begin(); j != i.end(); j++)
        {
            if (*j > 0) {
                (*L)(clusterId, j.index2()) = 1;
            }
        }
    }
    
    matrix_tools::normalize_columns(*L);
    
    return L;
}

Graph ncd_aware_rank::create_graph(const matrix_t& m) const
{
    Graph g(m.size2());
    
    Graph::vertex_iterator v,ve;
    
    tie(v, ve) = vertices(g);

    unsigned int id = 0;
    
    for (matrix_t::const_iterator1 i = m.begin1(); i != m.end1(); i++)
    {
        for (matrix_t::const_iterator2 j = i.begin(); j != i.end(); j++)
        {
            Graph::edge_descriptor edge;
            bool added = false;
            if (*j > 0) {
                tie(edge, added) = add_edge(v[j.index1()], v[j.index2()], g);
                if (added) {
                    put(edge_index, g, edge, id++);
                }
            }
        }
    }
    
    return g;
}
