#ifndef SINGULARUTY_EXPORTER_HPP
#define SINGULARUTY_EXPORTER_HPP

#include <iostream>
#include "relations.hpp"

namespace singularity {

    class exporter_t
    {
    public:
        exporter_t(std::ostream& output): output(output) {};
        
        void export_relation(relation_t& relation)
        {
            output 
                << ";" 
                << ";" 
                << relation.get_source() 
                << ";" 
                << relation.get_target() 
                << ";" 
                << relation.get_name() 
                << ";" << 
                relation.get_height()  
                << ";" <<  
                relation.get_weight()
                << ";" <<  
                relation.get_reverse_weight()
                << ";" <<  
                get_node_type_name (relation.get_source_type())
                << ";" <<  
                get_node_type_name (relation.get_target_type())
                << std::endl;
        }
        
        std::string get_node_type_name (node_type type)
        {
            std::string name;
            
            switch (type) {
                case node_type::ACCOUNT:
                    name = "ACCOUNT";
                    break;
                    
                case node_type::CONTENT:
                    name = "CONTENT";
                    break;
                    
                default:
                    name = "UNKNOWN";
            }
            
            return name;
        }
    private:
         std::ostream& output;
    };
}

#endif
