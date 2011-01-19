/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: routing_engine.cpp
 Date Created:  <Mon Oct 13 11:48:06 2003>
 Last Modified: <Wed Jan 19 15:49:42 2005>
 Description: Implementation of the routing engine class. Currently it does not
              pipeline the routing decision.
************************************************************************/

#include "routing_engine.hpp"
#include "misc.hpp"
#include "global_val.hpp"
#include "msg.hpp"
#include "pkt.hpp"
#include "router.hpp"
#include "arbiter.hpp"
#include <iostream>
#include <cstring>
Routing_engine::Routing_engine(Position p) {
    pos = p;
    routing_scheme = param.routing_scheme;
    routing_table = vector<unsigned int>();//UYO
    delay_xy = param.routing_engine_delay_xy;
    delay_oe = param.routing_engine_delay_oe;
    timer.set_delay(delay_xy);
}

Routing_engine::~Routing_engine() {
}

void Routing_engine::set_routing_scheme(Routing_scheme rs) {
    assert (rs != invalid_scheme);
    routing_scheme = rs;
}

void Routing_engine::set_routing_table(char * table_string) { // UYO
  //cout <<"At location " << this->pos.x <<", " <<this->pos.y <<":\n";
  //cout <<"u*** \n";
  for (unsigned int i=0; i<strlen(table_string); i+=2){
    unsigned int ali;
    sscanf(&table_string[i],"%d",&ali);
    routing_table.push_back(ali);
    //cout <<table_string[i] <<" string \n "; cout <<ali << " int \n";
    // cout << routing_table.back() <<" ";
  }
  // cout <<"\n***u\n"; 
}

void Routing_engine::bind_channel(class Input_channel * chan) {
    channel = chan;
}

// Based on it's local and neighbor information, this routine decides which routing 
// mode is the best for use at this time.
// The way we calculate it:
// If any of the 3 surrounding router's corresponding channels' inputs are higher than 
// a certain ratio, then use oe_fixed routing; otherwise, use oe
Routing_scheme Routing_engine::decide_best_routing_mode(void) const {
    pRouter router = channel->get_router();

    for (unsigned int i=0; i<param.n_of_ports; i++) {
        Direction dir = (Direction) i;
        if (dir == local)
            continue;
        Position channel_pos = channel->get_position();
        if (dir == ((Direction) channel_pos.x))   // no need to check backward router
            continue;

        pRouter neighbor_router = router->get_router(dir);
        if (neighbor_router) {
            pIn_port neighbor_in_port = neighbor_router->get_in_port(reverse(dir));
            unsigned int empty_slots = neighbor_in_port->num_of_empty_buffer_slots();
            unsigned int capacity = neighbor_in_port->total_buffer_capacity();
            if (((double) empty_slots)/capacity <= param.switch_mode_threshold)
                return oe;
        }
    }
    return oe_fixed;
}

// when a new head flit arrives, it will trigger this function to
// decide for the output channel. after delay by "delay" cycle, the
// routing engine will make the decision and inform the switching
// fabric about which output channel it wants to connect to
void Routing_engine::get_request(class Flit* flit) {



    if (!flit->is_header()) {
        cerr << ERRO_NOT_HEADER_FLIT;
        return;
    }

    Position src_pos = flit->get_src_position();
    Position dst_pos = flit->get_dst_position();

    if (param.extreme_verbose) 
        cout << "[I] Routing engine received a request " 
             << " (time = " << net_clock.get_clock() << ")\n";

    dir = decide_direction(src_pos, dst_pos);

    timer.set_delay(get_delay());
    // by calling this, its timer_fired member function will be called after "delay"
    // clock cycles.
    timer.initialize();
}

// decide the routing direction Daniel
Direction 
Routing_engine::decide_direction(class Flit** flit, const Position & src_pos, const Position & dst_pos) const {
    Routing_scheme current_routing_scheme = routing_scheme;
    int ali;
    pRouter curr_router = channel->get_router();          // current router
    pIn_port neighbor_port = 0;
    unsigned int empty_slots = 1;
    unsigned int capacity = 1;

    if (routing_scheme == dyad) 
        current_routing_scheme = decide_best_routing_mode();

    Direction dir = invalid_dir;

    switch(current_routing_scheme) {
    case xy: 
        dir = decide_direction_xy(src_pos, dst_pos);
        break;
    case oe:
        dir = decide_direction_oe(src_pos, dst_pos);
        break;
    case oe_fixed:
        dir = decide_direction_oe_fixed(src_pos, dst_pos);
        break;
    case predict:
        dir = decide_direction_predict(src_pos, dst_pos);
	break;
    case rtable: // UYO
        ali = (unsigned int) routing_table[dst_pos.y+param.n_of_cols*dst_pos.x];
	dir = (Direction) ali;
	/* for (i=0; i<param.n_of_rows*param.n_of_cols; i++)
	  cout <<routing_table[i] <<"$ "; 
	  cout << endl; 
	if (dir != decide_direction_xy(src_pos, dst_pos))
	    cout <<" ALARM! \n";
        cout << dst_pos.x <<" " <<dst_pos.y<< " " <<dst_pos.x+param.n_of_cols*dst_pos.y <<" " <<"\n";
	cout << this->pos.x <<" " <<this->pos.y << " " <<this->pos.x+param.n_of_cols*this->pos.y << "\n";
	cout << this->routing_table[dst_pos.x*param.n_of_cols+dst_pos.y] <<" why\n";
	cout << dir <<", " <<ali << " is direction \n";  */

	if (param.adaptive_LR1 && dir == 5) {
	  neighbor_port = curr_router->get_sink();
	  empty_slots = neighbor_port->num_of_empty_buffer_slots();
	  capacity = 2*neighbor_port->total_buffer_capacity();
	  //printf("empty: %u, capacity %u \n",empty_slots, capacity);
	  if (((double) empty_slots)/capacity < 0.49) {
	   // dir = decide_direction_xy(src_pos, dst_pos); // use XY due to congestion
	    // cout <<"*\n";
	  }
	} // if ( dir == 5) */
       
	// annotate power
	if (net_clock.get_clock() > param.warmup_period) {
	  // ebit energy models
	  param.energy.re_energy += param.fixed_routing_engine_epacket;
	}
        break;
    default:
        assert(0);
        dir = invalid_dir;
    } //switch
    return dir;
}

Direction
Routing_engine::decide_direction(const Position & src_pos, const Position & dst_pos) const {
    Routing_scheme current_routing_scheme = routing_scheme;
    int ali;
    pRouter curr_router = channel->get_router();          // current router
    pIn_port neighbor_port = 0;
    unsigned int empty_slots = 1;
    unsigned int capacity = 1;

    if (routing_scheme == dyad)
        current_routing_scheme = decide_best_routing_mode();

    Direction dir = invalid_dir;

    switch(current_routing_scheme) {
    case xy:
        dir = decide_direction_xy(src_pos, dst_pos);
        break;
    case oe:
        dir = decide_direction_oe(src_pos, dst_pos);
        break;
    case oe_fixed:
        dir = decide_direction_oe_fixed(src_pos, dst_pos);
        break;
    case test:
          dir = decide_direction_test(src_pos, dst_pos);
        break;
    case predict:
        dir = decide_direction_predict(src_pos, dst_pos);
        break;
    case rtable: // UYO
        ali = (unsigned int) routing_table[dst_pos.y+param.n_of_cols*dst_pos.x];
	dir = (Direction) ali;
	/* for (i=0; i<param.n_of_rows*param.n_of_cols; i++)
	  cout <<routing_table[i] <<"$ ";
	  cout << endl;
	if (dir != decide_direction_xy(src_pos, dst_pos))
	    cout <<" ALARM! \n";
        cout << dst_pos.x <<" " <<dst_pos.y<< " " <<dst_pos.x+param.n_of_cols*dst_pos.y <<" " <<"\n";
	cout << this->pos.x <<" " <<this->pos.y << " " <<this->pos.x+param.n_of_cols*this->pos.y << "\n";
	cout << this->routing_table[dst_pos.x*param.n_of_cols+dst_pos.y] <<" why\n";
	cout << dir <<", " <<ali << " is direction \n";  */

	if (param.adaptive_LR1 && dir == 5) {
	  neighbor_port = curr_router->get_sink();
	  empty_slots = neighbor_port->num_of_empty_buffer_slots();
	  capacity = 2*neighbor_port->total_buffer_capacity();
	  //printf("empty: %u, capacity %u \n",empty_slots, capacity);
	  if (((double) empty_slots)/capacity < 0.49) {
	   // dir = decide_direction_xy(src_pos, dst_pos); // use XY due to congestion
	    // cout <<"*\n";
	  }
	} // if ( dir == 5) */

	// annotate power
	if (net_clock.get_clock() > param.warmup_period) {
	  // ebit energy models
	  param.energy.re_energy += param.fixed_routing_engine_epacket;
	}
        break;
    default:
        assert(0);
        dir = invalid_dir;
    } //switch
    return dir;
}

// decide the routing direction under XY routing scheme
Direction
Routing_engine::decide_direction_xy(const Position & src_pos, const Position & dst_pos) const {
    // annotate power
    if (net_clock.get_clock() > param.warmup_period) {
        // ebit energy models
        param.energy.re_energy += param.fixed_routing_engine_epacket;
    }

    Direction d;
    if (dst_pos.x > pos.x)
        d = east;
    else if (dst_pos.x < pos.x)
        d = west;
    else if (dst_pos.y > pos.y)
        d = north;
    else if (dst_pos.y < pos.y)
        d = south;
    else
      d = local;

    return d;
}

//daniel test
Direction
Routing_engine::decide_direction_test(const Position & src_pos, const Position & dst_pos) const {
	 vector<Direction> dirs;
	    dirs.clear();

	    // annotate power
	    if (net_clock.get_clock() > param.warmup_period) {
	        // ebit energy models
	        param.energy.re_energy += param.oe_routing_engine_epacket;
	    }

	    int e0 = dst_pos.x - pos.x;
	    int e1 = dst_pos.y - pos.y;

	    if (e0 == 0 && e1 == 0)
	        dirs.push_back(local);
	    else {
	        if (e0 == 0) {       // currently in the same column as destination
	            if (e1 > 0)
	                dirs.push_back(north);
	            else
	                dirs.push_back(south);
	        }
	        else {
	            if (e0 > 0)  {    // east bound messages
	                if (e1 == 0)
	                    dirs.push_back(east);
	                else {
	                    if (pos.x%2 == 1 || pos.x == src_pos.x) {
	                        if (e1 > 0)
	                            dirs.push_back(north);
	                        else
	                            dirs.push_back(south);
	                    }
	                    if (dst_pos.x%2 == 1 || e0 != 1)   // odd destinatio column or >=2 columns of destination
	                        dirs.push_back(east);
	                }
	            }
	            else {     // west bound messges
	                dirs.push_back(west);
	                if (pos.x%2 == 0) {
	                    if (e1 > 0)
	                        dirs.push_back(north);
	                    else if (e1 < 0)
	                        dirs.push_back(south);
	                }
	            }
	        }
	    }

	    // now select a direction from dirs vector
	    assert(!dirs.empty());

	    if (dirs.size() == 1)
	        return dirs[0];

	    pRouter router = channel->get_router();
	    Direction choice = invalid_dir;

	    int maxscore=-2000;

	    //calculate the score
	    for (unsigned int i=0; i<dirs.size(); i++) {
	        Direction d = dirs[i];
	        pRouter neighbor_router = router->get_router(d);
	        assert(neighbor_router);

		    int candidaterequest_queue=0;
		    int candidateempty_slot=0;
		    int candidatereneighbour_quest_queue0=0;
		    int candidatereneighbour_quest_queue1=0;
		    int score=0;

		    //associate numbers
		    int a=4,b=2,c=1,e=1;

		    //calculate request_queue size
		    candidaterequest_queue=(router->get_arbiter())->get_requestsize(d);
		    //calculate empty slots
	        pIn_port neighbor_in_port = neighbor_router->get_in_port(reverse(d));
	        candidateempty_slot= neighbor_in_port->num_of_empty_buffer_slots();
        	candidatereneighbour_quest_queue0=(neighbor_router->get_arbiter())->get_requestsize(dirs[0]);
        	candidatereneighbour_quest_queue1=(neighbor_router->get_arbiter())->get_requestsize(dirs[1]);

	        //put together
	        score=b*candidateempty_slot-c*candidatereneighbour_quest_queue0-a*candidaterequest_queue-e*candidatereneighbour_quest_queue1;

	        if (score >= maxscore) {
	        	maxscore = score;
	            choice = d;
	        }
	    }
	    assert(choice != invalid_dir);

	    return choice;
}

// Decide the routing direction under Odd-Even routing scheme.
// We use a simple decision method here. We first get the two possible directions if 
// they exists, then choose the next hop whose input channel buffer has more empty space
// left.
Direction
Routing_engine::decide_direction_oe(const Position & src_pos, const Position & dst_pos) const {
    vector<Direction> dirs;
    dirs.clear();

    // annotate power
    if (net_clock.get_clock() > param.warmup_period) {
        // ebit energy models
        param.energy.re_energy += param.oe_routing_engine_epacket;
    }

    int e0 = dst_pos.x - pos.x;
    int e1 = dst_pos.y - pos.y;

    if (e0 == 0 && e1 == 0)
        dirs.push_back(local);
    else {
        if (e0 == 0) {       // currently in the same column as destination
            if (e1 > 0)
                dirs.push_back(north);
            else
                dirs.push_back(south);
        }
        else {
            if (e0 > 0)  {    // east bound messages
                if (e1 == 0)
                    dirs.push_back(east);
                else {
                    if (pos.x%2 == 1 || pos.x == src_pos.x) {
                        if (e1 > 0)
                            dirs.push_back(north);
                        else
                            dirs.push_back(south);
                    }
                    if (dst_pos.x%2 == 1 || e0 != 1)   // odd destinatio column or >=2 columns of destination
                        dirs.push_back(east);
                }
            }
            else {     // west bound messges
                dirs.push_back(west);
                if (pos.x%2 == 0) {
                    if (e1 > 0)
                        dirs.push_back(north);
                    else if (e1 < 0)
                        dirs.push_back(south);
                }
            }
        }
    }

    // now select a direction from dirs vector
    assert(!dirs.empty());

    if (dirs.size() == 1)
        return dirs[0];

    pRouter router = channel->get_router();


    Direction choice = invalid_dir;
    unsigned int max_empty_slots = 0;
    for (unsigned int i=0; i<dirs.size(); i++) {
        Direction d = dirs[i];
        pRouter neighbor_router = router->get_router(d);

        assert(neighbor_router);

        pIn_port neighbor_in_port = neighbor_router->get_in_port(reverse(d));
        unsigned int empty_slots = neighbor_in_port->num_of_empty_buffer_slots();
        if (empty_slots >= max_empty_slots) {
            max_empty_slots = empty_slots;
            choice = d;
        }
    }
    assert(choice != invalid_dir);

    return choice;
}


// In dyad router, we should use oe_fixed instead of XY routing policy when combined with
// OE routing policy. This is because we can when combining XY with OE routing, deadlock may happen.
// OE-fixed is a simple version of OE routing. It has the same routing delay as XY routing, and
// it always select the first legal routing direction from vector<Direction> dirs
Direction 
Routing_engine::decide_direction_oe_fixed(const Position & src_pos, const Position & dst_pos) const {
    vector<Direction> dirs;
    dirs.clear();

    // annotate power
    if (net_clock.get_clock() > param.warmup_period) {
        // ebit energy models
        param.energy.re_energy += param.fixed_routing_engine_epacket;
    }

    int e0 = dst_pos.x - pos.x;
    int e1 = dst_pos.y - pos.y;

    if (e0 == 0 && e1 == 0)
        return local;

    if (e0 == 0) {       // currently in the same column as destination
        if (e1 > 0)
            dirs.push_back(north);
        else
            dirs.push_back(south);
    }
    else {
        if (e0 > 0)  {    // east bound messages
            if (e1 == 0)
                dirs.push_back(east);
            else {
                if (pos.x%2 == 1 || pos.x == src_pos.x) {


                    if (e1 > 0)
                        dirs.push_back(north);
                    else
                        dirs.push_back(south);
                }
                if (dst_pos.x%2 == 1 || e0 != 1)   // odd destinatio column or >=2 columns of destination
                    dirs.push_back(east);
            }
        }
        else {     // west bound messges
            dirs.push_back(west);
            if (pos.x%2 == 0) {
                if (e1 > 0)	 
                    dirs.push_back(north);
                else if (e1 < 0)
                    dirs.push_back(south);
            }
        }
    }

    // now select a direction from dirs vector
    assert(!dirs.empty());

    return dirs[0];
}

/// Predicted routing UYO ///

// Decide the routing direction under Odd-Even routing scheme.
// We use a simple decision method here. We first get the two possible directions if 
// they exists, then choose the next hop whose input channel buffer has more empty space
// left.
Direction 
Routing_engine::decide_direction_predict(const Position & src_pos, const Position & dst_pos) const {
    vector<Direction> dirs;
    dirs.clear();

    // annotate power
    if (net_clock.get_clock() > param.warmup_period) {
        // ebit energy models
        param.energy.re_energy += param.oe_routing_engine_epacket;
    }

    int e0 = dst_pos.x - pos.x;
    int e1 = dst_pos.y - pos.y;

    if (e0 == 0 && e1 == 0)
        dirs.push_back(local);
    else {
        if (e0 == 0) {       // currently in the same column as destination
            if (e1 > 0)
                dirs.push_back(north);
            else
                dirs.push_back(south);
        }
        else {
            if (e0 > 0)  {    // east bound messages
                if (e1 == 0)
                    dirs.push_back(east);
                else {
                    if (pos.x%2 == 1 || pos.x == src_pos.x) {
                        if (e1 > 0)
                            dirs.push_back(north);
                        else
                            dirs.push_back(south);
                    }
                    if (dst_pos.x%2 == 1 || e0 != 1)   // odd destinatio column or >=2 columns of destination
                        dirs.push_back(east);
                }
            }
            else {     // west bound messges
                dirs.push_back(west);
                if (pos.x%2 == 0) {
                    if (e1 > 0)
                        dirs.push_back(north);
                    else if (e1 < 0)
                        dirs.push_back(south);
                }
            }
        }
    }

    // now select a direction from dirs vector
    assert(!dirs.empty());

    if (dirs.size() == 1) 
        return dirs[0];

    pRouter router = channel->get_router();

    Direction choice = invalid_dir;
    unsigned int max_availability = 0;
    for (unsigned int i=0; i<dirs.size(); i++) {
        Direction d = dirs[i];
        pRouter neighbor_router = router->get_router(d);  // Get neighbor router
        assert(neighbor_router);
	pInput_channel neighbor_chan = neighbor_router->get_in_channel(reverse(dir));
	unsigned int availability = neighbor_chan->get_can_accept();
 
	if ( availability >= max_availability ) {
            max_availability = availability;
            choice = d;
        }
    }
    assert(choice != invalid_dir);

    return choice;
}

/// End of predicted routing UYO ///


// Inform the switching fabric scheduler about which output channel it
// wants to connect to
int Routing_engine::send_request_to_arbiter(void) {

	req_time = net_clock.get_clock();
    pRouter router = channel->get_router();
    pArbiter arbiter = router->get_arbiter();

    pSw_connection_request req = new Sw_connection_request;
    req->channel = channel;
    req->dir = dir;
    req->indir=channel->get_port()->get_direction();
    req->timestamp = req_time;
    arbiter->receive_connection_request(req);

	return 0;
}


int Routing_engine::receive_grant_from_arbiter(void) {
    grant_time = net_clock.get_clock();
    return 0;
}


void Routing_engine::timer_fired(void) {
    if (param.extreme_verbose)
        cout << INFO_TIMER_FIRED << " (time = " << net_clock.get_clock() << ")\n";
    send_request_to_arbiter();
}


// Tear down connection for the corresponding channel
void Routing_engine::teardown_connection(void) {
    pRouter router = channel->get_router();
    pArbiter arbiter = router->get_arbiter();
    arbiter->teardown_connection(channel);
}

int Routing_engine::get_delay(void) const {
    Routing_scheme current_routing_scheme = routing_scheme;
    if (routing_scheme == dyad) 
        current_routing_scheme = decide_best_routing_mode();

    if (current_routing_scheme == xy || current_routing_scheme == oe_fixed || current_routing_scheme == rtable) // UYO
        return delay_xy;
    else
        return delay_oe;
}
