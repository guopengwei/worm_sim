/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: channel.cpp
 Date Created:  <Mon Oct 13 14:08:16 2003>
 Last Modified: <Mon Dec 20 20:35:55 2004>
 Description: 

 Modified by: Umit Ogras (umit@cmu.edu)
 Last Modified: <Sun Jan 29 18:52:28 2005>
 Modification: Added support for the "repeater" objects need to construct
               the long-range links.
			   
 Modified by: Shun-ping Chiu (shunpinc@cmu.edu)
 Modification: Change buf_Write/buffer_read energy model
************************************************************************/

#include <iostream>
#include "channel.hpp"
#include "pkt.hpp"
#include "msg.hpp"


Channel::Channel(pRouter r, pPort po, Position p, int buf_sz) : Buffer_owner(buf_sz),
                                                                router(r) {
    link_user = false; 
    pos = p;
    port = po;
    monitoring = 0;
}

Channel::Channel(pRepeater r, pPort po, Position p, int buf_sz) : Buffer_owner(buf_sz),
                                                                repeater(r) {
    router = 0;
    link_user = false; 
    pos = p;
    port = po;
    monitoring = 0;
}

void Channel::record_occupancy(Flit* a_flit, int action) 
{
    Packet* p = a_flit->get_packet();
    int flit_type = 1;  //assume header
    if (a_flit->is_tail())
      flit_type = 3;   //tail
    else if (!a_flit->is_header())
      flit_type = 2;  //body
      
    buffer_monitor.push_back(p->get_id());
    buffer_monitor.push_back(a_flit->get_sequence_id());
    buffer_monitor.push_back(flit_type);
    buffer_monitor.push_back(net_clock.get_clock());
    buffer_monitor.push_back(action);
    buffer_monitor.push_back(p->get_src_position().x);
    buffer_monitor.push_back(p->get_src_position().y);
    buffer_monitor.push_back(p->get_dst_position().x);
    buffer_monitor.push_back(p->get_dst_position().y);
    buffer_monitor.push_back(buffer.get_num_of_flits());
}

void Channel::reset_load(void) {
    load = 0;
    for (unsigned int i=local; i<((unsigned int) invalid_dir); i++) 
        dir_load[i] = 0;
}

// out_dir gives to which direction this part of traffic will go
void Channel::add_load(double l, Direction out_dir) {
    load += l;
    dir_load[out_dir] += l;
}

// display the channel id in the format of (x,y,dir). 
// NOTE since the matlab solver assumes index starts at 1, we plus the real index with 1
// here
ofstream & operator <<(ofstream & of, const Channel & ch) {
    // note the meaning of ch.pos is not (x,y)
    Position topo_pos = ch.router->get_position();
    of << "(" << topo_pos.x+1 << "," << topo_pos.y+1 << "," << ch.pos.x+1 << ")";
    return of;
}

// we don't need to trigger receiving explicitly here because if the device
// is a receiver, the sender will automatically trigger the receive call
void Output_channel::tick(void) {
    if (can_send()) 
        send();
}


Input_channel::Input_channel(pRouter r, pPort po, Position p, int buf_sz) 
    : Channel(r, po, p, buf_sz), re(r->get_position()) {
    re.bind_channel(this);
    swi = 0;
    can_accept = num_of_empty_buffer_slots()+param.look_ahead;
}

Input_channel::Input_channel(pRepeater r, pPort po, Position p, int buf_sz) 
    : Channel(r, po, p, buf_sz), re(r->get_position())  {swi = 0;}

// we don't need to do a_flit->arrived_in_this_cycle() here since inport tick() will 
// be called before outport tick. and also, we want to be able to process the newly
// generated packet from traffic source as soon as it is generated
void Input_channel::tick(void) {
    if (can_send()) 
      send();
    else {   
        pFlit a_flit = buffer.peek_flit();
        if (a_flit == 0)
        	return;
//         else if (a_flit->arrived_in_this_cycle())
//             return;
        else if (a_flit->is_header() && !a_flit->is_processed()) {
            if (param.extreme_verbose && a_flit->is_header()) {
	      if (this->router) {
		cout << "[I] Input channel " << pos << " at router " 
                     << router->get_position() << " starts processing the packet from " 
                     << a_flit->get_src_position() << " to " << a_flit->get_dst_position() 
                     << " (time = " << net_clock.get_clock() << ")\n";
	      }
	      else {
		cout << "[I] Input channel " << pos << " at repeater " 
                     << repeater->get_position() << " starts processing the packet from " 
                     << a_flit->get_src_position() << " to " << a_flit->get_dst_position() 
                     << " (time = " << net_clock.get_clock() << ")\n";
	      }
            }
	    a_flit->get_processed();
	    if (this->router)
	    {
	      re.get_request(a_flit);
	    }
	}
    }
}

// An input channel can send data only if the switch is granted, it has
// flit in the buffer and the receving channel can receive
bool Input_channel::can_send(void) const {
    if (buffer.is_empty())
        return false;
    // This is somehow tricky, we need to verify whether the first flit in the fifo
    // is received right in this clock cycle. If so, we can not send it
    const Flit * first_flit = buffer.peek_flit();
    if (first_flit->arrived_in_this_cycle())
      return false;
    pOutput_channel out_channel = get_receiver();
    if (out_channel == 0)
        return false;

    return out_channel->can_receive();
}


// An input channel can receive data only if it's authorized to use the link 
// and it has empty slot in its buffer. Note that depending on the order of 
// execution as shown in Network::tick(), we need to check carefully whether 
// the buffer is full. For instance, if output channel is executed after
// input channel in a tick(), it can happen that input channel was full at the 
// begining of this clock and will be empty at the end of this clock. So be 
// careful here. WE ASSUME WORMSIM ARCHITECTURE DOES NOT SUPPORT PIPELINE.
bool Input_channel::can_receive(void) const {
    assert (buffer.get_capacity() > 0);
    return (can_use_link() && !buffer.is_full_in_this_clock() && !buffer.is_full());
}

// Return the receiver channel. If not exist yet, return 0
pOutput_channel Input_channel::get_receiver(void) const {
    if (!switch_granted())
        return 0;
    pSw_output_end swo = (pSw_output_end) swi->get_other_end();
    return ((pOutput_channel) swo->get_bound_channel());
}

bool Input_channel::send(void) {
#ifdef DEBUG
    assert(can_send());
#endif
    pOutput_channel out_channel = get_receiver();
    //cout <<"Output channel is: " <<out_channel->get_position() <<endl;
    pFlit a_flit = get_flit();
    out_channel->receive(a_flit);
   
    if (net_clock.get_clock() > param.warmup_period) {
      pSwitch_fabric sw=NULL;
      if (this->router)
        sw = router->get_switch();
      else
        sw = repeater->get_switch();
      if (param.orion_power.use_orion_power_model) {
        pPower_module pm=NULL;
        if (this->router)
          pm = router->get_power_module();
        else
          pm = repeater->get_power_module();
        assert(pm);
        pm->power_buffer_read(pos.x, a_flit->raw());
        Position out_pos = out_channel->get_position();
        pm->power_crossbar_trav(pos.x, out_pos.x, a_flit->raw());
      }
      // ebit energy models
      param.energy.buf_read_energy += param.buf_read_ebit * a_flit->get_flit_size();
      //if (this->repeater)
      //  cout <<"For repeater: " << repeater->get_position() <<endl;

      if (this->router)
        param.energy.xbar_energy += sw->get_ebit() * a_flit->get_flit_size();

      if (monitoring) {
        record_occupancy(a_flit, 0); //0 means sent
      }
      
    }

      if (a_flit->is_tail())
        if (this->router)  
          re.teardown_connection();

    return true;
}

void Input_channel::update_can_accept(unsigned int prediction) {

  can_accept = prediction;
  //  if (param.flow_control && param.extreme_verbose)
  //cout << "Input channel " << pos << " at " 
  //    << router->get_position() << " can accept " 
  //     << prediction << " at time " << net_clock.get_clock() <<" \n";
  
}

bool Input_channel::receive(pFlit a_flit) {
#ifdef DEBUG
    assert(can_receive());
#endif

    a_flit->reset_processed_flag();
    add(a_flit); 
    if (net_clock.get_clock() > param.warmup_period) {
      if (param.orion_power.use_orion_power_model) {
        pPower_module pm; 
        if (this->router)
          pm = router->get_power_module();
        else
          pm = repeater->get_power_module();
        assert(pm);
        pm->power_buffer_write(pos.x, a_flit->raw());
      }
      param.energy.buf_write_energy += param.buf_write_ebit * a_flit->get_flit_size();
    
      if (monitoring) {
        record_occupancy(a_flit, 1);  //1 means received
      }
    }
    
    return true;
}

// get the service rate of this input channel under no congestion situation
double Input_channel::get_service_rate(void) {
    const int switch_fabric_delay = 1;

    int r_delay = re.get_delay();
    pArbiter arb = router->get_arbiter();
    int a_delay = arb->get_delay();
    //return (1. / (r_delay + a_delay + switch_fabric_delay - 1));
    return (1. / (r_delay + a_delay + switch_fabric_delay));
}


Output_channel::Output_channel(pRouter r, pPort po, Position p, int buf_sz) : 
  Channel(r, po, p, buf_sz) {
  swo = 0;
  last_inject_time = 0;
  can_output = 0;    // This should initialize from the down_stream input channel
}

Output_channel::Output_channel(pRepeater r, pPort po, Position p, int buf_sz) : 
    Channel(r, po, p, buf_sz) {
    swo = 0;
    last_inject_time = 0;
}

pConnector Output_channel::get_receiver(void) const {
    if (!link_user)
        return 0;
    pLink link = (pLink) port->get_sink();
    return link->get_sink();
}


// An output channel can send data only if it is authorized to use the link,
// and the down-stream input channel or the traffic sink is ready to receive
bool Output_channel::can_send(void) const {
    if (buffer.is_empty())
        return false;

    // This is somehow trick, we need to verify whether the first flit in the fifo
    // is received right in this clock cycle. If so, we can not send it
    const Flit * first_flit = buffer.peek_flit();
    if (first_flit->arrived_in_this_cycle())
        return false;

    if (!link_user)
        return false;
    pConnector receiver = get_receiver();

    if (receiver)
        return receiver->can_receive();
    else
        return false;
}


// An output channel can receive data only it has empty space in its buffer
bool Output_channel::can_receive(void) const {
    //return (!buffer.is_full());
    return (!buffer.is_full_in_this_clock() && !buffer.is_full());
}


bool Output_channel::send(void) {
#ifdef DEBUG
    assert(can_send());
#endif

    pFlit a_flit = get_flit();

    if (param.extreme_verbose && a_flit->is_header()) {
      if (this->router) {
        cout << "[I] Output channel " << pos << " at router " 
             << router->get_position() << " just sent the packet from " 
             << a_flit->get_src_position() << " to " << a_flit->get_dst_position() 
             << " (time = " << net_clock.get_clock() << ")\n";
      }
      else {
	cout << "[I] Output channel " << pos << " at repeater " 
             << repeater->get_position() << " just sent the packet from " 
             << a_flit->get_src_position() << " to " << a_flit->get_dst_position() 
             << " (time = " << net_clock.get_clock() << ")\n";
      }
    }


  //begin rtg
    if (monitoring && net_clock.get_clock() > param.warmup_period)
      record_occupancy(a_flit, 0);
  //end rtg



    pLink link = (pLink) port->get_sink();
    link->receive(a_flit);
    return true;
}


bool Output_channel::receive(pFlit a_flit) {
#ifdef DEBUG
    assert(can_receive());
#endif

    a_flit->reset_processed_flag();
    add(a_flit);

    if (param.extreme_verbose && a_flit->is_header()) {
      if (this->router) {
        cout << "[I] Output channel " << pos << " at router " 
             << router->get_position() << " receive a packet from " 
             << a_flit->get_src_position() << " to " << a_flit->get_dst_position() 
             << " (time = " << net_clock.get_clock() << ")\n";
      }
      else {
	cout << "[I] Output channel " << pos << " at repeater " 
             << repeater->get_position() << " receive a packet from " 
             << a_flit->get_src_position() << " to " << a_flit->get_dst_position() 
             << " (time = " << net_clock.get_clock() << ")\n";
      }
    }




    //begin rtg
    if (monitoring && net_clock.get_clock() > param.warmup_period) {
      record_occupancy(a_flit, 1);
    }
    //end rtg




    // TODO: if we have mutliple channels sharing the same port, we need to 
    // request the port here

    return true;
}
