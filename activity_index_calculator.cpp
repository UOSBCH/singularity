#include "include/activity_index_calculator.hpp"
#include "include/ncd_aware_rank.hpp"
#include <ctime>
#include <boost/numeric/ublas/io.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <fstream>
#include <thread>

using namespace boost::numeric::ublas;
using namespace boost;
using namespace singularity;

activity_index_calculator::activity_index_calculator(parameters_t parameters) {
    this->parameters = parameters;
    p_weight_matrix = std::make_shared<matrix_t>(initial_size, initial_size);
}

void activity_index_calculator::collect_accounts(
    account_id_map_t& account_id_map,
    const std::vector<transaction_t>& transactions
) {
    std::lock_guard<std::mutex> lock(accounts_lock);
    for(auto transaction: transactions){
        account_id_map.insert({transaction.source_account, account_id_map.size()});
        account_id_map.insert({transaction.target_account, account_id_map.size()});
    }
}

void activity_index_calculator::add_block(const std::vector<transaction_t>& transactions) {
    std::vector<transaction_t> filtered_transactions = filter_block(transactions);
    std::lock_guard<std::mutex> lock(weight_matrix_lock);
    collect_accounts(account_map, filtered_transactions);
    
    total_handled_blocks_count++;
    handled_blocks_count++;
    if (handled_blocks_count >= parameters.decay_period) {
        handled_blocks_count -= parameters.decay_period;
        *p_weight_matrix *= parameters.decay_koefficient;
    }
    
    if (p_weight_matrix->size1() < account_map.size()) {
        matrix_t::size_type new_size = p_weight_matrix->size1();
        while (new_size < account_map.size()) {
            new_size *= 2;
        }
        p_weight_matrix->resize(new_size, new_size);
    }

    update_weight_matrix(*p_weight_matrix, account_map, filtered_transactions);
}

std::vector<transaction_t> singularity::activity_index_calculator::filter_block(const std::vector<transaction_t>& block)
{
    std::vector<transaction_t> filtered_block;
    
    for (auto transaction: block) {
        if (check_transaction(transaction)) {
            filtered_block.push_back(transaction);
        }
    }
    
    return filtered_block;
}

void activity_index_calculator::skip_blocks(unsigned int blocks_count)
{
    std::lock_guard<std::mutex> lock(weight_matrix_lock);
    total_handled_blocks_count += blocks_count;
    handled_blocks_count += blocks_count;
    
    if (handled_blocks_count >= parameters.decay_period) {
        unsigned int decay_period_count = handled_blocks_count / parameters.decay_period;
        handled_blocks_count = handled_blocks_count - decay_period_count * parameters.decay_period;
        double_type decay_value = boost::multiprecision::pow(parameters.decay_koefficient, decay_period_count);
        *p_weight_matrix *= decay_value;
    }
}

account_activity_index_map_t activity_index_calculator::calculate()
{
    if (account_map.size() == 0) {
        return account_activity_index_map_t();
    }
    ncd_aware_rank nar(parameters);
    matrix_t outlink_matrix(account_map.size(), account_map.size());

    calculate_outlink_matrix(outlink_matrix, *p_weight_matrix);
    
    
    std::shared_ptr<vector_t> rank = nar.process(outlink_matrix);
    
    return calculate_score(account_map, *rank);
}

bool activity_index_calculator::check_account( account_t account ) 
{
    if (account.amount < parameters.token_usd_rate * parameters.account_amount_threshold * parameters.precision) {
        return false;
    }
    
    return true;
}

bool activity_index_calculator::check_transaction( transaction_t transaction) 
{
    if (transaction.amount < parameters.token_usd_rate * parameters.transaction_amount_threshold * parameters.precision) {
        return false;
    }

    if (transaction.source_account_balance < parameters.token_usd_rate * parameters.account_amount_threshold * parameters.precision) {
        return false;
    }

    if (transaction.target_account_balance < parameters.token_usd_rate * parameters.account_amount_threshold * parameters.precision) {
        return false;
    }
    
    return true;
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
                o(j.index2(), j.index1()) += *j;
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

void activity_index_calculator::update_weight_matrix(matrix_t& weight_matrix, account_id_map_t& account_id_map, const std::vector<transaction_t>& transactions) {
    for (auto t: transactions) {
        weight_matrix(account_id_map[t.source_account], account_id_map[t.target_account]) += t.amount;
    }
}

account_activity_index_map_t activity_index_calculator::calculate_score(
        const account_id_map_t& account_id_map,
        const vector_t& rank
)
{
    account_activity_index_map_t v;
    
    for (auto i: account_id_map) {
        v[i.first] = rank[i.second];
    }

    return v;
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


