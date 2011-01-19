/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: pkt.cpp
 Date Created:  <Sun Oct 12 20:22:52 2003>
 Last Modified: <Wed Dec 22 20:33:12 2004>
 Description: 
************************************************************************/

#include "pkt.hpp"
#include "global_val.hpp"
#include "network.hpp"
#include "msg.hpp"
#include <iostream>
#include <cstdlib>
using namespace std;


int Flit::get_src_id(void) const {
    if (seq_id != 0) {
        cerr << ERRO_NOT_HEADER_FLIT;
        return -1;
    }
    return src_id;
}

Position Flit::get_src_position(void) const {
    if (seq_id != 0) 
        cerr << ERRO_NOT_HEADER_FLIT;

    pNetwork network = param.network;
    pTraffic_source traffic_source = network->get_traffic_source(src_id);
    return traffic_source->get_position();
}

int Flit::get_dst_id(void) const {
    if (seq_id != 0) {
        cerr << ERRO_NOT_HEADER_FLIT;
        return -1;
    }
    return dst_id;
}

Position Flit::get_dst_position(void) const {
    if (seq_id != 0) 
        cerr << ERRO_NOT_HEADER_FLIT;

    pNetwork network = param.network;
    pTraffic_sink traffic_sink = network->get_traffic_sink(dst_id);
    return traffic_sink->get_position();
}

unsigned int Flit::get_packet_size(void) const {
    if (seq_id != 0) {
        cerr<<"[E] Information not available in non-header flit."<<endl;
        return 0;
    }
    return pkt_size;
}

Flit::Flit(class Packet * p, unsigned int s_id, unsigned int pkt_sz, 
           unsigned int flit_sz, int * pld) {
    assert(flit_sz > 0);
    assert(s_id >= 0 && s_id < pkt_sz);
    seq_id = s_id;

    pkt = p;
    pkt_size = pkt_sz;
    flit_size = flit_sz;

    processed = false;

    payload = pld;

    // we need the if gaurd to save a little bit run time and also to make sure
    // the results is compatible with the previous implementation as there we don't
    // use random number except for control packet generation time.
    if (param.orion_power.use_orion_power_model)
        raw_data = convert_to_raw();
    else 
        raw_data = 0;
}

// TODO: before we fix the packet format and support real trace, 
// we will just use random data
Atom_type Flit::convert_to_raw(void) {
    Atom_type ret = (Atom_type) rand();
    // if flit_size is 32 bits, the first half of ret will be 0
    if (param.flit_size != 8 * sizeof(Atom_type)) {
        assert(param.flit_size == 32);
        ret <<= 32;
//        ret += lrand48();
          ret+=rand();
    }
    return ret;
}

Flit::~Flit() {
    if (payload)
        delete []payload;
}

bool Flit::arrived_in_this_cycle(void) const {
    return (arrival_timestamp == net_clock.get_clock());
}

// called when this flit arrived and buffered at a new component
void Flit::arrive(void) {
    arrival_timestamp = net_clock.get_clock();
    processed = false;   // have not been processed by this component
}

void Flit::add_a_hop(pRouter router) {
    if (!is_header()) 
        cerr << ERRO_NOT_HEADER_FLIT;
    else 
        routing_path.push_back(router);
}

pRouter Flit::get_previous_hop(void) {
    if (!is_header()) {
        cerr << ERRO_NOT_HEADER_FLIT;
        return 0;
    }
    else if (routing_path.empty())
        return 0;
    else
        return routing_path[routing_path.size() - 1];
}


Packet::Packet(int pkt_id, unsigned int pkt_sz, unsigned int flit_sz) {
    id = pkt_id;
    packet_size = pkt_sz;
    flit_size = flit_sz;
    assert(packet_size > 0);
    assert(flit_size > 0);

    flits.clear();

    for (unsigned int i=0; i<pkt_sz; i++) {
        pFlit a_flit = new Flit(this, i, packet_size, flit_size);
        flits.push_back(a_flit);
    }

    birth_time = net_clock.get_clock();
}


Packet::~Packet() {
    for (vector<pFlit>::iterator iter=flits.begin(); iter<flits.end(); iter++)
        delete (*iter);
    flits.clear();
    if (param.verbose) 
        cout<<"[I] Packet "<<id<<" destroyed at time "<<net_clock.get_clock()<<endl;
}


pFlit Packet::get_flit(int f_id) {
    assert(f_id>=0);
    assert(((unsigned) f_id) < flits.size());
    return flits[f_id];
}


