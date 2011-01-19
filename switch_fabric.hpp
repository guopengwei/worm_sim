/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: switch_fabric.hpp
 Date Created:  <Tue Oct 14 13:21:03 2003>
 Last Modified: <Sun Oct 24 01:51:38 2004>
 Description: Switching fabric for the router
************************************************************************/

#ifndef SWITCH_FABRIC_HPP_
#define SWITCH_FABRIC_HPP_

#include "common.hpp"
#include <vector>

using namespace std;

typedef class Sw_end {
protected:
    class Channel * chan;
    class Sw_end * connected_to;              // which end does it currently connected
public:
    void bind(class Channel * c) { chan = c; }
    void connect(class Sw_end * e) { connected_to = e; }
    void disconnect(void) { connected_to = 0; }
    bool is_busy(void) const { return (connected_to != 0); }
    Sw_end(void) { chan = 0; connected_to = 0; }
    class Sw_end * get_other_end(void) { return connected_to; }
    class Channel * get_bound_channel(void) { return chan; }
} Sw_end, *pSw_end;


typedef class Sw_input_end : public Sw_end {
} Sw_input_end, *pSw_input_end; 


typedef class Sw_output_end : public Sw_end{
private:
} Sw_output_end, *pSw_output_end;


typedef class Switch_fabric {
    double ebit;
    unsigned int size;
    class Router * router;
    class Repeater * repeater; // UYO
    vector<class Sw_input_end *> inputs;
    vector<class Sw_output_end *> outputs;

public:
    void bind(class Router * r) { router = r; }
    void bind(class Repeater * r) { repeater = r; } // UYO
    Switch_fabric(int sz);
    void connect(class Sw_input_end * in, class Sw_output_end * out);
    unsigned int get_size(void) const { return size; }
    class Sw_input_end * get_input_end(int index) { return inputs[index]; }
    class Sw_output_end * get_output_end(int index) { return outputs[index]; }
    void setup_connection(pSw_input_end swi, pSw_output_end swo);
    void teardown_connection(pSw_input_end swi, pSw_output_end swo);
    void set_ebit(double e) { ebit = e; }
    double get_ebit(void) const { return ebit; }
    friend class Arbiter;
} Switch_fabric, *pSwitch_fabric;


#endif  // SWITCH_FABRIC_HPP_
