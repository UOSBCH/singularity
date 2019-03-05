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
    const std::vector<std::shared_ptr<relation_t> >& relations
) {
    std::lock_guard<std::mutex> lock(accounts_lock);
    for (unsigned int i=0; i<relations.size(); i++) {
        std::shared_ptr<relation_t> relation = relations[i];
        
        if (relation->get_source_type() == node_type::ACCOUNT) {
            get_account_id(relation->get_source(), true);
        } else if (relation->get_source_type() == node_type::CONTENT) {
            get_content_id(relation->get_source(), true);
        }

        if (relation->get_target_type() == node_type::ACCOUNT) {
            get_account_id(relation->get_target(), true);
        } else if (relation->get_target_type() == node_type::CONTENT) {
            get_content_id(relation->get_target(), true);
        }
    }
}

void social_index_calculator::add_block(const std::vector<std::shared_ptr<relation_t> >& relations) {
    std::vector<std::shared_ptr<relation_t> > filtered_transactions = filter_block(relations);
    std::lock_guard<std::mutex> lock(weight_matrix_lock);
    collect_accounts(filtered_transactions);
    
    total_handled_blocks_count++;
    handled_blocks_count++;
    
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
}

std::shared_ptr<vector_t> social_index_calculator::calculate_priority_vector()
{
    if (accounts_count == 0) {
        return std::shared_ptr<vector_t>();
    }
    
    matrix_t trust_outlink_matrix(accounts_count, accounts_count);
    additional_matrices_vector trust_additional_matrices;
    
    calculate_outlink_matrix(trust_outlink_matrix, *p_trust_matrix, trust_additional_matrices);

    vector_t stack_vector = create_stack_vector();
    vector_t default_initial_vector = create_default_initial_vector();
    
    vector_t trust_initial_vector = default_initial_vector * double_type(0.1) + stack_vector * double_type(0.9);
    
    auto p_rank = p_rank_calculator->process(trust_outlink_matrix, trust_initial_vector, trust_initial_vector, trust_additional_matrices);
    
    if (parameters.include_detailed_data) {
        calculate_detalization(account_priority_detalization, parameters.outlink_weight, trust_outlink_matrix, *p_rank, trust_initial_vector, trust_additional_matrices);
    }
    
    return p_rank;
}

std::map<node_type, std::shared_ptr<account_activity_index_map_t> > social_index_calculator::calculate()
{
    intermediate_results_t current_intermediate_results;
    
    if (accounts_count == 0) {
        return std::map<node_type, std::shared_ptr<account_activity_index_map_t> >();
    }
    
    vector_t stack_vector = create_stack_vector();
    vector_t default_initial_vector = create_default_initial_vector();
    
    current_intermediate_results.stack = vector2map(stack_vector);
    current_intermediate_results.default_initial = vector2map(default_initial_vector);
    
    auto p_trust_vector = calculate_priority_vector();

    current_intermediate_results.trust = vector2map(*p_trust_vector);
    
    matrix_t outlink_matrix(accounts_count, accounts_count);
    additional_matrices_vector additional_matrices;
    
    matrix_t vote_matrix_with_reposts(p_vote_matrix->size1(), p_vote_matrix->size2());
    matrix_tools::prod(vote_matrix_with_reposts, *p_repost_matrix, *p_vote_matrix);
    
    vote_matrix_with_reposts = vote_matrix_with_reposts + *p_vote_matrix;
    
    matrix_t weight_matrix(p_ownership_matrix->size1(), vote_matrix_with_reposts.size2());
    
    collapse_matrix(weight_matrix, *p_ownership_matrix, vote_matrix_with_reposts);
    
    if (mode == calculation_mode::DIAGONAL) {
        set_diagonal_elements(weight_matrix);
    }

    if (mode == calculation_mode::PHANTOM_ACCOUNT) {
        add_phantom_account_relations(weight_matrix);
    }
    
    calculate_outlink_matrix(outlink_matrix, weight_matrix, additional_matrices);

    vector_t initial_vector;
    vector_t priority_vector = matrix_tools::discretize(*p_trust_vector);
    
    current_intermediate_results.priority = vector2map(priority_vector);
    
    double_type stack_contribution = norm_1(stack_vector) > 0 ? parameters.stack_contribution : 0;
    double_type weight_contribution = norm_1(priority_vector) > 0 ? parameters.weight_contribution : 0;
    double_type const_contribution = double_type(1) - stack_contribution - weight_contribution;
    
    initial_vector = default_initial_vector * const_contribution + priority_vector * weight_contribution + stack_vector * stack_contribution;
    
    std::shared_ptr<vector_t> p_account_rank; 
    vector_t account_rank_final;
    
    p_account_rank = p_rank_calculator->process(outlink_matrix, initial_vector, initial_vector, additional_matrices);

    account_rank_final = *p_account_rank - stack_vector * ((double_type(1) - parameters.outlink_weight) * stack_contribution);
    
    if (norm_1(account_rank_final) > 0) {
        account_rank_final *= double_type(1) / norm_1(account_rank_final);
    }
    
    if (mode == calculation_mode::PHANTOM_ACCOUNT && account_map.size() > 1) {
        double_type k = double_type(1) / (double_type(1) -  account_rank_final[0]);
        account_rank_final *= k;
    }
    
    matrix_t content_matrix(contents_count, accounts_count);

    calculate_content_matrix(content_matrix, vote_matrix_with_reposts);
    
    auto content_rank = prod(content_matrix, account_rank_final);

    if (parameters.include_detailed_data) {
        calculate_detalization(account_rank_detalization, parameters.outlink_weight, outlink_matrix, account_rank_final, initial_vector, additional_matrices);
    }
    
    last_intermediate_results = current_intermediate_results;
    
    return calculate_score(account_rank_final, content_rank);
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

void social_index_calculator::calculate_content_matrix(
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
}


void social_index_calculator::update_weight_matrix(const std::vector<std::shared_ptr<relation_t> >& relations) {
    for (unsigned int i=0; i<relations.size(); i++) {
        std::shared_ptr<relation_t> t = relations[i];
        
        if (parameters.extended_logging) {
            exporter.export_relation(*t);
        }
        
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
            (*p_vote_matrix)(content_map[t->get_target()], account_map[t->get_source()]) = decay_value;
        }

        if (t->get_name() == "REPOST") {
            (*p_repost_matrix)(content_map[t->get_target()], content_map[t->get_source()]) = 1;
            (*p_repost_matrix)(content_map[t->get_source()], content_map[t->get_source()]) = -1;
        }
        if (t->get_name() == "TRUST") {
            (*p_trust_matrix)(account_map[t->get_target()], account_map[t->get_source()]) = 1;
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
        if (node_it.first != reserved_account) {
            (*account_rank_map)[node_it.first] = account_rank[node_it.second];
        }
    }
    for (auto node_it: content_map) {
        (*content_rank_map)[node_it.first] = content_rank[node_it.second];
    }
    
//     normalization_tools::scale_activity_index_to_1(*account_rank_map);
    
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

void social_index_calculator::set_priorities(const std::map<std::string, double_type>& priorities)
{
    priority_map = priorities;
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
    p_vote_matrix->set_real_size(contents_count, accounts_count);
    p_ownership_matrix->set_real_size(accounts_count, contents_count);
    p_repost_matrix->set_real_size(contents_count, contents_count);
    p_comment_matrix->set_real_size(contents_count, contents_count);
    p_trust_matrix->set_real_size(accounts_count, accounts_count);
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

vector_t social_index_calculator::create_priority_vector()
{
    vector_t result(accounts_count, 0);
    
    for (auto weight_it: priority_map) {
        std::string account_name = weight_it.first;
        double_type weight_value = weight_it.second;
        
        auto account_it = account_map.find(account_name);
        
        if (account_it != account_map.end()) {
            auto account_id = account_it->second;
            
            result(account_id) = weight_value;
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
    activity_index_detalization_t& detalization,
    double_type outlink_weight,
    const matrix_t& outlink_matrix, 
    const vector_t& activity_index_vector,
    const vector_t& weight_vector, 
    const additional_matrices_vector& additional_matrices
) 
{
    std::lock_guard<std::mutex> lock(accounts_lock);
    
    std::vector<std::string> reverse_account_map(account_map.size());
    
    double_type activity_index_vector_norm = norm_1(activity_index_vector);
    
    vector_t base_vector(account_map.size(), 0);
    
    base_vector += (double_type(1) - outlink_weight) * weight_vector * activity_index_vector_norm;
    
    for (auto it: additional_matrices) {
        base_vector += outlink_weight * prod(*it, activity_index_vector);
    }
    
    for (auto it: account_map) {
        std::string name = it.first;
        size_t id = it.second;
        reverse_account_map[id] = name;
        
        detalization.base_index[name] = base_vector[id];
    }
    
    for (auto i = outlink_matrix.cbegin1(); i != outlink_matrix.cend1(); i++) {
        for (auto j = i.cbegin(); j != i.cend(); j++) {
            contribution_t contribution;
            
            contribution.koefficient = outlink_weight * outlink_matrix(j.index1(), j.index2());
            contribution.rate = activity_index_vector(j.index2());
            
            detalization.activity_index_contribution[reverse_account_map[j.index1()]][reverse_account_map[j.index2()]] = contribution;
            
        }
    }
}

void social_index_calculator::collapse_matrix(matrix_t& out, const matrix_t& in1, const matrix_t& in2)
{
    for (matrix_t::const_iterator1 i = in1.begin1(); i != in1.end1(); i++) {
        
        for (size_t j=0; j < in2.size2(); j++) {
            
            double_type x = 0;
            for (matrix_t::const_iterator2 k = i.cbegin(); k != i.cend(); k++) {
                double_type y = (*k) * in2(k.index2(), j);
                if (y > x) {
                    x = y;
                }
            }
            
            if (x > 0 ) {
                out(i.index1(), j) = x;
            }
        }
    }
}

void social_index_calculator::set_diagonal_elements(matrix_t& m)
{
    if (m.size1() != m.size2()) {
        throw runtime_exception("A square matrix is expected");
    }
    
    for (size_t i=0; i<m.size1(); i++ ) {
        m(i,i) += 1;
    }
}

void social_index_calculator::add_phantom_account_relations (matrix_t& m)
{
    auto phantom_account_id = get_account_id(reserved_account, false);
    
    if (phantom_account_id) {
        for (auto i: account_map) {
            auto name = i.first;
            auto id = i.second;
            
            if (id != phantom_account_id) {
                m(*phantom_account_id, id) = 1;
            }
        }
    }
}

account_activity_index_map_t social_index_calculator::vector2map(vector_t& v)
{
    account_activity_index_map_t result;
    
    for (auto node_it: account_map) {
        if (node_it.first != reserved_account) {
            result[node_it.first] = v[node_it.second];
        }
    };

    return result;
}
