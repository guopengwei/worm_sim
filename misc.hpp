/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: misc.hpp
 Date Created:  <Mon Oct 13 10:44:44 2003>
 Last Modified: <Wed Jan 19 11:32:07 2005>
 Description: Some helper classes for the a2router project
************************************************************************/

#ifndef MISC_HPP_
#define MISC_HPP_

#include <list>
#include <vector>
#include <cstring>
#include "global_val.hpp"
#include "common.hpp"
#include <limits.h>

using namespace std;

typedef class Net_clock {
	int clk;
    list<class Timer *> timers;
public:
    Net_clock(void) { clk = 0; timers.clear(); }
    void reset(void) { clk = 0; }
    int tick(void);
    int get_clock(void) const { return clk; }
    void print_time(void) const;
    void register_timer(class Timer * t)
    { timers.push_back(t); }
};
typedef Net_clock *pNet_clock;


typedef struct Param {

  // simulator mode control
  bool verbose; 
  bool extreme_verbose;
  bool print_simulation_time;
  long int rand_seed;
  
  vector<char*> config_files;
  char* equation_file;
  char* perf_anal_lib_equation_file;
  char* config_file_to_dump;
  
  bool dump_traffic_source_trace;
  bool dump_input_buffer;
  bool dump_output_buffer;
  
  Topology topology;
  ATGSets  ATGSETs;
  
  unsigned int n_of_rows;
  unsigned int n_of_cols;
  unsigned int source_buffer_size;
  unsigned int sink_buffer_size;
  unsigned int in_channel_buffer_size;
  unsigned int out_channel_buffer_size;
  
  unsigned int n_of_switch_ports; 
  
  unsigned int n_of_ports;       // number of ports per router
  int n_of_extra_links; // UYO
  double  n_of_packets; // UYO
  unsigned int max_n_of_packets; 
  double  n_cnt ; //UYO
  double n_t;
  bool  adaptive_LR1;
  unsigned int look_ahead; // UYO how many steps ahead prediction
  bool flow_control;       // =1 if flow controller is on
  int availability_thresh;
  int on_off_complete_time;
  bool  print_n_of_packets;  // The number of packets in the network as a function of time
  char* file_to_print_n_of_packets; // File to print the number of packets
  ofstream packets_t;
  ofstream logFile;

  unsigned int n_of_vcs;         // number of virtual channels per port (unsupported yet)
  
  int routing_engine_delay_xy;
  int routing_engine_delay_oe;
  Routing_scheme routing_scheme;
  
  int arbitration_delay;
  
  // NOTE (10/24/2004) 
  
  // If false, there should be one cycle idle time after each packet
  // in this model. 
  // If true, the router architeture allows the use of the output
  // channel right in the next clock cycle. arbiter_lookahead model 
  // is more difficult for performance analysis
  bool arbiter_lookahead; 
  
  double packet_generating_rate;
  double packet_consuming_rate;
  
  unsigned int flit_size;
  unsigned int flits_per_packet;
  unsigned int overhead_per_packet;
  
  class Network * network;
  
  Traffic_mode traffic_mode;
  
  double switch_mode_threshold;    // threshold for switching the router mode 
  
  int simulation_length;
  int warmup_period;   // don't collect status data in the first warm_up_period clock cycles
  
  bool parse_error;
  bool quit_flag;       // if true, quit immediately without building network and simulation
  
  //    bool analyze_latency; // if true, do performance analysis
  
  // energy related parameters
  double link_ebit;
  double xbar_ebit;
  double fixed_routing_engine_epacket;
  double oe_routing_engine_epacket;
  double arbiter_epacket;
  double buf_read_ebit;
  double buf_write_ebit;
  
  // data collection
  int n_of_received_packets;
  int n_of_sent_packets;
  int gen_delay;              // UYO delay due to delaying packet generation
  double ave_gen_delay;
  double total_latency;
  double avg_latency;
  
  // accumulated energy consumption
  struct {
    double link_energy;
    double xbar_energy;
    double re_energy;
    double arbiter_energy;
    double buf_read_energy;
    double buf_write_energy; 
    double total_energy;
  } energy;
  
  
  // orion power model parameters
  struct {
    bool use_orion_power_model;
    double link_length;      // link length in um
    double link_energy;
    double xbar_energy;
    double re_energy;
    double arbiter_energy;
    double buf_energy;
    double total_energy;
  } orion_power;
  
  
  // hotspot traffic mode parameters
  // Refer to the paper "The odd-even turn model for adaptive routing" for a description of 
  // this model and some terminology
  double hotspot_percentage;             // how much extra percentage each hotspot get
  vector<unsigned int> hotspots;        // what are those hotspot nodes?
  vector<unsigned int> non_hotspots;    // what are those non hotspot nodes?
  
  string get_routing_scheme_in_string(void) const;
  string get_traffic_mode_in_string(void) const;
  Param(void);
};
typedef Param *pParam;


typedef class Timer {
  int delay;
  int init_time;
  int fire_time;
  bool initialized;
  bool fired;
  class Timer_owner * owner;
public:
  Timer(void);
  void initialize(void);
  void set_delay(int d) { delay = d; fire_time = init_time + delay; }
  int get_delay(void) const { return delay; }
  void tick(void);
  void fire(void);
  void bind(class Timer_owner * o);
};
typedef Timer *pTimer;


typedef class Timer_owner {
protected:
  class Timer timer;
public:
  virtual void timer_fired(void) {};
  Timer_owner(void);
  void init_timer(void) { timer.initialize(); }
  void set_delay(int d) { timer.set_delay(d); }
  virtual ~Timer_owner() {};
};
typedef Timer_owner *pTimer_owner;


typedef class Buffer {
private:
  list<class Flit *> fifo;
  int size;             // how many flits allowed in the fifo
  int last_full_time;   // when is the last time the fifo is full
  int last_get_time;    // when is the time for get an element from the buffer
  int last_add_time;    // when is the time for put an element to the buffer
  
public:
  Buffer(int sz);
  ~Buffer();
  bool add(class Flit * flit);
  class Flit * get_flit(void);
  class Flit * peek_flit(void) const;
  void set_flitage(int age);
  bool is_empty(void) const;
  bool is_full(void) const;
  bool is_full_in_this_clock(void) const;
  unsigned int empty_slots(void) const;
  unsigned int get_num_of_flits(void) const { return fifo.size(); }
  void flush(void) { fifo.clear(); }
  unsigned int get_capacity(void) const { return ((unsigned int) size); }
  void set_capacity(int s) { size = s; }
};
typedef Buffer *pBuffer;


typedef class Buffer_owner {
protected:
  class Buffer buffer;
  
  bool add(class Flit * flit);
  class Flit * get_flit(void);
  bool add(class Packet * packet);
  class Packet * get_packet(void);
  
public:
  Buffer_owner(int buf_sz) : buffer(buf_sz) {}
  virtual ~Buffer_owner() {}
  void flush_buffer(void) { buffer.flush(); }
  virtual void bind(class Link * l) {}
  virtual void bind(class Router * r) {}
  unsigned int num_of_empty_buffer_slots(void) const { return buffer.empty_slots(); }
  unsigned int get_capacity(void) const { return buffer.get_capacity(); }
  void set_buffer_size(int s) { buffer.set_capacity(s); }
  unsigned int get_num_of_flits(void) const { return buffer.get_num_of_flits();}
};
typedef Buffer_owner *pBuffer_owner;


typedef class Addressee {
protected:
  Position pos;
public:
  Addressee(Position p) { pos = p; }
  Position get_position(void) const { return pos; }
  void print_position(void) const;
};
typedef Addressee *pAddressee;


// A connector can be a port, a link, or a traffic source/sink
typedef class Connector {
protected:
  class Connector * source;
  class Connector * sink;
  
public:
  void bind_source(class Connector * s) { source = s; }
  void bind_sink(class Connector * s) { sink = s; }
    Connector() { source = sink = 0; }
  Connector(class Connector * src, class Connector * snk) { source = src; sink = snk; }
  
  class Connector * get_source(void) const { return source; }
  class Connector * get_sink(void) const { return sink; }
  virtual ~Connector() {}
  virtual bool can_send(void) const = 0;
  virtual bool can_receive(void) const = 0;
  virtual bool send(void) = 0;
  virtual bool receive(class Flit * a_flit) = 0;  
};
typedef Connector *pConnector;


bool operator ==(const class Addressee * a, const Position & pos);

#endif  // MISC_HPP_
