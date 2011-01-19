/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: router.hpp
 Date Created:  <Tue Oct 14 12:52:35 2003>
 Last Modified: <Thu Dec 16 18:50:36 2004>
 Description: 
************************************************************************/

#ifndef ROUTER_HPP_
#define ROUTER_HPP_

#include <vector>
#include "common.hpp"
#include "channel.hpp"
#include "routing_engine.hpp"
#include "switch_fabric.hpp"
#include "arbiter.hpp"
#include "network.hpp"
#include "port.hpp"
#include "power.hpp"

typedef class Router : public Addressee {
private:
  class Network * network;
  
  vector<class In_port *> in_ports;
  vector<class Out_port *> out_ports;
  
  class Switch_fabric sw;
  class Arbiter arbiter;
  
  class Power_module * power_module;    // module for orion power model
  In_port *sink_port;

  // UYO
  unsigned int monitor_router;
  


public:
  Router(class Network * n, struct Position p);
  ~Router();
  class Network * get_network(void) { return network; }
  class In_port * get_in_port(Direction dir) { return in_ports[dir]; }
  class Out_port * get_out_port(Direction dir) { return out_ports[dir]; }
  class Input_channel * get_in_channel(Direction dir, int index = 0);
  class Output_channel * get_out_channel(Direction dir, int index = 0);
  class Output_channel * get_link_user_channel(Direction dir);
  class Router * get_router(Direction dir);
  
  class Arbiter * get_arbiter(void) { return &arbiter; }
  class Switch_fabric * get_switch(void) { return &sw; }
  void receivecontention(int srcRouter,int dir, bool con);
  void set_switch_ebit(double e) { sw.set_ebit(e); }
  void set_power_module(int s); //UYO
  void clear_contention(void);
  void set_input_channel_buffer_size(int s);
  void set_input_channel_buffer_size(int s, Direction dir);
  void set_output_channel_buffer_size(int s);
  void set_routing_scheme(Routing_scheme rs);
  void set_routing_table(char *table_string); // UYO
  void set_sink(In_port *dst_port) {sink_port = dst_port;} //UYO
  In_port *get_sink() {return sink_port;} //UYO
  void set_xbar_ebit(double ebit) {this->sw.set_ebit(ebit);}
  class Power_module * get_power_module(void) { return power_module; }
  
  void in_ports_tick(void);
  void out_ports_tick(void);
  void arbiter_tick();
  //UYO
  unsigned int get_num_of_ports(void) {return in_ports.size();}
  void set_monitor(void);
  unsigned int get_monitor(void) {return monitor_router;}
  // FLOW CONTROL ROUTINES: UYO
  void update_prediction();
  bool contention[5][5];
};
typedef Router *pRouter;


#endif  // ROUTER_HPP_
