/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: network.hpp
 Date Created:  <Tue Oct 14 14:05:10 2003>
 Last Modified: <Wed Jan 19 12:08:30 2005>
 Description: Class that implements the whole network.
************************************************************************/

#ifndef NETWORK_HPP_
#define NETWORK_HPP_

#include "common.hpp"
#include "router.hpp"
#include "link.hpp"
#include "traffic.hpp"

typedef class Network {
private:
    bool success;

    // configuration parameters
    Topology topology;
    unsigned int n_of_rows;
    unsigned int n_of_cols;
    unsigned int n_of_ports;
    unsigned int n_of_extra_links; // UYO
    unsigned int n_of_switch_ports;
    unsigned int n_of_vcs;

    vector<class Router *> routers;
    vector<class Link *> links;
    vector<class Traffic_source *> sources;
    vector<class Traffic_sink *> sinks;

private:
    void calc_channel_load(void);
//     void perf_anal_init_lambdas(double * n_lambda);
//     void perf_anal_init_probs(double * n_prob);
    bool overloaded(double * n_lambda);

public:
    void reset(void); 
    Network(void);
    ~Network(void); 
    class Router * get_router(Position & p);
    class Router * get_router(int index) { return routers[index]; } 
    class Traffic_source * get_traffic_source(Position & p);
    class Traffic_source * get_traffic_source(int index) { return sources[index]; }
    class Traffic_sink * get_traffic_sink(Position & p);
    class Traffic_sink * get_traffic_sink(int index) { return sinks[index]; }
    class Link * connect(class Connector * src,  class Connector * dst, int delay=0);

    Topology get_topology(void) const { return topology; }
    vector<class Repeater *> repeaters; // UYO
    unsigned int get_num_of_rows(void) const { return n_of_rows; }
    unsigned int get_num_of_cols(void) const { return n_of_cols; }
    unsigned int get_num_of_ports(void) const { return n_of_ports; }
    unsigned int get_num_of_vcs(void) const { return n_of_vcs; }

    unsigned int get_num_of_traffic_sources(void) const { return sources.size(); }
    unsigned int get_num_of_traffic_sinks(void) const { return sinks.size(); }

    double get_total_injection_rate(void) const;
    
    void tick(void); 
    bool get_success_flag(void) { return success; }
    //    double analyze_latency(void);

    bool dump_equation_file(void);
    bool dump_perf_anal_lib_equation_file(void);
    bool dump_config_file(void);
};

typedef Network *pNetwork;

#endif  // NETWORK_HPP_
