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
    const std::vector<std::shared_ptr<relation_t> >& transactions
) {
    std::lock_guard<std::mutex> lock(accounts_lock);
    for (unsigned int i=0; i<transactions.size(); i++) {
        std::shared_ptr<relation_t> transaction = transactions[i];
        
        std::shared_ptr<account_id_map_t> source_map, target_map;
        
        auto source_map_iterator = node_maps.find(transaction->get_source_type());
        
        if (source_map_iterator == node_maps.end()) {
            source_map = std::make_shared<account_id_map_t>();
            node_maps[transaction->get_source_type()] = source_map;
        } else {
            source_map = source_map_iterator->second;
        }

        auto target_map_iterator = node_maps.find(transaction->get_target_type());

        if (target_map_iterator == node_maps.end()) {
            target_map = std::make_shared<account_id_map_t>();
            node_maps[transaction->get_target_type()] = target_map;
        } else {
            target_map = target_map_iterator->second;
        }
        
        
        account_id_map_t::iterator found_source = source_map->find(transaction->get_source());
        account_id_map_t::iterator found_target = target_map->find(transaction->get_target());
        if (found_source == source_map->end()) {
            source_map->insert(account_id_map_t::value_type(transaction->get_source(), nodes_count++));
        }
        if (found_target == target_map->end()) {
            target_map->insert(account_id_map_t::value_type(transaction->get_target(), nodes_count++));
        }
    }
}

void activity_index_calculator::add_block(const std::vector<std::shared_ptr<relation_t> >& transactions) {
    std::vector<std::shared_ptr<relation_t> > filtered_transactions = filter_block(transactions);
    std::lock_guard<std::mutex> lock(weight_matrix_lock);
    collect_accounts(filtered_transactions);
    
    total_handled_blocks_count++;
    handled_blocks_count++;
//     if (handled_blocks_count >= parameters.decay_period) {
//         handled_blocks_count -= parameters.decay_period;
//         *p_weight_matrix *= parameters.decay_koefficient;
//     }
    
    if (p_weight_matrix->size1() < nodes_count) {
        matrix_t::size_type new_size = p_weight_matrix->size1();
        while (new_size < nodes_count) {
            new_size *= 2;
        }
        p_weight_matrix->resize(new_size, new_size);
    }

    update_weight_matrix(*p_weight_matrix, filtered_transactions);
}

std::vector<std::shared_ptr<relation_t> > singularity::activity_index_calculator::filter_block(const std::vector<std::shared_ptr<relation_t> >& block)
{
    if (!p_filter) {
        return block;
    } else {
        
        std::vector<std::shared_ptr<relation_t> > filtered_block;
        
        for (auto transaction: block) {
            if (p_filter->check(transaction)) {
                filtered_block.push_back(transaction);
            }
        }
        
        return filtered_block;
    }
}

void activity_index_calculator::skip_blocks(unsigned int blocks_count)
{
    std::lock_guard<std::mutex> lock(weight_matrix_lock);
    total_handled_blocks_count += blocks_count;
    handled_blocks_count += blocks_count;
    
//     if (handled_blocks_count >= parameters.decay_period) {
//         unsigned int decay_period_count = handled_blocks_count / parameters.decay_period;
//         handled_blocks_count = handled_blocks_count - decay_period_count * parameters.decay_period;
//         double_type decay_value = boost::multiprecision::pow(parameters.decay_koefficient, decay_period_count);
//         *p_weight_matrix *= decay_value;
//     }
}

std::map<node_type, std::shared_ptr<account_activity_index_map_t> > activity_index_calculator::calculate()
{
    if (nodes_count == 0) {
        return std::map<node_type, std::shared_ptr<account_activity_index_map_t> >();
    }
    matrix_t outlink_matrix(nodes_count, nodes_count);

    additional_matrices_vector additional_matrices;
    
    vector_t initial_vector = create_initial_vector();
    
    calculate_outlink_matrix(outlink_matrix, *p_weight_matrix, additional_matrices, initial_vector);
    
    std::shared_ptr<vector_t> rank = p_rank_calculator->process(outlink_matrix, initial_vector, initial_vector, additional_matrices);
    
    return calculate_score(*rank);
}

void activity_index_calculator::calculate_outlink_matrix(
    matrix_t& o,
    matrix_t& weight_matrix,
    additional_matrices_vector& additional_matrices,
    const vector_t& initial_vector
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
                   
//                 o(j.index2(), j.index1()) += *j;
// 
//                 if (is_transfer) {
//                     o(j.index1(), j.index2()) -= *j;
//                 }
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
    
    normalize_columns(o, additional_matrices, initial_vector);
//     matrix_tools::normalize_columns(o);
}

void activity_index_calculator::update_weight_matrix(matrix_t& weight_matrix, const std::vector<std::shared_ptr<relation_t> >& transactions) {
    for (unsigned int i=0; i<transactions.size(); i++) {
        std::shared_ptr<relation_t> t = transactions[i];
        double_type decay_value;
        if (t->is_decayable()) {
            decay_value = p_decay_manager->get_decay_value(t->get_height());
        } else {
            decay_value = 1;
        }
        
        weight_matrix(node_maps[t->get_source_type()]->at(t->get_source()), node_maps[t->get_target_type()]->at(t->get_target())) += decay_value * t->get_reverse_weight();
        weight_matrix(node_maps[t->get_target_type()]->at(t->get_target()), node_maps[t->get_source_type()]->at(t->get_source())) += decay_value * t->get_weight();
    }
}

std::map<node_type, std::shared_ptr<account_activity_index_map_t> > activity_index_calculator::calculate_score(
        const vector_t& rank
)
{
    std::map<node_type, std::shared_ptr<account_activity_index_map_t> > result;

    for (auto node_map_it: node_maps) {
        result[node_map_it.first] = std::make_shared<account_activity_index_map_t>();
        std::shared_ptr<account_id_map_t> node_map = node_map_it.second;
        for (auto node_it: *node_map) {
            (*result[node_map_it.first])[node_it.first] = rank[node_it.second];
        }
    }

    return result;
}

unsigned int activity_index_calculator::get_total_handled_block_count() 
{
    return total_handled_blocks_count;
}

singularity::parameters_t singularity::activity_index_calculator::get_parameters()
{
    return parameters;
}

void singularity::activity_index_calculator::set_parameters(singularity::parameters_t params)
{
    parameters = params;
}

void activity_index_calculator::normalize_columns(matrix_t &m, additional_matrices_vector& additional_matrices, const vector_t& initial_vector)
{
//     auto node_type_count = node_maps.size();
    std::vector<node_type> reverse_map(nodes_count);
    node_type_map<sparce_vector_t> outlink_vectors; 
    node_type_map<sparce_vector_t> mask_vectors;
    node_type_map<sparce_vector_t> scale_vectors;
    node_type_map<sparce_vector_t> sum_vectors;
    node_type_map<sparce_vector_t> min_vectors;
    
    std::map<node_type, double_type> partial_norms;
    double_type norm(0);
    
    
    for (auto node_map_it: node_maps) {
        std::shared_ptr<account_id_map_t> node_map = node_map_it.second;
        node_type current_node_type = node_map_it.first;
        
        outlink_vectors[current_node_type] = std::make_shared<sparce_vector_t>(m.size2());
        mask_vectors[current_node_type] = std::make_shared<sparce_vector_t>(m.size2());
        scale_vectors[current_node_type] = std::make_shared<sparce_vector_t>(m.size2());
        sum_vectors[current_node_type] = std::make_shared<sparce_vector_t>(m.size2());
        min_vectors[current_node_type] = std::make_shared<sparce_vector_t>(m.size2());
        
        partial_norms[current_node_type] = 0;
        
        for (auto node_it: *node_map) {
            reverse_map[node_it.second] = current_node_type;
            sparce_vector_t& mask_vector = *mask_vectors[current_node_type];
            mask_vector(node_it.second) = 1;
            partial_norms[current_node_type] += initial_vector[node_it.second];
            norm += initial_vector[node_it.second];
            
        }
        
    }

    if (node_maps.find(node_type::CONTENT) != node_maps.end()) {
        for(auto node_it: *(node_maps[node_type::CONTENT])) {
            m(node_it.second, node_it.second) = 1;
        }
    }
//     if (node_maps.find(node_type::ACCOUNT) != node_maps.end()) {
//         if (!disable_negative_weights) {
//             for(auto node_it: *(node_maps[node_type::ACCOUNT])) {
//                  m(node_it.second, node_it.second) = 1;
//             }
//         }
//     }
    
    for (matrix_t::iterator1 i = m.begin1(); i != m.end1(); i++)
    {
        
        node_type current_node_type = reverse_map[i.index1()];
        
        sparce_vector_t& scale_vector = *sum_vectors[current_node_type];
        sparce_vector_t& min_vector = *min_vectors[current_node_type];
        
        for (matrix_t::iterator2 j = i.begin(); j != i.end(); j++)
        {
            if (*j != double_type (0) ) {
                scale_vector(j.index2()) += *j;
            }
            if (*j < double_type(min_vector(j.index2()))) {
                min_vector(j.index2()) = *j;
            }
        }
    }
    
    for (auto node_map_it: node_maps) {
        std::shared_ptr<account_id_map_t> node_map = node_map_it.second;
        
        node_type current_node_type = node_map_it.first;
        
        
        sparce_vector_t& scale_vector = *scale_vectors[current_node_type];
        sparce_vector_t& sum_vector = *sum_vectors[current_node_type];
        sparce_vector_t& min_vector = *min_vectors[current_node_type];
        sparce_vector_t& outlink_vector = *outlink_vectors[current_node_type];
        
        for(sparce_vector_t::size_type i = 0; i < sum_vector.size(); i++) {
            double_type c = 0;
            if (min_vector(i) < double_type(0) ) {
                c = double_type(min_vector(i)) * double_type (-1);
            } else if (sum_vector(i) == 0) {
                c = double_type (1);
            }
            scale_vector(i) = partial_norms[current_node_type] / ( norm * (double_type(sum_vector(i)) + node_map->size() * c) );
            outlink_vector(i) = c * double_type(scale_vector(i));
        }
    }
    
    for (matrix_t::iterator1 i = m.begin1(); i != m.end1(); i++)
    {
        node_type current_node_type = reverse_map[i.index1()];
        sparce_vector_t& scale_vector = *scale_vectors[current_node_type];
        
        for (matrix_t::iterator2 j = i.begin(); j != i.end(); j++)
        {
            if (*j != 0) {
                *j *= double_type(scale_vector(j.index2()));
            }
        }
    }
    
    for (auto node_map_it: node_maps) {
        node_type current_node_type = node_map_it.first;
        additional_matrices.push_back(std::make_shared<vector_based_matrix<double_type> >(*(mask_vectors[current_node_type]), *(outlink_vectors[current_node_type])));
    }
    
}

vector_t activity_index_calculator::create_initial_vector()
{
    std::lock_guard<std::mutex> ac_lock(accounts_lock);
    std::lock_guard<std::mutex> wm_lock(weight_matrix_lock);
    
    vector_t result(nodes_count, 0);
    
    for (auto node_map_it: node_maps) {
        
        if (node_map_it.first == node_type::CONTENT) {

            std::shared_ptr<account_id_map_t> node_map = node_map_it.second;
            auto account_node_map = node_maps[node_type::ACCOUNT];
            vector<uint8_t> content_mask_vector(nodes_count, 0);
            vector<uint8_t> account_mask_vector(nodes_count, 0);
            for (auto node_it: *node_map) {
                content_mask_vector[node_it.second] = 1;
            }
            for (auto node_it: *account_node_map) {
                account_mask_vector[node_it.second] = 1;
            }
            matrix_t::size_type size = nodes_count;
            
            for (matrix_t::iterator1 i = p_weight_matrix->begin1(); i != p_weight_matrix->end1(); i++)
            {
                if (i.index1() >= size) {
                    break;
                }
                
                if(account_mask_vector[i.index1()] == 0) {
                    continue;
                }
                
                uint64_t posts_per_account = 0;
                
                for (matrix_t::iterator2 j = i.begin(); j != i.end(); j++)
                {
                    if (j.index2() >= size) {
                        break;
                    }
                    if(content_mask_vector[j.index2()] == 0) {
                        continue;
                    }
                    
                    posts_per_account ++;
                }
                
                if (posts_per_account > 0) {
                    
                    double_type post_initial_rating = double_type(1) / (double_type(account_node_map->size()) * double_type(posts_per_account));
                    
                    for (matrix_t::iterator2 j = i.begin(); j != i.end(); j++)
                    {
                        if (j.index2() >= size) {
                            break;
                        }
                        if(content_mask_vector[j.index2()] == 0) {
                            continue;
                        }
                        
                        result[j.index2()] = post_initial_rating;
                    }
                }
            }
            
        } else {
            std::shared_ptr<account_id_map_t> node_map = node_map_it.second;
            auto node_count = node_map->size();
            double_type init_value = double_type(1) / double_type(node_count);
            for (auto node_it: *node_map) {
                result[node_it.second] = init_value;
            }
         }
        
    }
    
    return result;
}



