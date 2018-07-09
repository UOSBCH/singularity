#include "include/emission.hpp"
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <fstream>

using namespace singularity;

activity_period::activity_period()
{
    p_weight_matrix = std::make_shared<matrix_t>(initial_size, initial_size);
}

void activity_period::collect_accounts(
    account_id_map_t& account_id_map,
    const std::vector<transaction_t>& transactions
) {
    std::lock_guard<std::mutex> lock(accounts_lock);
    unsigned int account_id = account_id_map.size();
    for (unsigned int i=0; i<transactions.size(); i++) {
        transaction_t transaction = transactions[i];
        account_id_map_t::iterator found_source = account_id_map.find(transaction.source_account);
        account_id_map_t::iterator found_target = account_id_map.find(transaction.target_account);
        if (found_source == account_id_map.end()) {
            account_id_map.insert(account_id_map_t::value_type(transaction.source_account, account_id++));
        }
        if (found_target == account_id_map.end()) {
            account_id_map.insert(account_id_map_t::value_type(transaction.target_account, account_id++));
        }
    }
}

void activity_period::add_block(const std::vector<transaction_t>& transactions)
{
    std::lock_guard<std::mutex> lock(weight_matrix_lock);
    
    handled_blocks_count ++;
    
    collect_accounts(account_map, transactions);
    
    if (p_weight_matrix->size1() < account_map.size()) {
        matrix_t::size_type new_size = p_weight_matrix->size1();
        while (new_size < account_map.size()) {
            new_size *= 2;
        }
        p_weight_matrix->resize(new_size, new_size);
    }

    update_weight_matrix(*p_weight_matrix, account_map, transactions);
}

byte_matrix_t activity_period::calculate_link_matrix(
    matrix_t::size_type size,
    matrix_t& weight_matrix
)
{
    matrix_t l(size, size);
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
                l(j.index1(), j.index2()) -= *j;
                l(j.index2(), j.index1()) += *j;
            }
        }
    }
    
    byte_matrix_t result(size, size);
    
    for (matrix_t::iterator1 i = l.begin1(); i != l.end1(); i++)
    {
        for (matrix_t::iterator2 j = i.begin(); j != i.end(); j++)
        {
            if (*j > 0) {
                result(j.index1(), j.index2()) = 1;
            }
        }
    }
    
    return result;
}

void activity_period::update_weight_matrix(matrix_t& weight_matrix, account_id_map_t& account_id_map, const std::vector<transaction_t>& transactions) {
    for (unsigned int i=0; i<transactions.size(); i++) {
        transaction_t t = transactions[i];
        weight_matrix(account_id_map[t.source_account], account_id_map[t.target_account]) += t.amount;
    }
}

double_type activity_period::get_activity()
{
    byte_matrix_t n = calculate_link_matrix(account_map.size(), *p_weight_matrix);
    
    return double_type(n.nnz());
}

money_t emission_calculator::calculate(money_t total_emission, activity_period& period)
{
    emission_parameters_t current_parameters = parameters;
    
    money_t current_supply = current_parameters.initial_supply + total_emission;

    double_type emission_limit = boost::multiprecision::pow((1 + double_type(current_parameters.year_emission_limit) / 100), 1.0 / current_parameters.emission_event_count_per_year) - 1;
    
    double_type new_activity = period.get_activity();
    
    if (new_activity > emission_state.last_activity) {
        emission_state.target_emission += (money_t) (current_parameters.emission_scale * (new_activity - emission_state.last_activity));
        
        emission_state.last_activity = new_activity;
        double_type argument = (double_type(emission_state.target_emission - total_emission)) / current_supply;
        
        return (money_t) (current_supply * emission_limit * tanh(current_parameters.delay_koefficient * argument / emission_limit));
    } else {
        
        return 0;
    }
}

singularity::emission_state_t singularity::emission_calculator::get_emission_state()
{
    return emission_state;
}

void activity_period::clear()
{
    std::lock_guard<std::mutex> lock(weight_matrix_lock);
    
    handled_blocks_count = 0;
    
    p_weight_matrix->clear();
}

singularity::emission_parameters_t singularity::emission_calculator::get_parameters()
{
    return parameters;
}

void singularity::emission_calculator::set_parameters(singularity::emission_parameters_t emission_parameters)
{
    this->parameters = emission_parameters;
}

void activity_period::save_state_to_file(std::string filename) 
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

void activity_period::load_state_from_file(std::string filename) 
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

unsigned int singularity::activity_period::get_handled_block_count()
{
    return handled_blocks_count;
}
