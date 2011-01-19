/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: channel.hpp
 Date Created:  <Mon Oct 13 00:46:14 2003>
 Last Modified: <Sun Oct 24 02:05:28 2004>
 Description: 
************************************************************************/

#ifndef CHANNEL_HPP_
#define CHANNEL_HPP_

#include "routing_engine.hpp"
#include "misc.hpp"
#include "router.hpp"
#include "repeater.hpp" // UYO
#include "switch_fabric.hpp"

typedef class Channel : public Buffer_owner {
protected:
    Position pos;                  // pos.x gives direction, pos.y gives the id 
    class Port * port;
    class Router * router;         // to which router is it bound to
    class Repeater * repeater;     // UYO to which repeater it is connected
    bool link_user;                // whether this virtual channel is authorized to 
                                   // use link at this moment

    double load;                    // the overall traffic load
    double dir_load[5];             // dir_load[i] gives the lamda parameter of this input buffer's
                                   // to go to i direction

    virtual bool switch_granted(void) const = 0;

public:
    Channel(class Router * r, class Port * po, Position p, int buf_sz);
    Channel(class Repeater * r, class Port * po, Position p, int buf_sz); // UYO
    ~Channel() {} 
    virtual bool can_send(void) const = 0;
    virtual bool can_receive(void) const = 0;
    virtual bool send(void) = 0;
    virtual bool receive(class Flit * a_flit) = 0;
    virtual void tick(void) = 0;
    void grant_link(void)    { link_user = true; }
    void deprive_link(void)  { link_user = false; }
    bool can_use_link(void) const { return link_user; }
    class Port * get_port(void) const { return port; }
    class Router * get_router(void) const { return router; }
    class Repeater * get_repeater(void) const { return repeater;} // UYO
    Position get_position(void) const { return pos; }

    /* queueing analysis related routines */
    void reset_load(void);
    void add_load(double l, Direction out_dir);
    double get_load(void) const { return load; }
    double get_load(Direction out_dir) const { return dir_load[out_dir]; }
    
    vector <int> congestion_monitor; //UYO
    unsigned int monitoring;

  vector<int> buffer_monitor;

  void record_occupancy(class Flit* a_flit, int action);

    friend ofstream & operator <<(ofstream & of, const class Channel & ch);
};
typedef Channel *pChannel;


ofstream & operator <<(ofstream & of, const class Channel & ch);


typedef class Input_channel : public Channel {
private:
    class Routing_engine re;
    class Sw_input_end * swi;

    unsigned int can_accept; // For flow control

public:
    Input_channel(class Router * r, class Port * po, Position p, int buf_sz);
    Input_channel(class Repeater * r, class Port * po, Position p, int buf_sz); // UYO
    void bind(class Sw_input_end * se) { swi  = se; }
    bool can_send(void) const;
    bool can_receive(void) const;
    bool send(void);
    bool receive(class Flit * a_flit);
    class Sw_input_end * get_bound_sw_end(void) const { return swi; }
    class Output_channel * get_receiver(void) const;
    bool switch_granted(void) const { return (swi != 0 && swi->get_other_end()); }
    class Routing_engine * get_routing_engine(void) { return & re; }
    void tick(void);

    double get_service_rate(void);
    // FLOW CONTROL ROUTINES
    void update_can_accept(unsigned int prediction);
    unsigned int get_can_accept (void) { return can_accept;}

};
typedef Input_channel *pInput_channel;


typedef class Output_channel : public Channel {
private:
    class Sw_output_end * swo;
    int last_inject_time;

    unsigned int can_output; // For flow control

public:
    Output_channel(class Router * r, class Port * po, Position p, int buf_sz);
    Output_channel(class Repeater * r, class Port * po, Position p, int buf_sz); // UYO
    void bind(class Sw_output_end * se) { swo = se; }
    bool can_send(void) const;
    bool can_receive(void) const;
    bool send(void);
    bool receive(class Flit * a_flit);
    class Connector * get_receiver(void) const;
    bool switch_granted(void) const { return (swo != 0 && swo->get_other_end()); }
    class Sw_output_end * get_bound_sw_end(void) const { return swo; }
    void tick(void);
    void set_last_inject_time(int clk) { last_inject_time = clk; }
    int get_last_inject_time(void) const { return last_inject_time; }
    // FLOW CONTROL ROUTINES
    void update_can_output(unsigned int prediction) {can_output=prediction;}
    unsigned int get_can_output(){return can_output;}

};
typedef Output_channel *pOutput_channel;



#endif  // CHANNEL_HPP_
