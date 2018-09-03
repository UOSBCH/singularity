#include "include/activity_index_calculator.hpp"
#include "include/ncd_aware_rank.hpp"
#include <ctime>
#include <boost/numeric/ublas/io.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <fstream>
#include <thread>
#include "include/page_rank.hpp"

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
    
    if (p_weight_matrix->size1() < account_map.size()) {
        matrix_t::size_type new_size = p_weight_matrix->size1();
        while (new_size < account_map.size()) {
            new_size *= 2;
        }
        p_weight_matrix->resize(new_size, new_size);
    }

    update_weight_matrix(*p_weight_matrix, account_map, filtered_transactions);
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
    if (account_map.size() == 0) {
        return std::map<node_type, std::shared_ptr<account_activity_index_map_t> >();
    }
    matrix_t outlink_matrix(account_map.size(), account_map.size());

    calculate_outlink_matrix(outlink_matrix, *p_weight_matrix);
    
    std::shared_ptr<vector_t> rank = p_rank_calculator->process(outlink_matrix, create_initial_vector());
    
    return calculate_score(*rank);
}

void activity_index_calculator::calculate_outlink_matrix(
    matrix_t& o,
    matrix_t& weight_matrix
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

                o(j.index1(), j.index2()) -= *j;
                   
//                 o(j.index2(), j.index1()) += *j;
// 
//                 if (is_transfer) {
//                     o(j.index1(), j.index2()) -= *j;
//                 }
            }
        }
    }
    
    for (matrix_t::iterator1 i = o.begin1(); i != o.end1(); i++)
    {
        for (matrix_t::iterator2 j = i.begin(); j != i.end(); j++)
        {
            if (*j < 0) {
                *j = 0;
            }
        }
    }
    
    matrix_tools::normalize_columns(o);
}

void activity_index_calculator::update_weight_matrix(matrix_t& weight_matrix, account_id_map_t& account_id_map, const std::vector<std::shared_ptr<relation_t> >& transactions) {
    for (unsigned int i=0; i<transactions.size(); i++) {
        std::shared_ptr<relation_t> t = transactions[i];
        double_type decay_value;
        if (t->is_decayable()) {
            decay_value = p_decay_manager->get_decay_value(t->get_height());
        } else {
            decay_value = 1;
        }
        weight_matrix(account_id_map[t->get_source()], account_id_map[t->get_target()]) += decay_value * t->get_reverse_weight();
        weight_matrix(account_id_map[t->get_target()], account_id_map[t->get_source()]) += decay_value * t->get_weight();
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

void activity_index_calculator::save_state_to_file(std::string filename) 
{
    std::lock_guard<std::mutex> lock(weight_matrix_lock);
    try {
        std::ofstream ofs(filename, std::ofstream::out);
    
        boost::archive::binary_oarchive oarch(ofs);
        oarch << BOOST_SERIALIZATION_NVP(*this);
    } catch (std::ifstream::failure& e) {
        throw runtime_exception("Failed writing to a file " + filename);
    } catch ( boost::archive::archive_exception& e) {
        throw runtime_exception("Failed serialization to a file " + filename);
    }
}

void activity_index_calculator::load_state_from_file(std::string filename) 
{
    std::lock_guard<std::mutex> lock(weight_matrix_lock);
    try {
        std::ifstream ifs(filename, std::istream::in);
        boost::archive::binary_iarchive iarch(ifs);

        iarch >> *this;
    } catch (std::ifstream::failure& e) {
        throw runtime_exception("Failed reading from a file " + filename);
    } catch ( boost::archive::archive_exception& e) {
        throw runtime_exception("Failed deserialization from a file " + filename);
    }
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

void activity_index_calculator::normalize_columns(matrix_t &m)
{
    auto node_type_count = node_maps.size();
    
    for (auto node_map_it: node_maps) {
        std::shared_ptr<account_id_map_t> node_map = node_map_it.second;
//         auto node_count = node_map->size();
        std::set<uint32_t> id_set;
        for (auto node_it: *node_map) {
            id_set.emplace(node_it.second);
        }

        mapped_vector<double_type> f (m.size2());
        mapped_vector<double_type> s (m.size2());
        
        for (matrix_t::iterator1 i = m.begin1(); i != m.end1(); i++)
        {
            if (id_set.find(i.index1()) == id_set.end()) {
                continue;
            }
            
            for (matrix_t::iterator2 j = i.begin(); j != i.end(); j++)
            {
                if (*j != double_type (0) ) {
                    s[j.index2()] += *j;
                }
            }
        }
        for (matrix_t::iterator1 i = m.begin1(); i != m.end1(); i++)
        {
            if (id_set.find(i.index1()) == id_set.end()) {
                continue;
            }
            
            for (matrix_t::iterator2 j = i.begin(); j != i.end(); j++)
            {
                double_type norm = s[j.index2()];
                if (norm != 0) {
                    *j /= norm * node_type_count;
                }
            }
        }
        
    }
    
}

vector_t activity_index_calculator::create_initial_vector()
{
    std::lock_guard<std::mutex> lock(accounts_lock);

    vector_t result(nodes_count, 0);
    
    for (auto node_map_it: node_maps) {
        std::shared_ptr<account_id_map_t> node_map = node_map_it.second;
        auto node_count = node_map->size();
        double_type init_value = 1 / node_count;
        for (auto node_it: *node_map) {
            result[node_it.second] = init_value;
        }
    }
    
    return result;
}



