/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: arbiter.hpp
 Date Created:  <Tue Oct 14 13:18:28 2003>
 Last Modified: <Sat Dec 18 21:28:10 2004>
 Description: Arbiter for the switching fabric in the router
************************************************************************/

#ifndef ARBITER_HPP_
#define ARBITER_HPP_

#include "common.hpp"
#include "misc.hpp"

#include <vector>

extern "C" {
#include "SIM_power.h"
#include "SIM_router_power.h"
#include "SIM_power_router.h"
}


struct Sw_connection_request{
    class Input_channel * channel;
    int timestamp;
    Direction indir;
    Direction dir;
};
typedef Sw_connection_request *pSw_connection_request;

typedef class Arbiter {
    int delay;                  // the delay it takes to proecess the request
    int num_of_switch_port;     // the switch it controls has how many ports
    class Router * router;
    int numofreqgranted;
    int numofreqfailed;
    vector<struct Sw_connection_request *>* request_queue;
    bool content[5];
    void orion_record_arbiter_power(Direction dir);
    // Direction get_current_granted_channel_dir(Direction dir);
    Atom_type form_req_vector(Direction dir) const;

public:
    void tick(void);
    Direction get_current_granted_channel_dir(Direction dir);
    Arbiter(int d, int n_sw_p);
    ~Arbiter();
    void bind(class Router * r) { router = r; }
    void update_contention();
    void receive_connection_request(class Sw_connection_request * req);
    void signalcontention(int dir, bool cont);
    void process_requests(void);
    bool process_requests_of_port(unsigned int dir);
    void setup_connection(class Input_channel * in, class Output_channel * out);
    void teardown_connection(class Input_channel * in);
    int get_requestsize(Direction dir) { return request_queue[dir].size();}
    int get_delay(void) const { return delay; }
};
typedef Arbiter *pArbiter;


#endif  // ARBITER_HPP_
