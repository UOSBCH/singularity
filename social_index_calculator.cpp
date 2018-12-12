#include "include/social_index_calculator.hpp"
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

void social_index_calculator::collect_accounts(
    const std::vector<std::shared_ptr<relation_t> >& transactions
) {
    std::lock_guard<std::mutex> lock(accounts_lock);
    for (unsigned int i=0; i<transactions.size(); i++) {
        std::shared_ptr<relation_t> transaction = transactions[i];

        account_id_map_t& source_map = transaction->get_source_type() == node_type::ACCOUNT ? account_map : content_map;
        account_id_map_t& target_map = transaction->get_target_type() == node_type::ACCOUNT ? account_map : content_map;
        
        account_id_map_t::iterator found_source = source_map.find(transaction->get_source());
        account_id_map_t::iterator found_target = target_map.find(transaction->get_target());
        if (found_source == source_map.end()) {
            uint64_t& counter = transaction->get_source_type() == node_type::ACCOUNT ? accounts_count : contents_count;
            source_map.insert(account_id_map_t::value_type(transaction->get_source(), counter++));
        }
        if (found_target == target_map.end()) {
            uint64_t& counter = transaction->get_source_type() == node_type::ACCOUNT ? accounts_count : contents_count;
            target_map.insert(account_id_map_t::value_type(transaction->get_target(), counter++));
        }
    }
}

void social_index_calculator::add_block(const std::vector<std::shared_ptr<relation_t> >& transactions) {
    std::vector<std::shared_ptr<relation_t> > filtered_transactions = filter_block(transactions);
    std::lock_guard<std::mutex> lock(weight_matrix_lock);
    collect_accounts(filtered_transactions);
    
    total_handled_blocks_count++;
    handled_blocks_count++;
//     if (handled_blocks_count >= parameters.decay_period) {
//         handled_blocks_count -= parameters.decay_period;
//         *p_weight_matrix *= parameters.decay_koefficient;
//     }
    
    
    
    if (p_vote_matrix->size1() < contents_count || p_vote_matrix->size2() < accounts_count) {
        matrix_t::size_type new_size_1 = p_vote_matrix->size1();
        matrix_t::size_type new_size_2 = p_vote_matrix->size2();
        while (new_size_1 < contents_count) {
            new_size_1 *= 2;
        }
        while (new_size_2 < accounts_count) {
            new_size_2 *= 2;
        }
        p_vote_matrix->resize(new_size_1, new_size_2);
    }

    if (p_hierarchy_matrix->size2() < contents_count || p_hierarchy_matrix->size1() < accounts_count) {
        matrix_t::size_type new_size_1 = p_hierarchy_matrix->size1();
        matrix_t::size_type new_size_2 = p_hierarchy_matrix->size2();
        while (new_size_2 < contents_count) {
            new_size_2 *= 2;
        }
        while (new_size_1 < accounts_count) {
            new_size_1 *= 2;
        }
        p_hierarchy_matrix->resize(new_size_1, new_size_2);
    }
    
    update_weight_matrix(filtered_transactions);
}

std::vector<std::shared_ptr<relation_t> > social_index_calculator::filter_block(const std::vector<std::shared_ptr<relation_t> >& block)
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

void social_index_calculator::skip_blocks(unsigned int blocks_count)
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

rate_t social_index_calculator::calculate()
{
    if (accounts_count == 0) {
        return rate_t(account_activity_index_map_t(), account_activity_index_map_t());
    }
    
    matrix_t outlink_matrix(accounts_count, accounts_count);

    additional_matrices_vector additional_matrices;
    
    vector_t initial_vector = create_initial_vector();
    
    matrix_t weight_matrix = prod(*p_hierarchy_matrix, *p_vote_matrix);
    
    calculate_outlink_matrix(outlink_matrix, weight_matrix, additional_matrices, initial_vector);
    
    std::shared_ptr<vector_t> account_rank = p_rank_calculator->process(outlink_matrix, initial_vector, initial_vector, additional_matrices);
    
    auto content_rank = prod(*p_vote_matrix, *account_rank);
    
    return calculate_score(*account_rank, content_rank);
}

void social_index_calculator::calculate_outlink_matrix(
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

void social_index_calculator::update_weight_matrix(const std::vector<std::shared_ptr<relation_t> >& transactions) {
    for (unsigned int i=0; i<transactions.size(); i++) {
        std::shared_ptr<relation_t> t = transactions[i];
        double_type decay_value;
        if (t->is_decayable()) {
            decay_value = p_decay_manager->get_decay_value(t->get_height());
        } else {
            decay_value = 1;
        }
        
        if (auto ownership = dynamic_pointer_cast<ownwership_t>(t)) {
            (*p_hierarchy_matrix)(account_map[ownership->get_source()], content_map[ownership->get_target()]) = 1;
        }
        
        if (auto upvote = dynamic_pointer_cast<upvote_t>(t)) {
            (*p_vote_matrix)(content_map[upvote->get_target()], account_map[upvote->get_source()]) = 1;
        }
    }
}

rate_t social_index_calculator::calculate_score(
    const vector_t& account_rank,
    const vector_t& content_rank
)
{
    account_activity_index_map_t account_rank_map;
    account_activity_index_map_t content_rank_map;

    for (auto node_it: account_map) {
        account_rank_map[node_it.first] = account_rank[node_it.second];
    }
    for (auto node_it: content_map) {
        content_rank_map[node_it.first] = content_rank[node_it.second];
    }

    return rate_t(account_rank_map, content_rank_map);
}

unsigned int social_index_calculator::get_total_handled_block_count() 
{
    return total_handled_blocks_count;
}

singularity::parameters_t singularity::social_index_calculator::get_parameters()
{
    return parameters;
}

void singularity::social_index_calculator::set_parameters(singularity::parameters_t params)
{
    parameters = params;
}

void social_index_calculator::normalize_columns(matrix_t &m, additional_matrices_vector& additional_matrices, const vector_t& initial_vector)
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
        scale_vector(i) = double_type(1) / ( (double_type(sum_vector(i)) + c) );
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

vector_t social_index_calculator::create_initial_vector()
{
    std::lock_guard<std::mutex> ac_lock(accounts_lock);
    
    return vector_t(accounts_count, double_type(1)/accounts_count);
}



