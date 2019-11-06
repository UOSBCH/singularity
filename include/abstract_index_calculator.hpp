#ifndef ABSTRACT_INDEX_CALCULATOR_HPP
#define ABSTRACT_INDEX_CALCULATOR_HPP

#include "relations.hpp"

namespace singularity {

  class abstract_index_calculator
  {
  public:
      virtual void add_block(const std::vector<std::shared_ptr<relation_t> >& transactions) = 0;
      virtual void add_relation(const relation_t& relation) = 0;
      virtual std::map<node_type, std::shared_ptr<account_activity_index_map_t> > calculate() = 0;
  };
}
#endif // ABSTRACT_INDEX_CALCULATOR_HPP
