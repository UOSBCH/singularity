
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
            {};
        virtual ~relation_t(){};
        virtual int64_t get_weight() = 0;
        virtual int64_t get_reverse_weight() = 0;
        virtual std::string get_name() = 0;
        virtual std::string get_source() 
        {
            return source;
        };
        virtual std::string get_target() 
        {
            return target;
        };
        virtual uint64_t get_height() 
        {
            return height;
        };
        virtual bool is_decayable() = 0;
        virtual node_type get_source_type() = 0;
        virtual node_type get_target_type() = 0;
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
        {};
        virtual int64_t get_weight() {
            return weight;
        };
        virtual int64_t get_reverse_weight() {
            return reverse_weight;
        };
        virtual std::string get_name() {
            return name;
        }
        virtual bool is_decayable() {
            return decayable;
        };
        virtual node_type get_source_type() {
            return source_type;
        };
        virtual node_type get_target_type() {
            return target_type;
        };
        
    };
    
    class upvote_t: public relation_t 
    {
    public:
        upvote_t (std::string source, std::string target, uint64_t height):
        relation_t(source, target, height) 
        {};
        virtual int64_t get_weight() {
            return 1;
        };
        virtual int64_t get_reverse_weight() {
            return 0;
        };
        virtual std::string get_name() {
            return "UPVOTE";
        };
        virtual bool is_decayable() {
            return true;
        };
        virtual node_type get_source_type() {
            return node_type::ACCOUNT;
        };
        virtual node_type get_target_type(){
            return node_type::CONTENT;
        };
    };
    
    class downvote_t: public relation_t 
    {
    public:
        downvote_t (std::string source, std::string target, uint64_t height):
        relation_t(source, target, height) 
        {};
        virtual int64_t get_weight() {
            return -1;
        };
        virtual int64_t get_reverse_weight() {
            return 0;
        };
        virtual std::string get_name() {
            return "DOWNVOTE";
        };
        virtual bool is_decayable() {
            return true;
        };
        virtual node_type get_source_type() {
            return node_type::ACCOUNT;
        };
        virtual node_type get_target_type(){
            return node_type::CONTENT;
        };
    };
    
    class follow_t: public relation_t 
    {
    public:
        follow_t (std::string source, std::string target, uint64_t height):
        relation_t(source, target, height) 
        {};
        virtual int64_t get_weight() {
            return 2;
        };
        virtual int64_t get_reverse_weight() {
            return 0;
        };
        virtual std::string get_name() {
            return "FOLLOW";
        };
        virtual bool is_decayable() {
            return false;
        };
        virtual node_type get_source_type() {
            return node_type::ACCOUNT;
        };
        virtual node_type get_target_type(){
            return node_type::ACCOUNT;
        };
    };
    
    class trust_t: public relation_t 
    {
    public:
        trust_t (std::string source, std::string target, uint64_t height):
        relation_t(source, target, height) 
        {};
        virtual int64_t get_weight() {
            return 10;
        };
        virtual int64_t get_reverse_weight() {
            return 0;
        };
        virtual std::string get_name() {
            return "TRUST";
        };
        virtual bool is_decayable() {
            return true;
        };
        virtual node_type get_source_type() {
            return node_type::ACCOUNT;
        };
        virtual node_type get_target_type(){
            return node_type::ACCOUNT;
        };
    };

    class ownwership_t: public relation_t 
    {
    public:
        ownwership_t (std::string source, std::string target, uint64_t height):
        relation_t(source, target, height) 
        {};
        virtual int64_t get_weight() {
            return 1;
        };
        virtual int64_t get_reverse_weight() {
            return 1;
        };
        virtual std::string get_name() {
            return "OWNERSHIP";
        };
        virtual bool is_decayable() {
            return false;
        };
        virtual node_type get_source_type() {
            return node_type::ACCOUNT;
        };
        virtual node_type get_target_type(){
            return node_type::CONTENT;
        };
    };

    class comment_t: public relation_t 
    {
    public:
        comment_t (std::string source, std::string target, uint64_t height):
        relation_t(source, target, height) 
        {};
        virtual int64_t get_weight() {
            return 1;
        };
        virtual int64_t get_reverse_weight() {
            return 0;
        };
        virtual std::string get_name() {
            return "COMMENT";
        };
        virtual bool is_decayable() {
            return true;
        };
        virtual node_type get_source_type() {
            return node_type::CONTENT;
        };
        virtual node_type get_target_type(){
            return node_type::CONTENT;
        };
    };

    class repost_t: public relation_t 
    {
    public:
        repost_t (std::string source, std::string target, uint64_t height):
        relation_t(source, target, height) 
        {};
        virtual int64_t get_weight() {
            return 1;
        };
        virtual int64_t get_reverse_weight() {
            return 0;
        };
        virtual std::string get_name() {
            return "REPOST";
        };
        virtual bool is_decayable() {
            return true;
        };
        virtual node_type get_source_type() {
            return node_type::CONTENT;
        };
        virtual node_type get_target_type(){
            return node_type::CONTENT;
        };
    };
    
    class membership_t: public relation_t 
    {
    public:
        membership_t (std::string source, std::string target, uint64_t height):
        relation_t(source, target, height) 
        {};
        virtual int64_t get_weight() {
            return 10;
        };
        virtual int64_t get_reverse_weight() {
            return 10;
        };
        virtual std::string get_name() {
            return "MEMBERSHIP";
        };
        virtual bool is_decayable() {
            return true;
        };
        virtual node_type get_source_type() {
            return node_type::ACCOUNT;
        };
        virtual node_type get_target_type(){
            return node_type::ORGANIZATION;
        };
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
        { };
        virtual int64_t get_weight() {
            return (int64_t) amount;
        };
        virtual int64_t get_reverse_weight() {
            return - (int64_t) amount;
        };
        virtual std::string get_name() {
            return "TRANSFER";
        };
        money_t get_amount() 
        {
            return amount;
        };
        money_t get_source_account_balance() 
        {
            return source_account_balance;
        };
        money_t get_target_account_balance() 
        {
            return target_account_balance;
        };
        virtual bool is_decayable() {
            return true;
        };
        virtual node_type get_source_type() {
            return node_type::ACCOUNT;
        };
        virtual node_type get_target_type(){
            return node_type::ACCOUNT;
        };
    };

}

#endif /* RELATIONS_HPP */
