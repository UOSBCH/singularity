#include "include/activity_index_calculator.hpp"
#include "include/ncd_aware_rank.hpp"
#include <ctime>
#include <boost/numeric/ublas/io.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <fstream>
#include <thread>
#include "include/page_rank.hpp"
#include "include/vector_based_matrix.hpp"

using namespace boost::numeric::ublas;
using namespace boost;
using namespace singularity;

void activity_index_calculator::collect_accounts(
    const relation_t& relation
) {
    
    std::lock_guard<std::mutex> lock(accounts_lock);

    if (relation.get_source_type() == node_type::ACCOUNT) {
        get_account_id(relation.get_source(), true);
    }

    if (relation.get_target_type() == node_type::ACCOUNT) {
        get_account_id(relation.get_target(), true);
    }

    p_weight_matrix->set_real_size(nodes_count, nodes_count);
}

void activity_index_calculator::add_block(const std::vector<std::shared_ptr<relation_t> >& relations) {

    for (auto p_relation: relations) {
        add_relation(*p_relation);
    }
}

void activity_index_calculator::add_relation(const relation_t& relation)
{
    std::lock_guard<std::mutex> lock(weight_matrix_lock);
//    if (parameters.extended_logging) {
//        exporter.export_relation(relation);
//    }

    if (p_filter && !p_filter->check(relation)) {
        return;
    }

    collect_accounts(relation);

    double_type decay_value =
        relation.is_decayable() ?
            p_decay_manager->get_decay_value(relation.get_height()) :
            1;

    if (relation.get_name() == transaction_t::NAME) {
        (*p_weight_matrix)(account_map[relation.get_target()], account_map[relation.get_source()]) +=   decay_value * relation.get_weight();
        (*p_weight_matrix)(account_map[relation.get_source()], account_map[relation.get_target()]) += - decay_value * relation.get_weight();
    }
}

std::map<node_type, std::shared_ptr<account_activity_index_map_t> > activity_index_calculator::calculate()
{
    if (nodes_count == 0) {
        return std::map<node_type, std::shared_ptr<account_activity_index_map_t> >();
    }
    matrix_t outlink_matrix(nodes_count, nodes_count);

    additional_matrices_vector additional_matrices;
    
    vector_t initial_vector = create_initial_vector();
    
    calculate_outlink_matrix(outlink_matrix, *p_weight_matrix, additional_matrices);
    
    std::shared_ptr<vector_t> rank = p_rank_calculator->process(outlink_matrix, initial_vector, initial_vector, additional_matrices);
    
    return calculate_score(*rank);
}

void activity_index_calculator::calculate_outlink_matrix(
    matrix_t& o,
    matrix_t& weight_matrix,
    additional_matrices_vector& additional_matrices
)
{
    matrix_t::size_type size = o.size1();
    {
        std::lock_guard<std::mutex> lock(weight_matrix_lock);

        for (matrix_t::iterator1 i = weight_matrix.begin1(); i != weight_matrix.end1(); i++)
        {
            if (i.index1() >= size) {
                break;
            }
            for (matrix_t::iterator2 j = i.begin(); j != i.end(); j++)
            {
                if (j.index2() >= size) {
                    break;
                }

                o(j.index1(), j.index2()) += *j;
            }
        }
    }

    if (disable_negative_weights) {
        for (matrix_t::iterator1 i = o.begin1(); i != o.end1(); i++)
        {
            for (matrix_t::iterator2 j = i.begin(); j != i.end(); j++)
            {
                if (*j < 0) {
                    *j = 0;
                }
            }
        }
    }
    
    normalize_columns(o, additional_matrices);
}

std::map<node_type, std::shared_ptr<account_activity_index_map_t> > activity_index_calculator::calculate_score(
        const vector_t& rank
)
{
    std::map<node_type, std::shared_ptr<account_activity_index_map_t> > result;

    auto account_rank_map = std::make_shared<account_activity_index_map_t>();

    for (auto node_it: account_map) {
        (*account_rank_map)[node_it.first] = rank[node_it.second];
    }
    
    result[node_type::ACCOUNT] = account_rank_map;

    return result;
    
}

singularity::parameters_t singularity::activity_index_calculator::get_parameters()
{
    return parameters;
}

void singularity::activity_index_calculator::set_parameters(singularity::parameters_t params)
{
    validator.validate(params);
    parameters = params;
}

void activity_index_calculator::normalize_columns(matrix_t &m, additional_matrices_vector& additional_matrices)
{
    sparce_vector_t outlink_vector(m.size2());
    sparce_vector_t mask_vector(m.size2());
    sparce_vector_t scale_vector(m.size2());
    sparce_vector_t sum_vector(m.size2());
    sparce_vector_t min_vector(m.size2());
    
    for (matrix_t::iterator1 i = m.begin1(); i != m.end1(); i++)
    {
        for (matrix_t::iterator2 j = i.begin(); j != i.end(); j++)
        {
            if (*j != double_type (0) ) {
                sum_vector(j.index2()) += *j;
            }
            if (*j < double_type(min_vector(j.index2()))) {
                min_vector(j.index2()) = *j;
            }
        }
    }
    
    for(sparce_vector_t::size_type i = 0; i < sum_vector.size(); i++) {
        double_type c = 0;
        if (min_vector(i) < double_type(0) ) {
            c = double_type(min_vector(i)) * double_type (-1);
        } else if (sum_vector(i) == 0) {
            c = double_type (1);
        }
        scale_vector(i) = double_type(1) / ( (double_type(sum_vector(i)) + sum_vector.size() * c) );
        outlink_vector(i) = c * double_type(scale_vector(i));
    }
    
    for (matrix_t::iterator1 i = m.begin1(); i != m.end1(); i++)
    {
        for (matrix_t::iterator2 j = i.begin(); j != i.end(); j++)
        {
            if (*j != 0) {
                *j *= double_type(scale_vector(j.index2()));
            }
        }
    }
    
    additional_matrices.push_back(std::make_shared<vector_based_matrix<double_type> >(vector_t(m.size1(), 1), outlink_vector));
}

vector_t activity_index_calculator::create_initial_vector()
{
    std::lock_guard<std::mutex> ac_lock(accounts_lock);
    
    return vector_t(nodes_count, double_type(1)/nodes_count);
}

boost::optional<account_id_map_t::mapped_type> activity_index_calculator::get_account_id(std::string name, bool allow_create)
{
    auto item_it = account_map.find(name);
    
    if (item_it != account_map.end()) {
        auto id = item_it->second;
        
        return id;
    } else if (allow_create) {
        
        auto id = nodes_count++;
        
        account_map[name] = id;
        
        return id;
    }
    
    return boost::none;
}



