/************************************************************************
 Author: Umit Ogras  (uogras@ece.cmu.edu)
 File name: repeater.cpp
 Date written:  <Wed Jan 26 13:40:48 2005>
 Last Modified: 
 Description: 
************************************************************************/

#include "common.hpp"
#include "repeater.hpp"
#include "msg.hpp"
#include "global_val.hpp"
#include <iostream>

Repeater::Repeater(pNetwork n, Position p) : Addressee(p),
					     sw(2) {
    if (param.extreme_verbose) 
        cout << "[I] Building repeater for locatieon " << pos << endl;

    network = n;

    in_ports.clear();
    out_ports.clear();

    for (unsigned int i=0; i<2; i++) {  // Repeaters have only 2 ports
        Direction d = (Direction) i;  // Convention: direction={0,1}={North,South}
        pIn_port a_in_port = new In_port(this, d); 
        pOut_port a_out_port = new Out_port(this, d);
        in_ports.push_back(a_in_port);
        out_ports.push_back(a_out_port);
    }
    sw.bind(this);
    // bind switch ends with the input/output channels
    assert(sw.get_size() == in_ports.size() * in_ports[0]->get_size());
    assert(sw.get_size() == out_ports.size() * out_ports[0]->get_size());

    int index = 0;
    for (vector<pIn_port>::iterator iter=in_ports.begin(); iter<in_ports.end(); iter++) {
            pSw_input_end an_end = sw.get_input_end(index++);
            pInput_channel a_chan = (pInput_channel) (*iter)->get_channel(0);
            an_end->bind(a_chan);
            a_chan->bind(an_end);
    }

    index = 0;
    for (vector<pOut_port>::iterator iter=out_ports.begin(); iter<out_ports.end(); iter++) {
            pSw_output_end an_end = sw.get_output_end(index++);
            pOutput_channel a_chan = (pOutput_channel) (*iter)->get_channel(0);
            an_end->bind(a_chan);
            a_chan->bind(an_end);        
    }
    // UYO Make the switch connections: in_ports[0] is connected to out_ports[1] and 1 to 0
    pSw_input_end swi = sw.get_input_end(0);
    pSw_output_end swo = sw.get_output_end(1);
    sw.setup_connection(swi,swo);

    swi = sw.get_input_end(1);
    swo = sw.get_output_end(0);    
    sw.setup_connection(swi,swo);

    // set up the switch's ebit metric using default value
    sw.set_ebit(0);  //  This is adjusted for Repeater!

    // create orion power module
    if (param.orion_power.use_orion_power_model) 
        power_module = new Power_module(param.n_of_ports-param.n_of_extra_links);
    else 
        power_module = NULL;
}


Repeater::~Repeater() {
    if (param.extreme_verbose) 
        cout << "[I] Deleting repeater for location " << pos << endl;

    for (vector<pIn_port>::iterator iter=in_ports.begin(); iter<in_ports.end(); iter++)
        delete (*iter);
    for (vector<pOut_port>::iterator iter=out_ports.begin(); iter<out_ports.end(); iter++)
        delete (*iter);

    if (power_module)
        delete power_module;
}


void Repeater::set_input_channel_buffer_size(int s) {
    for (vector<pIn_port>::iterator iter=in_ports.begin(); iter<in_ports.end(); iter++)
        (*iter)->set_channel_buffer_size(s);
}

void Repeater::set_input_channel_buffer_size(int s, Direction dir) {
    pIn_port p = get_in_port(dir); 
    p->set_channel_buffer_size(s);
}


void Repeater::set_output_channel_buffer_size(int s) {
    for (vector<pOut_port>::iterator iter=out_ports.begin(); iter<out_ports.end(); iter++)
        (*iter)->set_channel_buffer_size(s);
}

// find the index-th input virtual channel in the dir direction of the router
pInput_channel Repeater::get_in_channel(Direction dir, int index) {
    pIn_port a_port = in_ports[dir];
    return ((pInput_channel) a_port->get_channel(index));
}


pOutput_channel Repeater::get_out_channel(Direction dir, int index) {
    pOut_port a_port = out_ports[dir];
    return ((pOutput_channel) a_port->get_channel(index));
}


void Repeater::in_ports_tick(void) {
    for (vector<pIn_port>::iterator iter=in_ports.begin(); iter<in_ports.end(); iter++) 
  	(*iter)->tick();      
}

void Repeater::out_ports_tick(void) {
    for (vector<pOut_port>::iterator iter=out_ports.begin(); iter<out_ports.end(); iter++) 
        (*iter)->tick();
}
                                                                          

