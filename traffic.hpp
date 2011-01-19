/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: traffic.hpp
 Date Created:  <Tue Oct 14 14:32:20 2003>
 Last Modified: <Sat Dec 18 16:31:13 2004>
 Description: Packets generator and packets consumer classes
************************************************************************/

#ifndef TRAFFIC_HPP_
#define TRAFFIC_HPP_

#include <fstream>
#include "common.hpp"
#include "misc.hpp"

typedef class Traffic_source : public Buffer_owner, public Connector, public Addressee{
private:
    static unsigned int id_base;
    unsigned int id;
    unsigned int packet_size;
    unsigned int flit_size;
    unsigned int time_start;
    double packet_generating_rate;
    int pkt_id;
    double* packet_to_destination_rate;
    // UYO 
    unsigned int on_period, off_period, wait_period;
    unsigned int on_off_model;
    int next_generation_time;
    int total_pkt_sent;
    int pkt_to_sent;

    Traffic_mode traffic_mode;

    int period;                // period for packet generation using trace_file
    ifstream trace_file;
    int trace_file_loop_cnt;   // how many times we have gone over the trace file so far

  bool trace_file_empty;

    ofstream trace_dump;       // trace file to dump out

    typedef struct Message {
        int timestamp;
        unsigned int destination;
        unsigned int size;
    } Message, *pMessage;

    Message pre_fetched_message;
    bool get_next_message(Message & msg);

    unsigned int get_destination_uniform(void) const; 
    unsigned int get_destination_transpose1(void) const;
    unsigned int get_destination_transpose2(void) const;
    unsigned int get_destination_hotspot(void) const;
    unsigned int get_destination_customized(void) const;

    void generate_a_packet(unsigned int dst_id);
    void generate_a_packet(unsigned int dst_id,unsigned int custom_packet_size);
    void generate_packets(const Message & rec);

public:
    Traffic_source(Position p, int buf_sz);
    ~Traffic_source();
    bool can_send(void) const;
    bool can_receive(void) const { return false; }
    bool send(void);
    bool receive(class Flit * a_flit) { return false; }
    class Connector * get_receiver(void) const; 

    static void reset_id_base(void) { id_base = 0; }

    void tick(void);

    /* traffic control routines */
    void set_time_start(unsigned time);
    void set_packet_generating_rate(double r);
    void set_packet_to_destination_rate(unsigned int dst_id, double rate);
    double get_packet_to_destination_rate(unsigned int dst_id) const;
    double get_total_packet_injection_rate(void) const;
    int set_trace_file(char * file_name);
    bool has_trace_file(void) { return (trace_file.is_open()); }
    int get_id(void) const { return id; }
    void set_OnOff(unsigned int on_per,unsigned int off_per,unsigned int wait_per);  // UYO
    vector <int> src_congestion_monitor; //UYO
    unsigned int src_monitoring;
  
  vector<int> buffer_monitor;

  int get_total_pkt_sent(void) {return total_pkt_sent;}

};

typedef Traffic_source *pTraffic_source;


typedef class Traffic_sink : public Buffer_owner, public Connector, public Addressee{
private:
    static unsigned int id_base;
    unsigned int id;
    double packet_consuming_rate;
    double load;

public:
    Traffic_sink(Position p, int buf_sz);
    static void reset_id_base(void) { id_base = 0; }
    bool can_send(void) const { return false; }
    bool can_receive(void) const;
    bool send(void) { return false; }
    bool receive(class Flit * a_flit);

    void tick(void);
    bool consume_a_packet(void);
    unsigned int get_id(void) { return id; }

    void set_packet_consuming_rate(double r) { assert(r>=0); packet_consuming_rate = r; }

  vector<int> buffer_monitor;
  int monitoring;
  

    /* queueing analysis related routines */
    void reset_load(void) { load = 0; }
    void add_load(double l) { load += l; }
    double get_load(void) const { return load; }
};

typedef Traffic_sink *pTraffic_sink;

#endif  // TRAFFIC_HPP_
