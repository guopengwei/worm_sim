/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: pkt.hpp
 Date Created:  <Sun Oct 12 20:08:52 2003>
 Last Modified: <Sat Dec 18 20:56:56 2004>
 Description: 
************************************************************************/

#ifndef PKT_HPP_
#define PKT_HPP_

#include <vector>
#include "common.hpp"

extern "C" {
#include "SIM_power.h"
#include "SIM_router_power.h"
#include "SIM_power_router.h"
}

using namespace std;

typedef class Flit {
private:
    class Packet * pkt;       // which packet does it belong to

    unsigned int seq_id;               // it's the seq_id-th flit of the packet
    unsigned int flit_size;            // size of the flit
    unsigned int pkt_size;             // how many flits in the same packet?

    // only header flit will contain src_id and dst_id.
    int src_id;
    int dst_id;


    int control_flag;         // reserved

    int * payload;           
    Atom_type raw_data;

    bool processed;           // whether it has been processed by the current node
    int arrival_timestamp;    // when did it arrive at the current node

    vector<class Router *> routing_path;    // records the routers on the path it passes

    Atom_type convert_to_raw(void);

public:

    Flit(class Packet * p, unsigned int s_id, unsigned int pkt_sz, 
         unsigned int flit_sz, int * pld = 0);
    ~Flit();

  int get_sequence_id() const { return seq_id; }
    int get_src_id(void) const;
    Position get_src_position(void) const;
    void write_src_id(int a_id) { src_id = a_id; }
    int get_dst_id(void) const;
    Position get_dst_position(void) const;
    void write_dst_id(int a_id) { dst_id = a_id; }

    unsigned int get_packet_size(void) const;
    unsigned int get_flit_size(void) const { return flit_size; }

    int * set_payload(int * pl);
    int * get_payload(void) { return payload; }

    class Packet * get_packet(void) { return pkt; }

    bool is_header(void) const { return (seq_id == 0); }
    bool is_tail(void) const { return (seq_id + 1 == pkt_size); }
    bool arrived_in_this_cycle(void) const;
    void arrive(void);
    bool is_processed(void) const { return processed; }
    void reset_processed_flag(void) { processed = false; }
  void get_processed(void) {processed = true;}

    void add_a_hop(class Router * router); 
    class Router * get_previous_hop(void);

    Atom_type raw(void) const { return raw_data; }     // retrieve the raw data of the flit to feed to power module

} Flit, *pFlit;


typedef class Packet {
    //    static int id_base;
    int id;                       // an id for this packet, (source, id) identifies a unique packet)
    vector<class Flit *> flits;
    unsigned int packet_size;     // how many flits in this packet
    unsigned int flit_size;

    int birth_time;               // when this packet is borned
    int death_time;               // when this packet die
    int sent_time;                // when this packet leaves the source queue: UYO

public:
    Packet(int pkt_id, unsigned int pkt_sz, unsigned int flit_sz);
    //    Packet(vector<class Flit *> fs);
    ~Packet();
    int age;
    pFlit get_flit(int f_id);
    bool add_flit(class Flit * a_flit);
    unsigned int size(void) const { return packet_size; }
    int get_id(void) const { return id; }
    void set_death_time(int timestamp) { death_time = timestamp; }
    int get_latency(void) const { return death_time - birth_time; }
    void set_sent_time(int timestamp) { sent_time = timestamp; } // UYO
    int get_network_latency(void) const { return death_time - sent_time; } //UYO
    Position get_src_position(void) const { return flits[0]->get_src_position(); }
    Position get_dst_position(void) const { return flits[0]->get_dst_position(); }
} Packet, *pPacket;


typedef vector<pFlit>::iterator iter_flit;
typedef vector<pPacket>::iterator iter_packet;


#endif // PKT_HPP_
