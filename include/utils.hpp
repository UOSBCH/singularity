
#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdlib>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/exception/all.hpp>
#include "mapped_matrix_resizable.hpp"
#include <boost/multiprecision/cpp_dec_float.hpp>

namespace singularity {
    typedef boost::multiprecision::number< boost::multiprecision::cpp_dec_float<10> > double_type;
    typedef boost::numeric::ublas::mapped_matrix_resizable<double_type, boost::numeric::ublas::row_major> matrix_t;
    typedef boost::numeric::ublas::vector<double_type> vector_t;
    typedef boost::numeric::ublas::mapped_vector<double_type> sparce_vector_t;
    typedef unsigned long int index_t;
    typedef boost::numeric::ublas::matrix_range<matrix_t> matrix_range_t;
    typedef boost::numeric::ublas::vector_range<vector_t> vector_range_t;
    typedef boost::numeric::ublas::range range_t;
    typedef boost::numeric::ublas::mapped_matrix_resizable<uint8_t, boost::numeric::ublas::row_major> byte_matrix_t;

    struct parameters_t {
        uint account_amount_threshold = 10000;
        uint transaction_amount_threshold = 100;
        double_type outlink_weight = 0.7;
        double_type interlevel_weight = 0.1;
        uint clustering_m = 4;
        double_type clustering_e = 0.3;
        uint decay_period = 86400; // 24h = 86400 1s-blocks
        double_type decay_koefficient = 0.9;
        unsigned int num_threads = 1;
        double_type token_usd_rate = 1;
    };
    
    namespace matrix_tools
    {
        void normalize_columns(matrix_t &m);
        void normalize_rows(matrix_t &m);
        sparce_vector_t calculate_correction_vector(const matrix_t& o);
        std::shared_ptr<matrix_t> resize(matrix_t& m, matrix_t::size_type size1, matrix_t::size_type size2);
        void prod( vector_t& out, const matrix_t& m, const vector_t& v, unsigned int num_threads);
        void partial_prod( vector_t& out, const matrix_t& m, const vector_t& v, range_t range);
        std::vector<range_t> split_range(range_t range, unsigned int max);
    };
    
    class runtime_exception: public std::runtime_error
    {
    public:
        runtime_exception(const std::string& __arg) :
        runtime_error(__arg) {
        }
    };
}

#endif /* UTILS_HPP */

