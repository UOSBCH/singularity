#include "include/emission.hpp"
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <fstream>

using namespace singularity;

double_type emission_calculator::get_emission_limit(
    double_type current_total_supply
) const
{
    double_type emission_event_count_per_year = double_type(31536000) / parameters_.emission_period_seconds;
    
    return current_total_supply * (boost::multiprecision::pow((1 + parameters_.yearly_emission_percent / 100), 1.0 / emission_event_count_per_year) - 1);
}

double_type emission_calculator::get_target_emission(
    double_type current_activity,
    double_type max_activity
) const
{
    if (current_activity > max_activity) {
        return parameters_.activity_monetary_value * (current_activity - max_activity);
    } else {
        return 0;
    }
}

double_type emission_calculator::get_resulting_emission(
    double_type target_emission,
    double_type emission_limit
) const
{
    if (target_emission > 0) {
        return emission_limit * tanh(parameters_.delay_koefficient * target_emission / emission_limit);
    } else {
        return 0;
    }
}

double_type emission_calculator::get_next_max_activity(
    double_type max_activity,
    double_type resulting_emission
) const
{
    return max_activity + resulting_emission / parameters_.activity_monetary_value;
}

unsigned int activity_period_new::activity_period_new::get_handled_block_count() const
{
    return handled_blocks_count;
}

void activity_period_new::add_block(
    const std::vector<std::shared_ptr<relation_t> >& relations
)
{
    handled_blocks_count++;
    for(auto relation: relations) {
        auto transaction = std::dynamic_pointer_cast<transaction_t>(relation);
        if(transaction) {
            uint64_t period_index = transaction->get_height() / period_length;
            if (period_index < period_count) {
                (*p_account_keepers)[period_index].get_account_id(transaction->get_source(), true);
                (*p_account_keepers)[period_index].get_account_id(transaction->get_target(), true);
            }
        }
    }
}

double_type activity_period_new::get_activity() const
{
    double_type result;
    
    for(uint32_t period_index=0; period_index < period_count; period_index++) {
        result += (*p_account_keepers)[period_index].get_account_count();
    }
    
    result /= period_count;
    
    return result;
}
