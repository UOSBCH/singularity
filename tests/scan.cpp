#define BOOST_TEST_MODULE SCAN
#include <boost/test/included/unit_test.hpp>
#include "../include/scan.hpp"
#include <stdlib.h>
#include <iostream>
#include <boost/numeric/ublas/io.hpp>
#include <boost/graph/graphviz.hpp>

using namespace singularity;
using namespace boost;

BOOST_AUTO_TEST_SUITE( scan_test )

BOOST_AUTO_TEST_CASE( test1 )
{
    Graph g(12);
    scan scan(0.2, 4);
    
    Graph::vertex_iterator v, v0;
    
    tie(v, v0) = vertices(g);

    add_edge(v[0],v[2],g);
    add_edge(v[1],v[2],g);
    add_edge(v[2],v[3],g);
    add_edge(v[2],v[4],g);
    add_edge(v[3],v[4],g);

    add_edge(v[5],v[6],g);
    add_edge(v[5],v[7],g);
    add_edge(v[6],v[7],g);
    add_edge(v[7],v[8],g);
    add_edge(v[7],v[9],g);
    add_edge(v[8],v[9],g);

    add_edge(v[3],v[10],g);
    add_edge(v[8],v[10],g);
    add_edge(v[0],v[11],g);
    
    scan.process(g);

    property_map<Graph, vertex_cluster_id_t>::type vertex_cluster_id_map = get(vertex_cluster_id, g);
    
    BOOST_CHECK_EQUAL(vertex_cluster_id_map[0], 0);
    BOOST_CHECK_EQUAL(vertex_cluster_id_map[1], 0);
    BOOST_CHECK_EQUAL(vertex_cluster_id_map[2], 0);
    BOOST_CHECK_EQUAL(vertex_cluster_id_map[3], 0);
    BOOST_CHECK_EQUAL(vertex_cluster_id_map[4], 0);
    BOOST_CHECK_EQUAL(vertex_cluster_id_map[5], 1);
    BOOST_CHECK_EQUAL(vertex_cluster_id_map[6], 1);
    BOOST_CHECK_EQUAL(vertex_cluster_id_map[7], 1);
    BOOST_CHECK_EQUAL(vertex_cluster_id_map[8], 1);
    BOOST_CHECK_EQUAL(vertex_cluster_id_map[9], 1);
    BOOST_CHECK_EQUAL(vertex_cluster_id_map[10], 2);
    BOOST_CHECK_EQUAL(vertex_cluster_id_map[11], 3);
}


BOOST_AUTO_TEST_SUITE_END()
        
