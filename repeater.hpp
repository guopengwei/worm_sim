/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: repeater.hpp
 Date Created:  <Tue Oct 14 12:52:35 2003>
 Last Modified: <Thu Dec 16 18:50:36 2004>
 Description: 
************************************************************************/

#ifndef REPEATER_HPP_
#define REPEATER_HPP_

#include <vector>
#include "common.hpp"
#include "channel.hpp"
#include "routing_engine.hpp"
#include "switch_fabric.hpp"
#include "network.hpp"
#include "port.hpp"
#include "power.hpp"

typedef class Repeater : public Addressee {
private:
    class Network * network;

    vector<class In_port *> in_ports;
    vector<class Out_port *> out_ports;

    class Switch_fabric sw;
    class Power_module * power_module;    // module for orion power model

public:
    Repeater(class Network * n, struct Position p);
    ~Repeater();
    class Network * get_network(void) { return network; }
    class In_port * get_in_port(Direction dir) { return in_ports[dir]; }
    class Out_port * get_out_port(Direction dir) { return out_ports[dir]; }
    class Input_channel * get_in_channel(Direction dir, int index = 0);
    class Output_channel * get_out_channel(Direction dir, int index = 0);

    class Switch_fabric * get_switch(void) { return &sw; }
    void set_switch_ebit(double e) { sw.set_ebit(e); }

    void set_input_channel_buffer_size(int s);
    void set_input_channel_buffer_size(int s, Direction dir);
    void set_output_channel_buffer_size(int s);
   
    class Power_module * get_power_module(void) { return power_module; }

    void in_ports_tick(void);
    void out_ports_tick(void);
};
typedef Repeater *pRepeater;


#endif  // REPEATER_HPP_
