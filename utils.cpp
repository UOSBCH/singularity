#include <thread>
#include "include/utils.hpp"

using namespace boost::numeric::ublas;
using namespace boost;
using namespace singularity;

void matrix_tools::normalize_columns(matrix_t &m)
{
    mapped_vector<double_type> a (m.size2());
    
    for (matrix_t::iterator1 i = m.begin1(); i != m.end1(); i++)
    {
        for (matrix_t::iterator2 j = i.begin(); j != i.end(); j++)
        {
            if (*j != double_type (0) ) {
                a[j.index2()] += *j;
            }
        }
    }
    for (matrix_t::iterator1 i = m.begin1(); i != m.end1(); i++)
    {
        for (matrix_t::iterator2 j = i.begin(); j != i.end(); j++)
        {
            double_type norm = a[j.index2()];
            if (norm != 0) {
                *j /= norm;
            }
        }
    }
}

void matrix_tools::normalize_rows(matrix_t &m)
{
    for (matrix_t::iterator1 i = m.begin1(); i != m.end1(); i++)
    {
        double_type norm = 0;
        for (matrix_t::iterator2 j = i.begin(); j != i.end(); j++)
        {
            norm += *j;
        }
        if (norm > 0) {
            for (matrix_t::iterator2 j = i.begin(); j != i.end(); j++)
            {
                *j /= norm;
            }
        }
    }
}

sparce_vector_t matrix_tools::calculate_correction_vector(const matrix_t& o) {
    
    sparce_vector_t v(o.size2()), a(o.size2());
    
    double_type correction_value = 1.0/o.size2();
    
    for (matrix_t::const_iterator1 j = o.begin1(); j != o.end1(); j++)
    {
        for (matrix_t::const_iterator2 i = j.begin(); i != j.end(); i++)
        {
            if (*i != 0) {
                a[i.index2()] += *i;
            }
        }
    }
    
    for (unsigned int i=0; i< a.size();i++) {
        if (a[i] == 0) {
            v(i) = correction_value;
        }
    }
   
    return v;
}

std::shared_ptr<matrix_t> matrix_tools::resize(matrix_t& m, matrix_t::size_type size1, matrix_t::size_type size2) {
    std::shared_ptr<matrix_t> m2(new matrix_t(size1, size2));
    
    if (size1 > m.size1() && size2 > m.size2()) {
        for (matrix_t::iterator1 i = m.begin1(); i != m.end1(); i++) {
            for (matrix_t::iterator2 j = i.begin(); j != i.end(); j++) {
                (*m2)(j.index1(), j.index2()) = *j;
            }
        }
    } else if (size1 < m.size1() && size2 < m.size2()) {
        range_t r1(0, m2->size1()), r2(0, m2->size2());
        matrix_range_t mr(m, r1, r2);
        *m2 = mr;
    } else if (size1 == m.size1() && size2 == m.size2()) {
        *m2 = m;
    } else {
        throw runtime_exception("Wrong sizes");
    }
    
    return m2;
}

void matrix_tools::prod( vector_t& out, const matrix_t& m, const vector_t& v, unsigned int num_threads) {
    std::vector<std::thread> threads;
    
    std::vector<range_t> ranges = split_range(range_t(0, m.size1()), num_threads);
    
    
    for (unsigned int i=0; i<ranges.size(); i++) {
        threads.push_back(std::thread(partial_prod, std::ref(out), std::ref(m), std::ref(v), ranges[i]));
    }

    for (unsigned int i=0; i<threads.size(); i++) {
        threads[i].join();
    }
}


void matrix_tools::partial_prod( vector_t& out, const matrix_t& m, const vector_t& v, range_t range)
{
    for (matrix_t::const_iterator1 i = m.begin1(); i != m.end1(); i++) {
        if (i.index1() >= range.start() && i.index1() < range.start() + range.size()) {
            double_type x = 0;
            for (matrix_t::const_iterator2 j = i.begin(); j != i.end(); j++) {
                x += (*j) * v(j.index2());
            }
            
            out[i.index1()] = x;
        }
    }
}

std::vector<range_t> matrix_tools::split_range(range_t range, unsigned int max)
{
    std::vector<range_t> result;
    range_t::size_type total_count = range.size();
    range_t::size_type rest = total_count;
    range_t::size_type partial_count;
    if (total_count <= max) {
        partial_count = 1;
    } else {
        if (total_count % max == 0) {
            partial_count = total_count / max;
        } else {
            partial_count = total_count / max + 1;
        }
    }
    range_t::size_type i=0;
    while(rest > 0) {
        range_t::size_type real_count = std::min(partial_count, rest);
        result.push_back(range_t(i, i+real_count));
        rest -= real_count;
        i += real_count;
    }
    
    return result;
}

account_activity_index_map_t normalization_tools::scale_activity_index(const account_activity_index_map_t& index_map, double_type new_norm)
{
    account_activity_index_map_t result;
    
    double_type old_norm = 0;

    for (auto index: index_map) {
        old_norm += index.second;
    }
    
    if (old_norm == 0) {
        return result;
    }
    
    double_type scale = new_norm / old_norm;
    
    for (auto index: index_map) {
        result[index.first] = index.second * scale;
    }
    
    return result;
}

account_activity_index_map_t normalization_tools::scale_activity_index_to_node_count(const account_activity_index_map_t& index_map)
{
    auto objects_count = double_type(index_map.size());
    
    if (objects_count == 0) {
        return account_activity_index_map_t();
    } else {
        return scale_activity_index(index_map, objects_count);
    }
}

account_activity_index_map_t normalization_tools::scale_activity_index_to_1(const account_activity_index_map_t& index_map)
{
    return scale_activity_index(index_map, 1);
}
