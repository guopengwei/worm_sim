/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: port.cpp
 Date Created:  <Wed Oct 15 16:04:13 2003>
 Last Modified: <Sun Dec 19 22:40:48 2004>
 Description: Added flow control routines Umit Y. Ogras
************************************************************************/

#include "port.hpp"
#include "pkt.hpp"

Port::~Port() {
    for (vector<pChannel>::iterator iter=channels.begin(); iter<channels.end(); iter++) 
        delete (*iter);
}

void Port::tick(void) {
    for (vector<pChannel>::iterator iter=channels.begin(); iter<channels.end(); iter++) 
      (*iter)->tick();                                                                      
}

void Port::set_channel_buffer_size(int s) {
    for (vector<pChannel>::iterator iter=channels.begin(); iter<channels.end(); iter++) 
        (*iter)->set_buffer_size(s);
}

// return the sum of the empty buffer slots of all the channels beglong to this port
unsigned int Port::num_of_empty_buffer_slots(void) const {
    unsigned int sum = 0;
    for (vector<pChannel>::const_iterator iter=channels.begin(); iter<channels.end(); iter++)
        sum += (*iter)->num_of_empty_buffer_slots();
    return sum;
}

// return the buffer capacity which is the sum of all input channels' buffer size of this port
unsigned int Port::total_buffer_capacity(void) const {
    unsigned int sum = 0;
    for (vector<pChannel>::const_iterator iter=channels.begin(); iter<channels.end(); iter++)
        sum += (*iter)->get_capacity();
    return sum;
}

In_port::In_port(class Router * r, Direction d) : Port(r, d) {
    pNetwork network = r->get_network();
    size = network->get_num_of_vcs();

    channels.clear();
    Position channel_pos;
    channel_pos.x = d;
    // override buffer size if it's local 
    int buffer_size = param.in_channel_buffer_size;
//     if (d == local) 
//         buffer_size = 100;
    for (unsigned int i=0; i<size; i++) {
        channel_pos.y = i;
        pInput_channel a_chan = new Input_channel(r, this, channel_pos, buffer_size);
        channels.push_back(a_chan);
        a_chan->deprive_link();
    }

    // at the beginning, the link is granted to channel0
    channels[0]->grant_link();
}

In_port::In_port(class Repeater * r, Direction d) : Port(r, d) { // UYO
    size = 1; // No virtual channel for repeater

    channels.clear();
    Position channel_pos;
    channel_pos.x = d;
    channel_pos.y = 0;
    int buffer_size = 1; // Fized buffer size for repeater
    pInput_channel a_chan = new Input_channel(r, this, channel_pos, buffer_size);
    channels.push_back(a_chan);
    a_chan->deprive_link();
    // at the beginning, the link is granted to channel0
    channels[0]->grant_link();
}

void In_port::set_routing_scheme(Routing_scheme rs) {
    for (vector<pChannel>::const_iterator iter=channels.begin(); iter<channels.end(); iter++) {
        pInput_channel a_channel = (pInput_channel) *iter;
        pRouting_engine re = a_channel->get_routing_engine();
        re->set_routing_scheme(rs);
    }
}

void In_port::set_routing_table(char * table_string) { //UYO
    for (vector<pChannel>::const_iterator iter=channels.begin(); iter<channels.end(); iter++) {
        pInput_channel a_channel = (pInput_channel) *iter;
        pRouting_engine re = a_channel->get_routing_engine();
        re->set_routing_table(table_string);
    }
}

// whether this port has any data to send to the out port via switch
bool In_port::can_send(void) const {
    for (vector<pChannel>::const_iterator iter=channels.begin(); iter<channels.end(); iter++) 
        if ((*iter)->can_send())
            return true;
    return false;
}

// whether the current link user can receive or not
bool In_port::can_receive(void) const {
    pInput_channel active_channel = (pInput_channel) channels[link_user_channel_id];
    return active_channel->can_receive();
}


bool In_port::send(void) {
    for (vector<pChannel>::const_iterator iter=channels.begin(); iter<channels.end(); iter++) 
        if ((*iter)->can_send()) {
            (*iter)->send();
        }
    return true;
}


bool In_port::receive(pFlit a_flit) {
#ifdef DEBUG
    assert(can_receive());
#endif
       
    pInput_channel active_channel = (pInput_channel) channels[link_user_channel_id];
    return active_channel->receive(a_flit);
}


unsigned int In_port::get_can_accept() {
       
    pInput_channel active_channel = (pInput_channel) channels[link_user_channel_id];
    return active_channel->get_can_accept();
}

unsigned int In_port::get_total_acceptable() {
  unsigned int sum = 0,ind=0;;
  for (vector<pChannel>::const_iterator iter= channels.begin(); iter<channels.end(); iter++) {
    pInput_channel curr_channel = (pInput_channel) channels[ind];
    sum += curr_channel->get_can_accept();
    ind ++;
  }
  return sum;
}

Out_port::Out_port(class Router * r, Direction d) : Port(r, d) {
    pNetwork network = r->get_network();
    size = network->get_num_of_vcs();

    channels.clear();
    Position channel_pos;
    channel_pos.x = d;
    int buffer_size = param.out_channel_buffer_size;
    if (d == local && param.out_channel_buffer_size < 2)
        buffer_size = 2;   // out channel should be at least 2 in order to mimic infinite local out buffer
    for (unsigned int i=0; i<size; i++) {
        channel_pos.y = i;
        pOutput_channel a_chan = new Output_channel(r, this, channel_pos, buffer_size);
        channels.push_back(a_chan);
        a_chan->deprive_link();
    }

    // channel 0 has the access to the link after initialization
    channels[0]->grant_link();
}

Out_port::Out_port(class Repeater * r, Direction d) : Port(r, d) {
    size = 1;

    channels.clear();
    Position channel_pos;
    channel_pos.x = d;
    channel_pos.y = 0;
    int buffer_size = 1;
    pOutput_channel a_chan = new Output_channel(r, this, channel_pos, buffer_size);
    channels.push_back(a_chan);
    a_chan->deprive_link();
    // channel 0 has the access to the link after initialization
    channels[0]->grant_link();
}

// whether the current link user can send data or not
bool Out_port::can_send(void) const {
    pOutput_channel active_channel = (pOutput_channel) channels[link_user_channel_id];
    return active_channel->can_send();
}

// whether this port's channel can receive any data via switch
bool Out_port::can_receive(void) const {
    for (vector<pChannel>::const_iterator iter=channels.begin(); iter<channels.end(); iter++) 
        if ((*iter)->can_receive())
            return true;
    return false;
}


bool Out_port::send(void) {
    pOutput_channel active_channel = (pOutput_channel) channels[link_user_channel_id];
    return active_channel->send();
}


// do nothing as the receiving at the granularity of each channel for out port
bool Out_port::receive(pFlit a_flit) {
    return true;
}

Direction Out_port::get_direction(void) const {
  return dir;
}
