/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: arbiter.cpp
 Date Created:  <Tue Oct 14 13:41:01 2003>
 Last Modified: <Mon Dec 20 20:11:46 2004>
 Description: Class arbiter for switch-fabric arbitration.
              We implement an arbiter for a fully connected switch fabric.
              The way we grant connection to inputs are based on FCFS,
              meanwhile, it will always try to connect the input to the
              available channel which has the lowest index.
************************************************************************/

#include "arbiter.hpp"
#include "channel.hpp"
#include "router.hpp"
#include "power.hpp"

Arbiter::Arbiter(int d, int n_sw_p) {
    delay = d;
    num_of_switch_port = n_sw_p;
    numofreqgranted=0;
    numofreqfailed=0;
    content[north]=false;
    content[south]=false;
    content[east]=false;
    content[west]=false;
    request_queue = new vector<struct Sw_connection_request *>[param.n_of_ports];
    for (unsigned int i=0; i<param.n_of_ports; i++) 
        request_queue[i].clear();
}


Arbiter::~Arbiter() {
    for (unsigned int i=0; i<param.n_of_ports; i++) {
        for (vector<pSw_connection_request>::iterator iter=request_queue[i].begin(); 
             iter<request_queue[i].end(); iter++) {
            delete (*iter);
        }
    }
    delete [] request_queue;
}


void Arbiter::tick(void) {
    content[north]=false;
    content[south]=false;
    content[east]=false;
    content[west]=false;
    process_requests();
    update_contention();
}

void Arbiter::update_contention(){
	if(!content[north]) signalcontention(north, false);
	if(!content[west]) signalcontention(west, false);
	if(!content[east]) signalcontention(east, false);
	if(!content[south]) signalcontention(south, false);
}

void Arbiter::process_requests(void) {

    for (unsigned int i=0; i<param.n_of_ports; i++) {
        process_requests_of_port(i); 
    }
}

void Arbiter::signalcontention(int dir, bool cont)
{
	Router * routertoinform;

	if(dir!=north) {
	routertoinform= router->get_router(north);
	if(routertoinform!=0)
	routertoinform->receivecontention(south,dir,cont);
	}
	if(dir!=south) {
	routertoinform= router->get_router(south);
	if(routertoinform!=0)
	routertoinform->receivecontention(north,dir,cont);
	}
	if(dir!=east) {
	routertoinform= router->get_router(east);
	if(routertoinform!=0)
	routertoinform->receivecontention(west,dir,cont);
	}
	if(dir!=west) {
	routertoinform= router->get_router(west);
	if(routertoinform!=0)
	routertoinform->receivecontention(east,dir,cont);
	}
}

bool Arbiter::process_requests_of_port(unsigned int dir) {

    if (request_queue[dir].empty())
        return false;

    pOut_port o_port = router->get_out_port((Direction) dir);
    pOutput_channel a_channel = 0;
    for (unsigned int i=0; i<o_port->get_size(); i++) {
        pOutput_channel tmp_channel = (pOutput_channel) o_port->get_channel(i);

        if (tmp_channel->switch_granted())    // if the output channel is in use already
            continue;
        else if ((!param.arbiter_lookahead) && tmp_channel->get_last_inject_time() + delay >= net_clock.get_clock())
            continue;
        else {
            a_channel = tmp_channel;
            break;
        }
    }

    if (a_channel == 0)
    {
    	//arbiter failed to grant the following requests
    	  for (vector<pSw_connection_request>::iterator iter=request_queue[dir].begin();
    	         iter<request_queue[dir].end(); iter++) {
    	        pSw_connection_request a_req = *iter;
    	        if (param.extreme_verbose)
    	               cout << "[I] arbiter at router "
    	                    << router->get_position() << "failed to grant a connection from inport "
    	                    << a_req->indir<<" to outport "<<a_req->dir<< " at router " << router->get_position()
    	                    << " (time = " << net_clock.get_clock() << ")\n";
    	        if(a_req->dir!=local)
    	        {
					if(content[a_req->dir])
							  signalcontention(a_req->dir,true);
					else content[a_req->dir]=true;
    	        }
    	  }
        return false;
    }

    // first determine how many requests are qualified
    int choices = 0;
    int min_time = INT_MAX;
    // note that the requests in the queue are ordered by time
    for (vector<pSw_connection_request>::iterator iter=request_queue[dir].begin(); 
         iter<request_queue[dir].end(); iter++) {
        pSw_connection_request a_req = *iter;
        if (a_req->timestamp + delay > net_clock.get_clock())
            break;
        if (a_req->timestamp <= min_time) {
            min_time = a_req->timestamp;
            choices ++;
        }
        else
            break;
    }

    if (choices == 0)
        return false;

    //int this_choice = int (drand48() * choices);
    int this_choice = 0;//??

    pSw_connection_request winner = request_queue[dir][this_choice];

    // create the connection
    setup_connection(winner->channel, a_channel);
    numofreqgranted++;
    // send grant signal to routing_engine
    pRouting_engine r = winner->channel->get_routing_engine();
    r->receive_grant_from_arbiter();

    orion_record_arbiter_power(winner->dir);

    if (param.extreme_verbose) 
        cout << "[I] arbiter at router " 
             << router->get_position() << " granted a connection from inport "
             << winner->indir<<" to outport "<<winner->dir<< " at router " << router->get_position()
             << " (time = " << net_clock.get_clock() << ")\n";

    vector<pSw_connection_request>::iterator iter=request_queue[dir].begin();
    request_queue[dir].erase(iter + this_choice);

    delete winner;
    winner = 0;

    return true; 
}


void Arbiter::receive_connection_request(pSw_connection_request req) {
    // process request here
    request_queue[req->dir].push_back(req);
    orion_record_arbiter_power(req->dir);

    // ebit model only need to record once
    if (net_clock.get_clock() > param.warmup_period) 
        param.energy.arbiter_energy += param.arbiter_epacket; 
    
    if (param.extreme_verbose)

        cout << "[I] arbiter at router " 
             << router->get_position() << " receive a connection request " 
             << " (time = " << net_clock.get_clock() << ")\n";
}

// record the arbiter power using orion power model for dir outport. 
// it should be called every time the request vector changes or
// grant signal changes
void Arbiter::orion_record_arbiter_power(Direction dir) {
    if (net_clock.get_clock() > param.warmup_period) {
        if (param.orion_power.use_orion_power_model) {
            pPower_module pm = router->get_power_module();
            assert(pm);
            Atom_type req_vector = form_req_vector(dir);
            unsigned int grant = get_current_granted_channel_dir(dir);
            pm->power_arbiter(dir, req_vector, grant);
        }
    }
}


// return to which inport is the outport granted to
Direction Arbiter::get_current_granted_channel_dir(Direction dir) {
    pOut_port o_port = router->get_out_port(dir);

    // currently we only support one virtual channel (or no virtual channel ;)
    pOutput_channel o_channel = (pOutput_channel) o_port->get_channel(0);
    if (!o_channel->switch_granted())
        return invalid_dir;    // nobody is granted yet

    pSw_output_end swo = o_channel->get_bound_sw_end();
    pSw_input_end swi = (pSw_input_end) swo->get_other_end();
    pInput_channel i_channel = (pInput_channel) swi->get_bound_channel();
    Position tmp_p = i_channel->get_position();
    return (Direction) tmp_p.x;
}


// return a bit map of the channels which request the outport port in 
// the dir direction
Atom_type Arbiter::form_req_vector(Direction dir) const {
    Atom_type req_vector = 0;
    for (vector<pSw_connection_request>::const_iterator iter=request_queue[dir].begin();
         iter<request_queue[dir].end(); iter++) {
        pSw_connection_request areq = *iter;
        if (areq->dir == dir) {
            Position tmp_p = areq->channel->get_position();
            int in_port = tmp_p.x;
            int i = 1;
            for (int j=0; j<in_port; j++) 
                i = i*2;
            req_vector += i;
        }
    }
    return req_vector;
}


// connect the input channel in with output channel out
void Arbiter::setup_connection(pInput_channel in, pOutput_channel out) {

    // make sure that input channel and output channel have connection with other channel
    assert(!in->switch_granted());
    assert(!out->switch_granted());    

    pSw_input_end swi = in->get_bound_sw_end();
    pSw_output_end swo = out->get_bound_sw_end();

    pSwitch_fabric sw = router->get_switch();
    sw->setup_connection(swi, swo);
}


// tear down the connection for the input channel
void Arbiter::teardown_connection(pInput_channel in) {
    assert(in->switch_granted());
    pOutput_channel out = in->get_receiver();
    assert(out->switch_granted());

    out->set_last_inject_time(net_clock.get_clock());

    pSw_input_end swi = in->get_bound_sw_end();
    pSw_output_end swo = out->get_bound_sw_end();

    pSwitch_fabric sw = router->get_switch();
    sw->teardown_connection(swi, swo);
}
