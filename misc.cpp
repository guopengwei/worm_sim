/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: misc.cpp
 Date Created:  <Mon Oct 13 11:10:31 2003>
 Last Modified: <Wed Jan 19 15:43:38 2005>
 Description: 
************************************************************************/

#include <iostream>
#include <iomanip>
#include <vector>
#include <list>
#include <algorithm>
#include "misc.hpp"
#include "pkt.hpp"
#include "msg.hpp"
#include "global_val.hpp"

using namespace std;






void check_timer(pTimer t) {
    t->tick();
}

void Net_clock::print_time(void) const {
    cout << "At time " << setw(4) << clk << ":";
}

int Net_clock::tick(void) {
    int old_clk = clk;
    clk ++;

    if (old_clk >= clk) 
        cerr << ERRO_WRAP_CLOCK;

    for_each(timers.begin(), timers.end(), check_timer);
    return clk;
}

// Initialize some default system parameters
Param::Param(void) {

    config_files.clear();                               // no configure file
    equation_file = 0;                                  // no equation file
    perf_anal_lib_equation_file = 0;                    // no performance analysis lib equ
    config_file_to_dump = 0;                            // no configure file to dump out

    dump_traffic_source_trace = false;                  // don't dump trace by default
    dump_input_buffer = false;
    dump_output_buffer =false;


    verbose = false;
    extreme_verbose = false;
    print_simulation_time = false;

    rand_seed = 123456;
    logFile.open ("wormsim_data/log.txt");
    topology = mesh;
    n_of_rows = 8;
    n_of_cols = 8;
    source_buffer_size = INT_MAX;
    sink_buffer_size = INT_MAX;
    in_channel_buffer_size = 3;
    out_channel_buffer_size = 1;

    n_of_ports = 6; // UYO    
    n_of_packets = 0; // UYO
    gen_delay = 0;
    ave_gen_delay = 0.0;
    max_n_of_packets = 0;
    n_cnt = 0; //UYO
    n_t = 0; //UYO
    n_of_extra_links = 1; // UYO: At most one random link to each router
    adaptive_LR1 = false;
    look_ahead = 5;       // UYO: Same as default flits per packet
    flow_control = 0;     // flow controller is off by default
    availability_thresh = 0; // Basic controller by default
    on_off_complete_time = 0;
    print_n_of_packets = false;
    file_to_print_n_of_packets = 0;
    //param.packets_t.open(file_to_print_n_of_packets);

    n_of_vcs = 1;

    n_of_switch_ports = n_of_ports * n_of_vcs;

    flit_size = 64;                 // only 64 bits are supported now if you want to simulate power
                                    // using orion power model
    flits_per_packet = 5;
    overhead_per_packet = 12;       // how many bits are used for packet overhead

    packet_generating_rate = 0;     // generate a packet with a probability of 0.025 every clock
    packet_consuming_rate = 0;          // consume a packet immediately

    routing_engine_delay_xy = 1;
    routing_engine_delay_oe = 2;

    routing_scheme = xy;

    arbitration_delay = 1;
    arbiter_lookahead = false;

    traffic_mode = uniform;

    simulation_length = 200000;
    warmup_period = 0;

    switch_mode_threshold = 0.61;
    
    //    analyze_latency = false;

    parse_error = false;
    quit_flag = false;

    // default value for hotspot traffic mode parameters
    hotspot_percentage = 6.0;        // each hotspot got extra 6.0 percentage in being destination
    hotspots.clear();

    // default value for energy consumption
    link_ebit = 0.449e-12;
    xbar_ebit = 0.284e-12;
    fixed_routing_engine_epacket = 0.1e-12;
    oe_routing_engine_epacket = 0.2e-12;
    arbiter_epacket = 1.155e-12;
    buf_read_ebit = 1.056e-12;
    buf_write_ebit = 2.831e-12;

    energy.link_energy = 0.;
    energy.xbar_energy = 0.;
    energy.re_energy = 0.;
    energy.arbiter_energy = 0.;
    energy.buf_read_energy = 0.;
    energy.buf_write_energy = 0.;
    energy.total_energy = 0.;

    // orion power model
    orion_power.use_orion_power_model = false;
    orion_power.link_length = 1000;    // each link is 1000 um long
    orion_power.link_energy = 0.;
    orion_power.xbar_energy = 0.;
    orion_power.re_energy = 0.;
    orion_power.arbiter_energy = 0.;
    orion_power.buf_energy = 0.;
    orion_power.total_energy = 0.;

    // statistics collection data
    n_of_received_packets = 0;
    n_of_sent_packets = 0;
    total_latency = 0.;
    avg_latency = 0.;
}


string Param::get_routing_scheme_in_string(void) const {
    switch (routing_scheme) {
    case xy:
        return "XY";
    case test:
           return "test";
    case oe:
        return "Odd Even";
    case oe_fixed:
        return "Fixed Odd Even";
    case dyad:
        return "DyAD";
    case predict:
        return "Prediction_based";
    case rtable:
        return "Routing Table"; //UYO
    default:
        return "Invalid";
    }
}

string Param::get_traffic_mode_in_string(void) const {
    switch (traffic_mode) {
    case uniform:
        return "uniform"; 
    case transpose1:
        return "transpose1";
    case transpose2:
        return "transpose2";
    case hotspot:
        return "hotspot";
    default:
        return "invalid";
    }
}


Timer::Timer(void) {
    initialized = false;
    fired = false;
}

void Timer::bind(pTimer_owner o) {
    owner = o;
}

void Timer::initialize(void) {
    init_time = net_clock.get_clock();
    fire_time = init_time + delay;
    fired = false;
    initialized = true;

    // in the case of delay case, fire immediately
    if (delay == 0)
        fire();
}


void Timer::tick(void) {
    if (!initialized)
        return;
    if (fire_time == net_clock.get_clock()) 
        fire();
    else if (fire_time < net_clock.get_clock()) {
        cerr<<"[W] timer missed its scheduled fire time, fire now."<<endl;
        fire();
    }
}


void Timer::fire(void) {
    fired = true;
    initialized = false;
    // call the owner's callback function
    owner->timer_fired();
}


Timer_owner::Timer_owner(void) {
    timer.bind(this);
    net_clock.register_timer(&timer);
}


Buffer::Buffer(int sz) {
    fifo.clear();
    size = sz;
    last_full_time = -1;
    last_get_time  =-1;
    last_add_time = -1;
    assert (size >= 0);
}


Buffer::~Buffer() {
    fifo.clear();
}

// get the next flit from the buffer and delete the flit entry in the buffer
// return 0 if no flit left in the buffer
pFlit Buffer::get_flit(void) {
    if (fifo.empty())
        return 0;

    if (is_full()) 
        last_full_time = net_clock.get_clock();

    pFlit f = *(fifo.begin());
    fifo.erase(fifo.begin());

    last_get_time = net_clock.get_clock();
    return f; 
}

// check whether the buffer is indeed full during sometime of this clock
bool Buffer::is_full_in_this_clock(void) const {
    return (last_full_time == net_clock.get_clock());
}

// return false if success, true if not 
bool Buffer::add(pFlit flit) {
    if (is_full()) {
        cerr << WARN_BUFFER_OVERFLOW;
        return true;
    }
    else {
        fifo.push_back(flit); 
        last_add_time = net_clock.get_clock();
        return false;
    }
}


// peek into the buffer and get the pointer to the first flit in the buffer
class Flit * Buffer::peek_flit(void) const{
    if (fifo.empty())
        return 0;
    const pFlit f = *(fifo.begin());
    return f;
}


bool Buffer::is_full(void) const {
    return (fifo.size() == (unsigned) size);
}


bool Buffer::is_empty(void) const {
    return fifo.empty();
}


// return the number of empty slots at the end of the previous clock edge
unsigned int Buffer::empty_slots(void) const {
    int res = size - fifo.size();
    if (last_get_time == net_clock.get_clock()) 
        res --;
    if (last_add_time == net_clock.get_clock())
        res ++;
    return res;
}


// return true if adding flit fails
// return false otherwise
bool Buffer_owner::add(pFlit flit) {
    if (buffer.is_full()) {
        cerr << WARN_BUFFER_OVERFLOW;
        return true;
    }

    // arrived this flit, we should mark it with the current time stamp, and reset some of
    // the flags
    flit->arrive();

    buffer.add(flit);
    return false;
}

// return true if adding packet fails
// return false otherwise
bool Buffer_owner::add(pPacket packet) {
    if (buffer.empty_slots() >= packet->size()) {
        for (unsigned int i=0; i<packet->size(); i++) {
            pFlit a_flit = packet->get_flit(i);
            buffer.add(a_flit);
        }
        return false;
    }
    else {
        cerr << WARN_BUFFER_OVERFLOW;
        return true;
    }
}

// return the flit if success, return 0 otherwise
pFlit Buffer_owner::get_flit(void) {
    if (buffer.is_empty())
        return 0;
    return buffer.get_flit();
}

// return the packet if success, return 0 otherwise
pPacket Buffer_owner::get_packet(void) {
    if (buffer.is_empty()) 
        return 0;
    const Flit* f = buffer.peek_flit();
    if (f->is_header()) {
        unsigned int packet_size = f->get_packet_size();
        // we need to check if the buffer contains all the trailing flits for the packet
        if (buffer.get_num_of_flits() >= packet_size) {
            pFlit header_flit = buffer.get_flit();
            for (unsigned int i=1; i<packet_size; i++) 
                buffer.get_flit();
            return header_flit->get_packet();
        }
    }
    return 0;
}

void Addressee::print_position(void) const {
    cout << "(" << pos.x << "," << pos.y << ")";
}


bool operator ==(const class Addressee * a, const Position & pos) {
    return (a->get_position() == pos);
}

bool operator ==(const Position & pos1, const Position & pos2) {
    return (pos1.x == pos2.x && pos1.y == pos2.y);
}

bool operator !=(const Position & pos1, const Position & pos2) {
    return (!(pos1 == pos2));
}

ostream & operator <<(ostream & os, const Position & pos) {
    os << "(" << pos.x << "," << pos.y << ")";
    return os;
}

Direction reverse(Direction dir) {
    switch (dir) {
    case north:
        return south;
    case south:
        return north;
    case east:
        return west;
    case west:
        return east;
    case local:
        return local;
    default:
        return invalid_dir;
    }
}
