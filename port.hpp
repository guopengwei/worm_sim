/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: port.hpp
 Date Created:  <Wed Oct 15 15:55:56 2003>
 Last Modified: <Fri Dec 17 20:17:01 2004>
 Description: A port is essentially a bundle of virtual channels,
************************************************************************/

#ifndef PORT_HPP_
#define PORT_HPP_

#include "common.hpp"
#include "misc.hpp"
#include "channel.hpp"

typedef class Port : public Connector { 
protected:
    unsigned int size;
    Direction dir;
    class Router * router;
    class Repeater * repeater;
    vector<class Channel *> channels;
    unsigned int link_user_channel_id;      // which channel in the port is currently using
                                            // the link
	bool async_connect;						//set this port to connect async ..
	Router *async_src;
	Router *async_dst;
public:
    Port(class Router * r, Direction d) { router = r; dir = d; link_user_channel_id = 0; }
    Port(class Repeater * r, Direction d) { repeater = r; dir = d; link_user_channel_id = 0; } // UYO
    class Channel * get_channel(int index) { return channels[index]; }
    class Channel * get_link_user_channel(void) { return channels[link_user_channel_id]; }
    unsigned int get_size(void) const { return size; }
    unsigned int num_of_empty_buffer_slots(void) const;
    unsigned int total_buffer_capacity(void) const;
    void set_channel_buffer_size(int s);
    Direction get_direction(void) const { return dir; }
    class Router * get_router(void) { return router; }
    class Repeater * get_repeater(void) { return repeater; }
    ~Port();
    void tick(void);
} Port, *pPort;


typedef class In_port : public Port {
public:
  In_port(class Router * r, Direction d);
  In_port(class Repeater * r, Direction d);
  bool can_send(void) const;
  bool can_receive(void) const;
  bool send(void);
  bool receive(class Flit * a_flit);
  void set_routing_scheme(Routing_scheme rs);
  void set_routing_table(char * table_string); // UYO
  // UYO: Flow control
  unsigned int get_can_accept(void);
  unsigned int get_total_acceptable(void);

} In_port, *pIn_port;


typedef class Out_port : public Port {
public:
  Out_port(class Router * r, Direction d);
  Out_port(class Repeater * r, Direction d);
  bool can_send(void) const;
  bool can_receive(void) const;
  bool send(void);
  bool receive(class Flit * a_flit);
  Direction get_direction(void) const;
} Out_port, *pOut_port;

#endif  // PORT_HPP_
