
#ifndef RELATIONS_HPP
#define RELATIONS_HPP

#include "utils.hpp"

namespace singularity {
    
    enum node_type {ACCOUNT, CONTENT, ORGANIZATION};

    template <class T>
        using node_type_map = std::map<node_type, std::shared_ptr<T> >;
    
    class relation_t {
    private:
        std::string source;
        std::string target;
        uint64_t height;
    public:
        relation_t(std::string source, std::string target, uint64_t height):
            source(source),
            target(target),
            height(height)
            {}
        virtual ~relation_t(){}
        virtual int64_t get_weight() const = 0;

        /**
         * @deprecated
         */
        virtual int64_t get_reverse_weight() const = 0;
        virtual std::string get_name() const = 0;
        virtual std::string get_source() const
        {
            return source;
        }
        virtual std::string get_target() const
        {
            return target;
        }
        virtual uint64_t get_height() const
        {
            return height;
        }
        virtual bool is_decayable() const = 0;
        virtual node_type get_source_type() const = 0;
        virtual node_type get_target_type() const = 0;
    };
    
    class custom_relation_t: public relation_t
    {
    private:
        std::string name;
        node_type source_type;
        node_type target_type;
        int64_t weight;
        int64_t reverse_weight;
        bool decayable;
    public:
        custom_relation_t(
            std::string name,
            std::string source, 
            std::string target, 
            node_type source_type, 
            node_type target_type, 
            uint64_t height, 
            int64_t weight,
            int64_t reverse_weight,
            bool decayable
        ): relation_t(source, target, height), name(name), source_type(source_type), target_type(target_type), weight(weight), reverse_weight(reverse_weight), decayable(decayable) 
        {}
        virtual int64_t get_weight() const override {
            return weight;
        }
        virtual int64_t get_reverse_weight() const override {
            return reverse_weight;
        }
        virtual std::string get_name() const override {
            return name;
        }
        virtual bool is_decayable() const override {
            return decayable;
        }
        virtual node_type get_source_type() const override {
            return source_type;
        }
        virtual node_type get_target_type() const override {
            return target_type;
        }
        
    };
    
    class upvote_t: public relation_t 
    {
    public:
        const static std::string NAME;
        upvote_t (std::string source, std::string target, uint64_t height):
        relation_t(source, target, height) 
        {}
        virtual int64_t get_weight() const override {
            return 1;
        }
        virtual int64_t get_reverse_weight() const override {
            return 0;
        }
        virtual std::string get_name() const override {
            return upvote_t::NAME;
        }
        virtual bool is_decayable() const override {
            return true;
        }
        virtual node_type get_source_type() const override {
            return node_type::ACCOUNT;
        }
        virtual node_type get_target_type() const override {
            return node_type::CONTENT;
        }
    };
    
    class downvote_t: public relation_t 
    {
    public:
        const static std::string NAME;
        downvote_t (std::string source, std::string target, uint64_t height):
        relation_t(source, target, height) 
        {}
        virtual int64_t get_weight() const override {
            return -1;
        }
        virtual int64_t get_reverse_weight() const override {
            return 0;
        }
        virtual std::string get_name() const override {
            return downvote_t::NAME;
        }
        virtual bool is_decayable() const override {
            return true;
        }
        virtual node_type get_source_type() const override {
            return node_type::ACCOUNT;
        }
        virtual node_type get_target_type() const override {
            return node_type::CONTENT;
        }
    };
    
    class follow_t: public relation_t 
    {
    public:
        const static std::string NAME;
        follow_t (std::string source, std::string target, uint64_t height):
        relation_t(source, target, height) 
        {}
        virtual int64_t get_weight() const override {
            return 2;
        }
        virtual int64_t get_reverse_weight() const override {
            return 0;
        }
        virtual std::string get_name() const override {
            return follow_t::NAME;
        }
        virtual bool is_decayable() const override {
            return false;
        }
        virtual node_type get_source_type() const override {
            return node_type::ACCOUNT;
        }
        virtual node_type get_target_type() const override {
            return node_type::ACCOUNT;
        }
    };
    
    class trust_t: public relation_t 
    {
    public:
        const static std::string NAME;
        trust_t (std::string source, std::string target, uint64_t height):
        relation_t(source, target, height) 
        {}
        virtual int64_t get_weight() const override {
            return 10;
        }
        virtual int64_t get_reverse_weight() const override {
            return 0;
        }
        virtual std::string get_name() const override {
            return trust_t::NAME;
        }
        virtual bool is_decayable() const override {
            return true;
        }
        virtual node_type get_source_type() const override {
            return node_type::ACCOUNT;
        }
        virtual node_type get_target_type() const override {
            return node_type::ACCOUNT;
        }
    };

    class ownership_t: public relation_t
    {
    public:
        const static std::string NAME;
        ownership_t (std::string source, std::string target, uint64_t height):
        relation_t(source, target, height) 
        {}
        virtual int64_t get_weight() const override {
            return 1;
        }
        virtual int64_t get_reverse_weight() const override {
            return 1;
        }
        virtual std::string get_name() const override {
            return ownership_t::NAME;
        }
        virtual bool is_decayable() const override {
            return false;
        }
        virtual node_type get_source_type() const override {
            return node_type::ACCOUNT;
        }
        virtual node_type get_target_type() const override {
            return node_type::CONTENT;
        }
    };

    class comment_t: public relation_t 
    {
    public:
        const static std::string NAME;
        comment_t (std::string source, std::string target, uint64_t height):
        relation_t(source, target, height) 
        {}
        virtual int64_t get_weight() const override {
            return 1;
        }
        virtual int64_t get_reverse_weight() const override {
            return 0;
        }
        virtual std::string get_name() const override {
            return comment_t::NAME;
        }
        virtual bool is_decayable() const override {
            return true;
        }
        virtual node_type get_source_type() const override {
            return node_type::CONTENT;
        }
        virtual node_type get_target_type() const override {
            return node_type::CONTENT;
        }
    };

    class repost_t: public relation_t 
    {
    public:
        const static std::string NAME;
        repost_t (std::string source, std::string target, uint64_t height):
        relation_t(source, target, height) 
        {}
        virtual int64_t get_weight() const override {
            return 1;
        }
        virtual int64_t get_reverse_weight() const override {
            return 0;
        }
        virtual std::string get_name() const override {
            return repost_t::NAME;
        }
        virtual bool is_decayable() const override {
            return true;
        }
        virtual node_type get_source_type() const override {
            return node_type::CONTENT;
        }
        virtual node_type get_target_type() const override {
            return node_type::CONTENT;
        }
    };
    
    class membership_t: public relation_t 
    {
    public:
        const static std::string NAME;
        membership_t (std::string source, std::string target, uint64_t height):
        relation_t(source, target, height) 
        {}
        virtual int64_t get_weight() const override {
            return 10;
        }
        virtual int64_t get_reverse_weight() const override {
            return 10;
        }
        virtual std::string get_name() const override {
            return membership_t::NAME;
        }
        virtual bool is_decayable() const override {
            return true;
        }
        virtual node_type get_source_type() const override {
            return node_type::ACCOUNT;
        }
        virtual node_type get_target_type() const override {
            return node_type::ORGANIZATION;
        }
    };
    
    class transaction_t: public relation_t 
    {
    private:
        money_t amount;
        money_t comission;
        money_t source_account_balance;
        money_t target_account_balance;
        time_t timestamp;
    public:
        const static std::string NAME;
        transaction_t (
            money_t amount, 
            money_t comission, 
            std::string source, 
            std::string target, 
            time_t timestamp, 
            money_t source_account_balance,
            money_t target_account_balance,
            uint64_t height
        ) :
        relation_t(source, target, height),
        amount(amount), 
        comission(comission), 
        source_account_balance(source_account_balance),
        target_account_balance(target_account_balance),
        timestamp(timestamp)
        { }
        virtual int64_t get_weight() const override {
            return int64_t(amount);
        }
        virtual int64_t get_reverse_weight() const override {
            return - int64_t(amount);
        }
        virtual std::string get_name() const override {
            return transaction_t::NAME;
        }
        money_t get_amount() 
        {
            return amount;
        }
        money_t get_source_account_balance() const
        {
            return source_account_balance;
        }
        money_t get_target_account_balance()  const
        {
            return target_account_balance;
        }
        virtual bool is_decayable() const override {
            return true;
        }
        virtual node_type get_source_type() const override {
            return node_type::ACCOUNT;
        }
        virtual node_type get_target_type() const override {
            return node_type::ACCOUNT;
        }
        time_t get_timestamp() const
        {
            return timestamp;
        }
        money_t get_comission() const
        {
            return comission;
        }
    };

    typedef std::shared_ptr<relation_t> relation_ptr;
}

#endif /* RELATIONS_HPP */
