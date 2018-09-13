
#ifndef VECTOR_BASED_MATRIX_HPP
#define VECTOR_BASED_MATRIX_HPP

#include <boost/numeric/ublas/expression_types.hpp>

namespace boost  { namespace numeric { namespace ublas {

template<class T>
class vector_based_matrix: 
    public matrix_container<vector_based_matrix<T> >
    {
    public:
        
        typedef vector_based_matrix<T> self_type;
        typedef std::size_t size_type;
        typedef T value_type;
        typedef mapped_vector<T> basic_vector_type;
        
        // Construction and destruction
        BOOST_UBLAS_INLINE
        vector_based_matrix (const mapped_vector<T>& left_generator, const mapped_vector<T>& right_generator):
            left_generator(left_generator), right_generator(right_generator)
        {};
        
        // Element access
        BOOST_UBLAS_INLINE
        const T operator () (size_type i, size_type j) const {
            
            return left_generator(i) * right_generator(j);
        }

        BOOST_UBLAS_INLINE
        size_type size1 () const {
            
            return left_generator.size();
        }
        BOOST_UBLAS_INLINE
        size_type size2 () const {
            
            return right_generator.size();
        }
        
        template<class AT>
        BOOST_UBLAS_INLINE
        self_type& operator *= (const AT &at) {
            right_generator *= at;
            
            return *this;
        }
        
        template<class AT>
        BOOST_UBLAS_INLINE
        self_type& operator -= (const AT &at) {
            right_generator -= at;
            
            return *this;
        }
        
        const basic_vector_type get_left_generator() const {
            
            return left_generator;
        }

        const basic_vector_type get_right_generator() const {
            
            return right_generator;
        }
        
    private:
        mapped_vector<T> left_generator;
        mapped_vector<T> right_generator;
    };
    
   
    template<typename T, typename E2>
    BOOST_UBLAS_INLINE
    typename vector_based_matrix<T>::basic_vector_type prod (const vector_based_matrix<T> &e1,  const vector_expression<E2> &e2) 
    {
        typename vector_based_matrix<T>::basic_vector_type left = e1.get_left_generator();
        typename vector_based_matrix<T>::basic_vector_type right = e1.get_right_generator();
    
        left *= inner_prod(right, e2);
        
        return left;
    };
    
}}}

#endif /* VECTOR_BASED_MATRIX_HPP */
