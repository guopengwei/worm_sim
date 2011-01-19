/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: routing_engine.hpp
 Date Created:  <Mon Oct 13 00:24:00 2003>
 Last Modified: <Sat Oct 23 23:37:43 2004>
 Description: Routing engine is used to decide the output direction of 
              the output channel. Each channel/virtual channel will have 
              one of the engine
************************************************************************/

#ifndef ROUTING_ENGINE_HPP_
#define ROUTING_ENGINE_HPP_

#include "common.hpp"
#include "misc.hpp"


typedef class Routing_engine : public Timer_owner {
protected:
    Position pos;
    Routing_scheme routing_scheme;
    vector<unsigned int> routing_table; // UYO
    class Input_channel * channel;
    int req_time;
    int grant_time;
    int delay_xy;
    int delay_oe;

    Direction dir;      // where to route this packet

    int send_request_to_arbiter(void);    // send the request to arbiter for port connection
    void timer_fired(void);
    Routing_scheme decide_best_routing_mode(void) const;

public:
    Direction decide_direction(class Flit** flit, const Position & src_pos, const Position & dst_pos) const;
    Direction decide_direction(const Position & src_pos, const Position & dst_pos) const;
    Direction decide_direction_xy(const Position & src_pos, const Position & dst_pos) const;
    Direction decide_direction_oe(const Position & src_pos, const Position & dst_pos) const;
    Direction decide_direction_test(const Position & src_pos, const Position & dst_pos) const;
    Direction decide_direction_oe_fixed(const Position & src_pos, const Position & dst_pos) const;
    Direction decide_direction_predict(const Position & src_pos, const Position & dst_pos) const;

    void set_routing_scheme(Routing_scheme rs);
    void set_routing_table(char * table_string); // UYO
    void get_request( class Flit * flit);
    int receive_grant_from_arbiter(void);
    void teardown_connection(void);
    Routing_engine(Position p);
    ~Routing_engine();
    void bind_channel(class Input_channel * chan); 
    int get_delay(void) const;
};

typedef Routing_engine *pRouting_engine;


#endif  //ROUTING_ENGINE_HPP_
