
#ifndef FILTERS_HPP
#define FILTERS_HPP

#include "relations.hpp"
#include "utils.hpp"

namespace singularity {
    
    class filter_interface
    {
    public:
        virtual bool check(const relation_t& relation) const = 0;
        virtual ~filter_interface() {}
    };

    class transfer_filter: public filter_interface
    {
    public:
        transfer_filter(parameters_t parameters): parameters_(parameters) {}
        
        virtual bool check(const relation_t& relation) const override
        {
            try {
                auto transaction = dynamic_cast<const transaction_t&>(relation);

                if (transaction.get_amount() < parameters_.token_usd_rate * parameters_.transaction_amount_threshold * parameters_.precision) {
                    return false;
                }

                if (transaction.get_source_account_balance() < parameters_.token_usd_rate * parameters_.account_amount_threshold * parameters_.precision) {
                    return false;
                }

                if (transaction.get_target_account_balance() < parameters_.token_usd_rate * parameters_.account_amount_threshold * parameters_.precision) {
                    return false;
                }

                return true;
            }  catch (std::bad_cast& e) {
                return false;
            }
        }
    private:
        parameters_t parameters_;
    };
}


#endif /* FILTERS_HPP */
