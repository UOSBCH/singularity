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
            uint64_t& counter = transaction->get_target_type() == node_type::ACCOUNT ? accounts_count : contents_count;
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
    
    adjust_matrix_sizes();
    
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

std::map<node_type, std::shared_ptr<account_activity_index_map_t> > social_index_calculator::calculate()
{
    if (accounts_count == 0) {
        return std::map<node_type, std::shared_ptr<account_activity_index_map_t> >();
    }
    
    matrix_t outlink_matrix(accounts_count, accounts_count);

    additional_matrices_vector additional_matrices;
    
    matrix_t vote_matrix_new(p_vote_matrix->size1(), p_vote_matrix->size2());
    matrix_tools::prod(vote_matrix_new, *p_repost_matrix, *p_vote_matrix);
    
    vote_matrix_new = vote_matrix_new + *p_vote_matrix;
    
    matrix_t weight_matrix(p_ownership_matrix->size1(), vote_matrix_new.size2());
    matrix_tools::prod(weight_matrix, *p_ownership_matrix, vote_matrix_new);
    
    limit_values(weight_matrix);
    
    calculate_outlink_matrix(outlink_matrix, weight_matrix, additional_matrices);

    vector_t default_initial_vector = create_default_initial_vector();
    vector_t stack_vector = create_stack_vector();
    
    std::shared_ptr<vector_t> p_account_rank;
    
    p_account_rank = p_rank_calculator->process(outlink_matrix, default_initial_vector, default_initial_vector, additional_matrices);
    
    if (parameters.include_detailed_data) {
        calculate_detalization(outlink_matrix, *p_account_rank, stack_vector, default_initial_vector, additional_matrices);
    }
    
    if (norm_1(stack_vector) > 0) {
        vector_t stack_vector_part = prod(outlink_matrix, stack_vector);
        for (auto additional_matrix: additional_matrices) {
            stack_vector_part += prod(*additional_matrix, stack_vector);
        }
        *p_account_rank = *p_account_rank * (double_type(0.5)) + stack_vector_part * (double_type(0.5));
    }
    
    matrix_t content_outlink_matrix(contents_count, accounts_count);
    additional_matrices_vector content_additional_matrices;

    calculate_outlink_matrix(content_outlink_matrix, vote_matrix_new, content_additional_matrices);
    
    auto content_rank = prod(content_outlink_matrix, *p_account_rank);
    
    return calculate_score(*p_account_rank, content_rank);
}

void social_index_calculator::calculate_outlink_matrix(
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
    
    normalize_columns(o, additional_matrices);
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
        
        if (t->get_name() == "OWNERSHIP") {
            (*p_ownership_matrix)(account_map[t->get_source()], content_map[t->get_target()]) = 1;
        }
        
        if (t->get_name() == "UPVOTE") {
            (*p_vote_matrix)(content_map[t->get_target()], account_map[t->get_source()]) = 1;
        }

        if (t->get_name() == "REPOST") {
            (*p_repost_matrix)(content_map[t->get_target()], content_map[t->get_source()]) = 1;
            (*p_repost_matrix)(content_map[t->get_source()], content_map[t->get_source()]) = -1;
        }
    }
}

std::map<node_type, std::shared_ptr<account_activity_index_map_t> > social_index_calculator::calculate_score(
    const vector_t& account_rank,
    const vector_t& content_rank
)
{
    std::map<node_type, std::shared_ptr<account_activity_index_map_t> > result;
    
    auto account_rank_map = std::make_shared<account_activity_index_map_t>();
    auto content_rank_map  = std::make_shared<account_activity_index_map_t>();

    for (auto node_it: account_map) {
        (*account_rank_map)[node_it.first] = account_rank[node_it.second];
    }
    for (auto node_it: content_map) {
        (*content_rank_map)[node_it.first] = content_rank[node_it.second];
    }
    
    result[node_type::ACCOUNT] = account_rank_map;
    result[node_type::CONTENT] = content_rank_map;

    return result;
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

void social_index_calculator::normalize_columns(matrix_t &m, additional_matrices_vector& additional_matrices)
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

vector_t social_index_calculator::create_default_initial_vector()
{
    std::lock_guard<std::mutex> ac_lock(accounts_lock);
    
    return vector_t(accounts_count, double_type(1)/accounts_count);
}

void social_index_calculator::add_stack_vector(const std::map<std::string, double_type>& stacks)
{
    stack_map = stacks;
}

void social_index_calculator::limit_values(matrix_t& m)
{
    for (matrix_t::iterator1 i = m.begin1(); i != m.end1(); i++)
    {
        for (matrix_t::iterator2 j = i.begin(); j != i.end(); j++)
        {
            if (*j > 0) {
                *j = 1;
            }
        }
    }
}

void social_index_calculator::adjust_matrix_sizes()
{
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

    if (p_ownership_matrix->size2() < contents_count || p_ownership_matrix->size1() < accounts_count) {
        matrix_t::size_type new_size_1 = p_ownership_matrix->size1();
        matrix_t::size_type new_size_2 = p_ownership_matrix->size2();
        while (new_size_2 < contents_count) {
            new_size_2 *= 2;
        }
        while (new_size_1 < accounts_count) {
            new_size_1 *= 2;
        }
        p_ownership_matrix->resize(new_size_1, new_size_2);
    }

    if (p_repost_matrix->size1() < contents_count || p_repost_matrix->size2() < contents_count) {
        matrix_t::size_type new_size_1 = p_repost_matrix->size1();
        matrix_t::size_type new_size_2 = p_repost_matrix->size2();
        while (new_size_1 < contents_count) {
            new_size_1 *= 2;
        }
        while (new_size_2 < contents_count) {
            new_size_2 *= 2;
        }
        p_repost_matrix->resize(new_size_1, new_size_2);
    }
    
    if (p_comment_matrix->size1() < contents_count || p_comment_matrix->size2() < contents_count) {
        matrix_t::size_type new_size_1 = p_comment_matrix->size1();
        matrix_t::size_type new_size_2 = p_comment_matrix->size2();
        while (new_size_1 < contents_count) {
            new_size_1 *= 2;
        }
        while (new_size_2 < contents_count) {
            new_size_2 *= 2;
        }
        p_comment_matrix->resize(new_size_1, new_size_2);
    }
    
}

vector_t social_index_calculator::create_stack_vector()
{
    vector_t result(accounts_count, 0);
    
    for (auto stack_it: stack_map) {
        std::string account_name = stack_it.first;
        double_type stack_value = stack_it.second;
        
        auto account_it = account_map.find(account_name);
        
        if (account_it != account_map.end()) {
            auto account_id = account_it->second;
            
            result(account_id) = stack_value;
        }
    }
    
    double_type norm = norm_1(result);
    
    if (norm > 0) {
        result *= double_type(1) / norm;
    }
        
    return result;
}

boost::optional<account_id_map_t::mapped_type> social_index_calculator::get_account_id(std::string name, bool allow_create)
{
    std::lock_guard<std::mutex> lock(accounts_lock);
    
    auto item_it = account_map.find(name);
    
    if (item_it != account_map.end()) {
        auto id = item_it->second;
        
        return id;
    } else if (allow_create) {
        
        auto id = accounts_count++;
        
        account_map[name] = id;
        
        return id;
    }
    
    return boost::none;
}

boost::optional<account_id_map_t::mapped_type> social_index_calculator::get_content_id(std::string name, bool allow_create)
{
    std::lock_guard<std::mutex> lock(accounts_lock);
    
    auto item_it = content_map.find(name);
    
    if (item_it != content_map.end()) {
        auto id = item_it->second;
        
        return id;
    } else if (allow_create) {
        
        auto id = contents_count++;
        
        content_map[name] = id;
        
        return id;
    }
    
    return boost::none;
}

void social_index_calculator::calculate_detalization (
    const matrix_t& outlink_matrix, 
    const vector_t& activity_index_vector,
    const vector_t& stack_vector, 
    const vector_t& weight_vector, 
    const additional_matrices_vector& additional_matrices
) 
{
    std::lock_guard<std::mutex> lock(accounts_lock);
    
    double_type stack_share, activity_share;
    
    if (norm_1(stack_vector) == 0) {
        activity_share = 1;
        stack_share = 0;
    } else {
        activity_share = 0.5;
        stack_share = 0.5;
    }
    
    std::vector<std::string> reverse_account_map(account_map.size());
    
    vector_t base_vector(account_map.size(), 0);
    
    base_vector += activity_share * (double_type(1) - parameters.outlink_weight) * weight_vector;
    
    for (auto it: additional_matrices) {
        base_vector += activity_share * parameters.outlink_weight * prod(*it, activity_index_vector);
        if (stack_share > 0) {
            base_vector += stack_share * prod(*it, stack_vector);
        }
    }
    
    for (auto it: account_map) {
        std::string name = it.first;
        size_t id = it.second;
        reverse_account_map[id] = name;
        
        detalization.base_index[name] = base_vector[id];
    }
    
    for (auto i = outlink_matrix.cbegin1(); i != outlink_matrix.cend1(); i++) {
        
        if (i.index1() >= account_map.size()) {
            break;
        }
        
        for (auto j = i.cbegin(); j != i.cend(); j++) {
            if (j.index2() >= account_map.size()) {
                break;
            }
            
            contribution_t a_contribution;
            
            a_contribution.koefficient = activity_share * parameters.outlink_weight * outlink_matrix(j.index1(), j.index2());
            a_contribution.rate = activity_index_vector(j.index2());
            
            detalization.activity_index_contribution[reverse_account_map[j.index1()]][reverse_account_map[j.index2()]] = a_contribution;
            
            if (stack_share > 0) {
                contribution_t s_contribution;
            
                s_contribution.koefficient = stack_share * outlink_matrix(j.index1(), j.index2());
                s_contribution.rate = stack_vector(j.index2());
            
                detalization.stack_contribution[reverse_account_map[j.index1()]][reverse_account_map[j.index2()]] = s_contribution;
            }
        }
    }
}

