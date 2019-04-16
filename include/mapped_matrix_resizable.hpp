
#ifndef MAPPED_MATRIX_RESIZABLE_HPP
#define MAPPED_MATRIX_RESIZABLE_HPP

#include <boost/numeric/ublas/matrix_sparse.hpp>

namespace boost  { namespace numeric { namespace ublas {

template<class T, class L = row_major, class A = std::map<std::size_t, T> >
class mapped_matrix_resizable: 
    public mapped_matrix<T, L, A>
    {
    public:
        typedef L layout_type;
        typedef mapped_matrix<T, L, A> parent_type;
        
        // Construction and destruction
        BOOST_UBLAS_INLINE
        mapped_matrix_resizable ():
            mapped_matrix<T, L, A> () {}
        BOOST_UBLAS_INLINE
        mapped_matrix_resizable (typename A::size_type size1, typename A::size_type size2, typename A::size_type non_zeros = 0):
            mapped_matrix<T, L, A> (size1, size2, non_zeros) {}
        BOOST_UBLAS_INLINE
        mapped_matrix_resizable (const mapped_matrix_resizable &m):
            mapped_matrix<T, L, A> (m) {}
            
        template<class AE>
        BOOST_UBLAS_INLINE
        mapped_matrix_resizable (const matrix_expression<AE> &ae, typename A::size_type non_zeros = 0):
            mapped_matrix<T, L, A> (ae, non_zeros) {}
            
        void resize (typename A::size_type size1, typename A::size_type size2, bool preserve = true) {
            if (preserve) {
                typename A::size_type old_size1 = parent_type::size1();
                typename A::size_type old_size2 = parent_type::size2();
                A old_data = parent_type::data();
                parent_type::resize(size1, size2, false);
                for (auto it=old_data.cbegin(); it != old_data.cend(); it++) {
                    typename A::size_type i = L::index_i(it->first, old_size1, old_size2);
                    typename A::size_type j = L::index_j(it->first, old_size1, old_size2);
                    (*this)(i, j) = it->second;
                }
            } else {
                parent_type::resize(size1, size2, false);
            }
        }
        
        template<class AT>
        BOOST_UBLAS_INLINE
        mapped_matrix_resizable& operator += (const AT &at) {
            A& data = parent_type::data();
            for (auto it=data.begin(); it != data.end(); it++) {
                it->second += at;
            }
            return *this;
        }
        
        template<class AT>
        BOOST_UBLAS_INLINE
        mapped_matrix_resizable& operator -= (const AT &at) {
            A& data = parent_type::data();
            for (auto it=data.begin(); it != data.end(); it++) {
                it->second -= at;
            }
            return *this;
        }
        
        template<class AT>
        BOOST_UBLAS_INLINE
        mapped_matrix_resizable& operator *= (const AT &at) {
            A& data = parent_type::data();
            for (auto it=data.begin(); it != data.end(); it++) {
                it->second *= at;
            }
            return *this;
        }
        
        template<class AT>
        BOOST_UBLAS_INLINE
        mapped_matrix_resizable& operator /= (const AT &at) {
            A& data = parent_type::data();
            for (auto it=data.begin(); it != data.end(); it++) {
                it->second /= at;
            }
            return *this;
        }
        
        typename A::size_type get_real_size_1() {
            return real_size_1_;
        }

        typename A::size_type get_real_size_2() {
            return real_size_2_;
        }
        
        void set_real_size(typename A::size_type real_size_1, typename A::size_type real_size_2) {
            if (real_size_1 > parent_type::size1() || real_size_2 > parent_type::size2()) {
                typename A::size_type new_size_1 = parent_type::size1();
                typename A::size_type new_size_2 = parent_type::size2();
                while (new_size_1 < real_size_1) {
                    new_size_1 *= 2;
                }
                while (new_size_2 < real_size_2) {
                    new_size_2 *= 2;
                }
                
                this->resize(new_size_1, new_size_2, true);
            };
            
            real_size_1_ = real_size_1;
            real_size_2_ = real_size_2;
        }
        
    private:
        typename A::size_type real_size_1_ = 0;
        typename A::size_type real_size_2_ = 0;
    };
    
    template<class T, class L = row_major, class A = std::map<std::size_t, T> >
    vector_expression<T> prod(const mapped_matrix_resizable<T, L, A> &e1, const vector_expression<T> &e2)
    {
        vector<T> result(e1.size1());
        
        for (auto it: e1.data()) {
            auto i = L::index_i(it.first, e1.size1(), e1.size2());
            auto j = L::index_j(it.first, e1.size1(), e1.size2());
            result(i) += it.second * e2(j);
        }
        
        return result;
    }
}}}

#endif /* MAPPED_MATRIX_RESIZABLE_HPP */
