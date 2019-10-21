#ifndef ABSTRACT_INDEX_CALCULATOR_HPP
#define ABSTRACT_INDEX_CALCULATOR_HPP

#include "relations.hpp"

namespace singularity {

  class abstract_index_calculator
  {
  public:
      virtual void add_block(const std::vector<std::shared_ptr<relation_t> >& transactions) = 0;
  };
}
#endif // ABSTRACT_INDEX_CALCULATOR_HPP
