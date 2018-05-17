
#ifndef SCAN_HPP
#define SCAN_HPP

#include "utils.hpp"

namespace singularity {
    enum node_status_t {
        node_status_unclassified,
        node_status_non_member,
        node_status_member,
        node_status_hub,
        node_status_outlier
    };

    enum vertex_cluster_id_t {
        vertex_cluster_id
    };

    enum vertex_neighbour_count_t {
        vertex_neighbour_count
    };

    enum vertex_is_core_t {
        vertex_is_core
    };

    enum vertex_status_t {
        vertex_status
    };

    enum edge_similarity_t {
        edge_similarity
    };

    enum edge_similarity_is_high_t {
        edge_similarity_is_high
    };

    enum graph_num_clusters_t {
        graph_num_clusters
    };

    typedef boost::property< edge_similarity_t, double, 
            boost::property< boost::edge_index_t, unsigned int,
            boost::property< edge_similarity_is_high_t, bool > > >
    EdgeProperties;

    typedef boost::property< vertex_cluster_id_t, unsigned int,
            boost::property< vertex_neighbour_count_t, unsigned int,
            boost::property< vertex_is_core_t, bool,
            boost::property< boost::vertex_name_t, std::string,
            boost::property< vertex_status_t, node_status_t > > > > >
    VertexProperties;

    typedef boost::property< graph_num_clusters_t, unsigned int > GraphProperties;

    typedef boost::adjacency_list<
            boost::vecS, 
            boost::vecS, 
            boost::undirectedS, 
            VertexProperties, 
            EdgeProperties, 
            GraphProperties
    > Graph;

    class id_generator
    {
    public:
        id_generator();
        void init();
        unsigned int get_next_id();
    private:
        volatile unsigned int current_id;
    };
    
    class scan
    {
    public:
        scan(double parameter_e, uint parameter_m);
        void process(Graph &g);
        void print_graph(Graph& g);
    private:
        double parameter_e;
        uint parameter_m;
        void calculate_neighbours(Graph &g);
        void calculate_neighbours_partial(Graph &g, range_t r);
        void calculate_neighbours(Graph &g, Graph::vertex_descriptor vertex);
        void calculate_similarity(Graph &g);
        void calculate_similarity_partial(Graph& g, range_t r);
        void calculate_similarity(Graph &g, Graph::edge_descriptor link);
        void find_clusters(Graph &g);
    };
}

namespace boost {
    using namespace singularity;
    BOOST_INSTALL_PROPERTY(vertex, cluster_id);
    BOOST_INSTALL_PROPERTY(vertex, neighbour_count);
    BOOST_INSTALL_PROPERTY(vertex, is_core);
    BOOST_INSTALL_PROPERTY(vertex, status);
    BOOST_INSTALL_PROPERTY(edge, similarity);
    BOOST_INSTALL_PROPERTY(edge, similarity_is_high);
    BOOST_INSTALL_PROPERTY(graph, num_clusters);
}

#endif /* SCAN_HPP */

