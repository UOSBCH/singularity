#include "include/scan.hpp"
#include <queue>
#include <boost/graph/graphviz.hpp>

using namespace boost;
using namespace boost::numeric::ublas;
using namespace singularity;

scan::scan(double_type parameter_e, uint parameter_m) {
    this->parameter_e = parameter_e;
    this->parameter_m = parameter_m;
}

void scan::process(Graph& g) {
    calculate_similarity(g);
    calculate_neighbours(g);
    find_clusters(g);
}

void scan::calculate_neighbours(Graph& g)
{
    calculate_neighbours_partial(g, range_t(0, g.m_vertices.size()));
}

void scan::calculate_neighbours_partial(Graph& g, range_t r) {
    Graph::vertex_iterator current, end;
    tie(current, end) = vertices(g);
    for (;current != end; current++) {
        unsigned int id = get(vertex_index, g, *current);
        if (id >= r.start() && id < r.start() + r.size()) {
            calculate_neighbours(g, *current);
        }
    }
}


void scan::calculate_neighbours(Graph& g, Graph::vertex_descriptor vertex)
{
    Graph::out_edge_iterator current, end;
    tie(current, end) =  out_edges(vertex, g);
    unsigned int neighbours_count = 0;
    for(; current != end; current++) {
        bool structural_similarity_is_high = get(edge_similarity_is_high, g, *current);
        if (structural_similarity_is_high) {
            neighbours_count++;
        }
    }
    
    bool is_core;
    
    if (neighbours_count >= parameter_m) {
        is_core = true;
    } else {
        is_core = false;
    }
    put(vertex_is_core, g, vertex, is_core);
    put(vertex_neighbour_count, g, vertex, neighbours_count);
}


void scan::calculate_similarity(Graph& g)
{
    calculate_similarity_partial(g, range_t(0,g.m_edges.size()));
}

void scan::calculate_similarity_partial(Graph& g, range_t r)
{
    Graph::edge_iterator current, end;
    tie(current, end) = edges(g);
    for (;current != end; current++) {
        unsigned int id = get(edge_index, g, *current);
        if (id >= r.start() && id < r.start() + r.size()) {
            calculate_similarity(g, *current);
        }
    }
}

void scan::calculate_similarity(Graph &g, Graph::edge_descriptor link)
{
    double_type parameter_e_sq = parameter_e * parameter_e;
    
    Graph::vertex_descriptor v1 = source(link, g), v2 = target(link, g);
    
    Graph::degree_size_type v_1_N=0, v_2_N=0, v_12_N=0;
    
    v_1_N = out_degree(v1, g) + 1;
    v_2_N = out_degree(v2, g) + 1;
    v_12_N = 2;
    
    Graph::adjacency_iterator current_it, end_it;
    
    tie( current_it, end_it ) = adjacent_vertices(v1, g);
    
    for (; current_it < end_it; current_it++) {
        bool found;
        Graph::edge_descriptor link2x;
        Graph::vertex_descriptor vx = *current_it;
        tie(link2x, found) = edge(v2, vx, g);
        if (found) {
            v_12_N++;
        }
    }
    
    double_type similarity = (double_type(v_12_N * v_12_N)) / (double_type(v_1_N * v_2_N));

    put(edge_similarity, g, link, similarity);
    put(edge_similarity_is_high, g, link, similarity >= parameter_e_sq);
}

void scan::find_clusters(Graph& g)
{
    unsigned int new_cluster_id = 0;
    
    std::queue<Graph::vertex_descriptor> q;
    
    auto vertex_cluster_id_map = get(vertex_cluster_id, g);
    auto vertex_status_map = get(vertex_status, g);
    auto vertex_is_core_map = get(vertex_is_core, g);
    auto edge_similarity_is_high_map = get(edge_similarity_is_high, g);
    
    Graph::vertex_iterator current, end;
    
    tie(current, end) = vertices(g);
    
    for ( ; current != end; current++ ) {
        vertex_status_map[*current] = node_status_unclassified;
    }

    tie(current, end) = vertices(g);
    
    id_generator gen;
    
    for ( ; current != end; current++ ) {
        Graph::vertex_descriptor v =  *current;
        
        if (vertex_status_map[v] != node_status_unclassified) {
            continue;
        }
        
        if (vertex_is_core_map[v]) {
            new_cluster_id = gen.get_next_id();
            vertex_cluster_id_map[v] = new_cluster_id;
            vertex_status_map[v] = node_status_member;
            q.push(v);
            
            while (q.size() > 0) {
                Graph::vertex_descriptor y = q.front();
                
                if (vertex_is_core_map[y]) {
                    Graph::out_edge_iterator current_edge, end_edge;
                    tie(current_edge, end_edge) = out_edges(y, g);
                    for (; current_edge != end_edge; current_edge++ ) {
                        bool similarity_is_high = edge_similarity_is_high_map[*current_edge];
                        if (similarity_is_high) {
                            Graph::vertex_descriptor x = target(*current_edge, g);
                            node_status_t status = vertex_status_map[x];
                            if (status == node_status_unclassified || status == node_status_non_member) {
                                vertex_cluster_id_map[x] = new_cluster_id;
                                vertex_status_map[x] = node_status_member;
                            }
                            if (status == node_status_unclassified) {
                                q.push(x);
                            }
                        }
                    }
                }
                
                q.pop();
            }
        } else {
            vertex_status_map[v] = node_status_non_member;
        }
    }
    
    tie(current, end) = vertices(g);
    
    for ( ; current != end; current++ ) {
        Graph::vertex_descriptor v =  *current;
        node_status_t status = vertex_status_map[v];
        if (status == node_status_non_member) {
            new_cluster_id = gen.get_next_id();
            vertex_cluster_id_map[v] = new_cluster_id;
            bool cluster_is_found = false;
            unsigned int found_cluster_id;
            bool node_is_hub = false;
            Graph::out_edge_iterator current_edge, end_edge;
            tie(current_edge, end_edge) = out_edges(v, g);
            for (; current_edge != end_edge; current_edge++ ) {
                Graph::vertex_descriptor x = target(*current_edge, g);
                if (vertex_status_map[x] == node_status_member) {
                    if (!cluster_is_found) {
                        cluster_is_found = true;
                        found_cluster_id = vertex_cluster_id_map[x];
                    } else {
                        if (found_cluster_id != vertex_cluster_id_map[x]) {
                            node_is_hub = true;
                            break;
                        }
                    }
                }
            }
            
            vertex_status_map[v] = node_is_hub ? node_status_hub : node_status_outlier;
        }
    }

    set_property(g, graph_num_clusters, new_cluster_id + 1);
}

id_generator::id_generator() {
    init();
}


void id_generator::init() {
    current_id = 0;
}

unsigned int id_generator::get_next_id() {
    return current_id ++;
}

void scan::print_graph(Graph& g) {
    
//    
//    dynamic_properties dp;
//    
//    
//    Graph::vertex_iterator current, end;
//    
//    tie (current, end) = vertices(g);
//    for (; current != end; current++) {
//        unsigned int index = get(vertex_index, g, *current);
//        unsigned int cluster_id = get(vertex_cluster_id, g, *current);
//        node_status_t status = get(vertex_status, g, *current);
//        std::string name; 
//        switch (status) {
//            case node_status_member:
//                name = std::string("member");
//                break;
//            case node_status_hub:
//                name = std::string("hub");
//                break;
//            case node_status_outlier:
//                name = std::string("outlier");
//                break;
//            default:
//                name = std::string("?");
//        }
//        
//        std::string format = std::string("%d (%d) %s");
//        put(vertex_name, g, *current, name);
//    } 
//    
//    dp.property("similarity", boost::get(edge_similarity, g));
//    dp.property("neighbours", boost::get(vertex_neighbour_count, g));
//    dp.property("node_id", boost::get(vertex_index, g));
//    dp.property("is_core", boost::get(vertex_is_core, g));
//    dp.property("cluster_id", boost::get(vertex_cluster_id, g));
//    dp.property("status", boost::get(vertex_name, g));
//    
//    write_graphviz_dp(std::cout, g, dp);
    
    property_map<Graph, vertex_cluster_id_t>::type vertex_cluster_id_map = get(vertex_cluster_id, g);
    property_map<Graph, vertex_index_t>::type vertex_id_map = get(vertex_index, g);
    property_map<Graph, edge_similarity_t>::type edge_similarity_map = get(edge_similarity, g);
    
    dynamic_properties dp;
    
    dp.property("cluster_id", vertex_cluster_id_map);
    dp.property("id", vertex_id_map);
    dp.property("similarity", edge_similarity_map);
    
    write_graphviz_dp(std::cout, g, dp, "id");
}

