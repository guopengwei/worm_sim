/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: network.cpp
 Date Created:  <Tue Oct 14 14:11:10 2003>
 Last Modified: <Wed Jan 19 12:17:27 2005>
 Description: 
************************************************************************/

#include "network.hpp"
#include "msg.hpp"
#include "util.hpp"
#include <iostream>
#include <algorithm>

//#include "./libperf/libperf.h"

// reset the network 
void Network::reset(void) {
}


// build the network with the parameters given in param
Network::Network(void) {
    success           = true;
    n_of_rows         = param.n_of_rows;
    n_of_cols         = param.n_of_cols;
    n_of_ports        = param.n_of_ports;
    n_of_extra_links  = param.n_of_extra_links; // UYO
    topology          = param.topology;
    n_of_switch_ports = param.n_of_switch_ports;
    n_of_vcs          = param.n_of_vcs;

    Traffic_source::reset_id_base();
    Traffic_sink::reset_id_base();

    if (param.verbose)
        cout << "[I] Building network..." << endl;

    if (topology != mesh) {
        cerr << ERRO_ARCH_NOT_SUPPORTED;
        success = false;
        return;
    }

    if (param.verbose)
        cout << "[I] Creating routers..." << endl;

    Position pos;
    // Build all the routers in the network
    for (pos.x=0; pos.x<param.n_of_cols; pos.x++) {
        for (pos.y=0; pos.y<param.n_of_rows; pos.y++) {
            pRouter a_router = new Router(this, pos);
            routers.push_back(a_router);
        }
    }

    if (param.verbose)
        cout << "[I] Creating traffic sources/sinks..." << endl;

    // Build all the traffic sources/sinks in the network
    for (pos.x=0; pos.x<param.n_of_cols; pos.x++) {
        for (pos.y=0; pos.y<param.n_of_rows; pos.y++) {
            pTraffic_source a_source = new Traffic_source(pos, param.source_buffer_size);
            pTraffic_sink a_sink = new Traffic_sink(pos, param.sink_buffer_size);
            sources.push_back(a_source);
            sinks.push_back(a_sink);
        }
    }

    if (param.verbose)
        cout << "[I] Attaching traffic sources/sinks with routers..." << endl;
    // connect sources and sinks with the router
    for (pos.x=0; pos.x<param.n_of_cols; pos.x++) {
        for (pos.y=0; pos.y<param.n_of_rows; pos.y++) {
            pTraffic_source a_source = get_traffic_source(pos);
            pTraffic_sink a_sink = get_traffic_sink(pos);
            pRouter a_router = get_router(pos);
            pIn_port a_in_port = a_router->get_in_port(local);
            pOut_port a_out_port = a_router->get_out_port(local);
            connect(a_source, a_in_port);
            connect(a_out_port, a_sink);
        }
    }

    if (param.verbose)
        cout << "[I] Connecting routers with links..." << endl;
    // connect the port of each router to the corresponding output port
    // of its neighboring router
    for (pos.x=0; pos.x<param.n_of_cols; pos.x++) {
        for (pos.y=0; pos.y<param.n_of_rows; pos.y++) {
            pRouter src_router = get_router(pos);
            for (unsigned int i=0; i<n_of_ports-n_of_extra_links; i++) { // UYO
                Direction dir = (Direction) i;
                if (dir == local)  // no go for local router
                    continue;
                pRouter dst_router = src_router->get_router(dir);
                if (dst_router == 0)
                    continue;
                pOut_port a_out_port = src_router->get_out_port(dir);
                pIn_port a_in_port = dst_router->get_in_port(reverse(dir));
                connect(a_out_port, a_in_port);
            }
        }
    }

    if (param.verbose)
        cout << "[I] Network successfully built..."<<endl;

    param.network = this;

    // File to dump number of packets as a function of time
    if (param.print_n_of_packets)
      {
	// ofstream packets_t;
        param.packets_t.open(param.file_to_print_n_of_packets);
        if (!param.packets_t.is_open() || !param.packets_t.good()) {
            cerr << "Error in opening file " << param.file_to_print_n_of_packets << " for printing number of packets" << endl;
            exit(-1);
        }
        param.packets_t << "% Simulation length (cycles) \t" << param.simulation_length << endl;
        param.packets_t << "% File printed by worm_sim " << endl;
	param.packets_t <<"% The format of the folloing lines is:" << endl
			<<"% (timestamp)\t" << "(number of packets in the network at timestamp)" << endl;
	param.packets_t <<"A = ["; 

      }
//adjust the starttime and endtime for every ATG
    for(vector<pATG>::iterator iter=param.ATGSETs.sets.begin(); iter<param.ATGSETs.sets.end(); iter++)
    {
    	if((*iter)->time_end==-1) (*iter)->time_end=param.simulation_length;
    	if((*iter)->time_start==-1) (*iter)->time_end=0;
    	(*iter)->running=false;
    }

//set all PEs to be available
    param.ATGSETs.currentnodes=new int[128];
    for(int t=0;t<128;t++)
    param.ATGSETs.currentnodes[t]=1;
}


// return the router in the network which has the given position
pRouter Network::get_router(Position & p) {
    vector<pRouter>::iterator iter = find(routers.begin(), routers.end(), p);
    if (iter == routers.end()) {
        cerr << ERRO_RESOURCE_NOT_FOUND;
        return 0;
    }
    else
        return *iter;
}


// return the traffic source in the network which has the given position
pTraffic_source Network::get_traffic_source(Position & p) {
    vector<pTraffic_source>::iterator iter = find(sources.begin(), sources.end(), p);
    if (iter == sources.end()) {
        cerr << ERRO_RESOURCE_NOT_FOUND;
        return 0;
    }
    else
        return *iter;
}


// return the traffic sink in the network which has the given position
pTraffic_sink Network::get_traffic_sink(Position & p) {
    vector<pTraffic_sink>::iterator iter = find(sinks.begin(), sinks.end(), p);
    if (iter == sinks.end()) {
        cerr << ERRO_RESOURCE_NOT_FOUND;
        return 0;
    }
    else
        return *iter;
}


// destructor of Network class
Network::~Network() {
    if (param.verbose) 
        cout<<"[I] removing the whole network..."<<endl;

    // delete all the links in the network
    for (vector<pRouter>::iterator iter=routers.begin(); iter<routers.end(); iter++) 
        delete *iter;

    // delete all the routers in the network
    for (vector<pLink>::iterator iter=links.begin(); iter<links.end(); iter++)
        delete *iter;

    // delete all the traffic sources/sinks in the network
    for (vector<pTraffic_source>::iterator iter=sources.begin(); iter<sources.end(); iter++) 
        delete *iter;
    for (vector<pTraffic_sink>::iterator iter=sinks.begin(); iter<sinks.end(); iter++) 
        delete *iter;
}


// Create a link which connect src and dst with the given delay (default = 0).
// Return the link which is newly created.
pLink Network::connect(pConnector src, pConnector dst, int delay) {
    pLink link = new Link(src, dst, delay);
    links.push_back(link);
    src->bind_sink(link);
    // cout <<"Link bound with src \n";
    dst->bind_source(link);
    // cout <<"Link bound with dst: " <<delay <<endl;
    return link;
}


void Network::tick(void) {
    // THE ORDER OF THE FIRING WILL AFFECT THE RESULTS.
//     for (vector<pTraffic_sink>::iterator iter=sinks.begin(); iter<sinks.end(); iter++)
//         (*iter)->tick();


    for (vector<pTraffic_sink>::iterator iter=sinks.end()-1; iter>=sinks.begin(); iter--)
      (*iter)->tick();
    if(param.ATGSETs.sets.size()==0)
    	for (vector<pTraffic_source>::iterator iter=sources.begin(); iter<sources.end(); iter++)
    		(*iter)->tick();
    else{
		for(vector<pATG>::iterator iter=param.ATGSETs.sets.begin(); iter<param.ATGSETs.sets.end(); iter++)
		{
			if(net_clock.get_clock()>=((*iter)->time_start+param.warmup_period) && net_clock.get_clock()<=((*iter)->time_end+param.warmup_period))
			{
				bool canload=true;int k=0;

				//check if all PEs it requires are available
				for(vector<int>::iterator itersources=(*iter)->nodes.begin(); itersources<(*iter)->nodes.end(); itersources++)
					if(param.ATGSETs.currentnodes[*itersources]==0)
					{ canload=false;break;}
				//if not available, check if is the running task, if it is currently running, keep running
				if(canload==false)
				for(vector<pATG>::iterator cur_atg_iter=param.ATGSETs.currentATG.begin(); cur_atg_iter<param.ATGSETs.currentATG.end(); cur_atg_iter++)
				{
						if(*cur_atg_iter==*iter) canload=true;
				}
				if(canload)
				for(vector<int>::iterator itersources=(*iter)->nodes.begin(); itersources<(*iter)->nodes.end(); itersources++)
				{
					k=*itersources;
					/*if it is the first time to load the ATG*/
					if(!(*iter)->running)
					{
						param.ATGSETs.currentnodes[*itersources]=0; //take the PE
						param.ATGSETs.currentATG.push_back(*iter);
						(*iter)->running=true;
						for (vector<char*>::iterator f = (*iter)->config_files.begin();//set trace_file
								 f <(*iter)->config_files.end(); f++) {
								if (apply_config_file(*f)) {
									cerr << ERRO_CONFIG_FILE;
									return;
								}
							}
					}
					sources.at(*itersources)->tick();
				}
				if(net_clock.get_clock()==(*iter)->time_end)//release the PEs
				{
					for(vector<int>::iterator itersources=(*iter)->nodes.begin(); itersources<(*iter)->nodes.end(); itersources++)
						param.ATGSETs.currentnodes[*itersources]=1;
					for(vector<pATG>::iterator cur_atg_iter=param.ATGSETs.currentATG.begin(); cur_atg_iter<param.ATGSETs.currentATG.end(); cur_atg_iter++)
					{
									if(*cur_atg_iter==*iter) *cur_atg_iter=0;
					}
						(*iter)->running=false;
				}
			}
		}
    }
    for (vector<pRouter>::iterator iter1=routers.begin(); iter1<routers.end(); iter1++)
      (*iter1)->arbiter_tick();
    for (vector<pRouter>::iterator iter1=routers.begin(); iter1<routers.end(); iter1++)
      (*iter1)->in_ports_tick();
    for (vector<pRouter>::iterator iter1=routers.begin(); iter1<routers.end(); iter1++)
      (*iter1)->out_ports_tick();

    // For the repeaters in the LR links
    for (vector<pRepeater>::iterator iter2=repeaters.begin(); iter2<repeaters.end(); iter2++)  // UYO
      (*iter2)->in_ports_tick();
    for (vector<pRepeater>::iterator iter2=repeaters.begin(); iter2<repeaters.end(); iter2++)  // UYO
      (*iter2)->out_ports_tick();
    // For the Flow control algrithm
    for (vector<pRouter>::iterator iter2=routers.begin(); iter2<routers.end(); iter2++)
      (*iter2)->update_prediction();
}


double Network::get_total_injection_rate(void) const {
    double total = 0;
    for (vector<pTraffic_source>::const_iterator iter=sources.begin(); iter<sources.end(); iter++) {
        total += (*iter)->get_total_packet_injection_rate();
    }
    return total;
}


void Network::calc_channel_load(void) {
    pInput_channel ch = 0;

    // first, clear the load parameters of every input channel
    for (vector<pRouter>::iterator r = routers.begin(); r < routers.end(); r++) {
        for (Direction dir = local; dir < invalid_dir; dir=(Direction) (((int) dir) + 1)) {
            ch = (*r)->get_in_channel(dir);
            ch->reset_load();
        }
    }
    // clear the load parameters of every sinks
    for (vector<pTraffic_sink>::iterator iter=sinks.begin(); iter<sinks.end(); iter++) 
        (*iter)->reset_load();

    // now enumerate all the traffic transactions between any source/destination pairs
    for (unsigned int src=0; src<sources.size(); src++) {
        pTraffic_source pts = sources[src];
        for (unsigned int dst=0; dst<sinks.size(); dst++) {
            if (src == dst) 
                continue;
            double load = pts->get_packet_to_destination_rate(dst);
            if (load == 0)
                continue;
            // now let's update all the input channel load used by this traffic 
            Position src_pos = pts->get_position();
            Position dst_pos = sinks[dst]->get_position();

            // first, fill in the local input channel in the local router
            pRouter r = get_router(src_pos);
            ch = r->get_in_channel(local);
            pRouting_engine re = ch->get_routing_engine();
            Direction out_dir = re->decide_direction(src_pos, dst_pos);
            ch->add_load(load, out_dir);

            // now start following the routing path
            Position cur_pos = src_pos;
            pRouter cur_r = r;
            pInput_channel cur_ch = ch;
            while (cur_pos != dst_pos) {
                re = cur_ch->get_routing_engine();
                out_dir = re->decide_direction(src_pos, dst_pos);
                pRouter next_r = cur_r->get_router(out_dir);
                pInput_channel next_ch = next_r->get_in_channel(reverse(out_dir));
                re = next_ch->get_routing_engine();
                out_dir = re->decide_direction(src_pos, dst_pos);
                next_ch->add_load(load, out_dir);

                cur_r = next_r;
                cur_ch = next_ch;
                cur_pos = cur_r->get_position();
            }

            assert(cur_pos == dst_pos);
            assert(out_dir == local);
            // finally, we need to update the traffic sinks load
            sinks[dst]->add_load(load);

        }
    }

}

// Dump the equations to a file which matlab can call later for performance analysis
// Return true if success
bool Network::dump_equation_file(void) {
    if (param.verbose)
        cout << "[I] preparing equation file for fsolve..." << endl;

    if (topology != mesh || n_of_vcs != 1 || param.flits_per_packet != 1
        || (param.routing_scheme != xy && param.routing_scheme != oe_fixed)) {
        cerr << ERRO_EQUATION_FILE;
        return false;
    }

    
    pInput_channel ch = 0;

    if (param.verbose)
        cout << "[I] collecting equation related data..." << endl;

    calc_channel_load();    

    if (param.verbose) 
        cout << "[I] generating equation file..." << endl;

    ofstream eqf;
    eqf.open(param.equation_file);
 
    if (!eqf.is_open() || !eqf.good()) {
        cerr << ERRO_OPEN_FILE;
        return false;
    }

    eqf << "function y = equations(mu) " << endl;
    eqf << "%% function automatically generated by worm_sim \n" 
        << "%% notation: \n"
        << "%% mu(i,j,k)     service rate of k-th input channel at router (i,j)\n"
        << "%% ro(i,j,k)     utilization of k-th input channel at router (i,j)\n"
        << "%% lambda(i,j,k)  load of k-th input channel at router (i,j)\n"
        << "%% mu_eff(i,j,k) effective service rate of k-th input channel at (i,j)\n"
        << "%%               as observed by its upstream input channel buffers\n"
        << "%% mu_tmp(i,j,k) service rate of k-th input channel without taking into\n"
        << "%%               consideration the channel's physical service rate\n"
        << "%% b(i,j,k)      buffer depth (in flits) for k-th input channel at (i,j)\n"
        << "\n\n\n";

    eqf << "%% once you get the mu of each channel, you can estimate the latency indication\n"
        << "%% by calculating sum(lambda(i,j,k) / (mu(i,j,k) - lambda(i,j,k))).\n";
    eqf << "%% This value gives you an indication of the packet latency, and the real\n"
        << "%% estimated value should be devided by the sum of all the packets injection\n"
        << "%% rate.\n\n";

    eqf << "%% buffer depth will be passed from external scripts as global variables\n";
    eqf << "global b;\n\n";

    eqf << "%% lambda variable will also be used by external scripts, so make it global\n";
    eqf << "global lambda;\n\n";

    eqf << "%% lambda (load) values: " << endl;
    for (unsigned int i=0; i<routers.size(); i++) {
      for (Direction dir = local; dir < invalid_dir - param.n_of_extra_links;   // TODO: ugly extra links
             dir=(Direction) (((int) dir) + 1)) {
            ch = routers[i]->get_in_channel(dir, 0);
            eqf << "lambda";
            eqf << (*ch) << " = " << ch->get_load() << ";\n";
        }
    }
    eqf << endl << endl;

    eqf << "%% ro value calculation: " << endl;
    for (unsigned int i=0; i<routers.size(); i++) {
      for (Direction dir = local; dir < invalid_dir - param.n_of_extra_links;    // TODO: ugly extra links
             dir=(Direction) (((int) dir) + 1)) {
            ch = routers[i]->get_in_channel(dir, 0);
            eqf << "ro";
            eqf << (*ch);
            if (ch->get_load() == 0) 
                eqf << " = 0;\n";
            else {
                eqf << " = lambda";
                eqf << (*ch) << " / mu";
                eqf << (*ch) << ";\n";
            }
        }
    }
    eqf << endl << endl;

    eqf << "%% mu_eff (effective service rate seen by upstream input buffers) calculation: " << endl;
    for (unsigned int i=0; i<routers.size(); i++) {
      for (Direction dir = local; dir < invalid_dir - param.n_of_extra_links;    // TODO: ugly extra links
             dir=(Direction) (((int) dir) + 1)) {
            ch = routers[i]->get_in_channel(dir, 0);
            eqf << "mu_eff";
            eqf << (*ch);
            if (dir == local)
                // local input buffer has effective service rate of 1, although useless
                eqf << " = 1;\n";
            else {
                eqf << " = (1 - ro";
                eqf << (*ch) << " ^ b";
                eqf << (*ch) << ") / (1 - ro";
                eqf << (*ch) << " ^ (b";
                eqf << (*ch) << " + 1));\n";
             }
        }
    }
    eqf << endl << endl;

    eqf << "%% equations describing mu_eff and mu_tmp: " << endl;
    for (unsigned int i=0; i<routers.size(); i++) {
      for (Direction dir = local; dir < invalid_dir - param.n_of_extra_links;     // TODO: ugly extra links
             dir=(Direction) (((int) dir) + 1)) {
            ch = routers[i]->get_in_channel(dir);
            eqf << "mu_tmp";
            eqf << (*ch) << " = ";
            double load = ch->get_load();
            if (load == 0) {
                eqf << "1;\n";
            }
            else {
                bool begin = true;
                for (Direction out_dir = local; out_dir < invalid_dir; 
                     out_dir=(Direction) ((int) out_dir + 1)) {
                    double dir_load = ch->get_load(out_dir);
                    if (dir_load == 0)
                        continue;
                    else {
                        double prob = dir_load / load;
                        assert(prob <= 1);
                        if (begin)
                            eqf << prob << " * ";
                        else
                            eqf << " + " << prob << " * ";
                        if (out_dir == local) {
                            // if output to local, we can take the downstream
                            // input buffer's effective service rate as 1 since
                            // we assume infinite local buffer
                            double tmp = 1 - sinks[i]->get_load() + dir_load;
                            assert(sinks[i]->get_load() >= dir_load);
                            eqf << tmp;
                        }
                        else {
                            pRouter next_r = routers[i]->get_router(out_dir);
                            pInput_channel next_ch = next_r->get_in_channel(reverse(out_dir));
                            eqf << "(mu_eff";
                            eqf << (*next_ch) << " - " << next_ch->get_load() - dir_load << ")";
                            assert(next_ch->get_load() >= dir_load);
                        }
                        begin = false;
                    }
                }
                eqf << ";\n";
            }
        }
    }

    eqf << endl << endl;

    eqf << "%% equations describing mu_tmp and mu: " << endl;
    for (unsigned int i=0; i<routers.size(); i++) {
      for (Direction dir = local; dir < invalid_dir - param.n_of_extra_links;  // TODO: ugly extra links
             dir=(Direction) (((int) dir) + 1)) {
            ch = routers[i]->get_in_channel(dir);
            eqf << "y";
            eqf << (*ch) << " = 1 / (mu";
            eqf << (*ch) << " - lambda";
            eqf << (*ch) << ") - 1 / (" << ch->get_service_rate() << " - lambda";
            eqf << (*ch) << ") - 1 / (mu_tmp";
            eqf << (*ch) << " - lambda";
            eqf << (*ch) << ");\n";
        }
    }


    // estimate if the equation file has solution by check whether if all the
    // load is less than 1. 
    for (vector<pRouter>::iterator r = routers.begin(); r < routers.end(); r++) {
      for (Direction dir = local; dir < invalid_dir - param.n_of_extra_links; // TODO: ugly extra links
             dir=(Direction) (((int) dir) + 1)) {
            ch = (*r)->get_in_channel(dir);
            double load = ch->get_load();
            if (load >= ch->get_service_rate()) {
                cerr << WARN_EQN_NOT_SOLVABLE;
                // we also put this warning into equation file
                eqf << "%% PRELIMINARY CHECK BY WORM_SIM SHOW " << WARN_EQN_NOT_SOLVABLE;
                break;
            }
        }
    }
    for (vector<pTraffic_sink>::iterator iter=sinks.begin(); iter<sinks.end(); iter++) {
        double load = (*iter)->get_load();
        if (load >= 1) {
            cerr << WARN_EQN_NOT_SOLVABLE;
            // we also put this warning into equation file
            eqf << "%% PRELIMINARY CHECK BY WORM_SIM SHOW " << WARN_EQN_NOT_SOLVABLE;
            break;
        }
    }

    eqf.close();
    return true;
}


// Dump the equations to a file which matlab compiler late can use to build our 
// performance analysis library
// Return true if success
bool Network::dump_perf_anal_lib_equation_file(void) {
    if (param.verbose)
        cout << "[I] preparing equation file for building latency analysis lib..." << endl;

    if (topology != mesh || n_of_vcs != 1 || param.flits_per_packet != 1
        || (param.routing_scheme != xy && param.routing_scheme != oe_fixed)) {
        cerr << ERRO_EQUATION_FILE;
        return false;
    }

    
    pInput_channel ch = 0;

    if (param.verbose)
        cout << "[I] collecting equation related data..." << endl;

    calc_channel_load();    

    if (param.verbose) 
        cout << "[I] generating equation file..." << endl;

    ofstream eqf;
    eqf.open(param.perf_anal_lib_equation_file);
 
    if (!eqf.is_open() || !eqf.good()) {
        cerr << ERRO_OPEN_FILE;
        return false;
    }

    eqf << "function y = equations(mu) " << endl;
    eqf << "%% function automatically generated by worm_sim \n" 
        << "%% notation: \n"
        << "%% mu(i,j,k)     service rate of k-th input channel at router (i,j)\n"
        << "%% ro(i,j,k)     utilization of k-th input channel at router (i,j)\n"
        << "%% lambda(i,j,k)  load of k-th input channel at router (i,j)\n"
        << "%% mu_eff(i,j,k) effective service rate of k-th input channel at (i,j)\n"
        << "%%               as observed by its upstream input channel buffers\n"
        << "%% mu_tmp(i,j,k) service rate of k-th input channel without taking into\n"
        << "%%               consideration the channel's physical service rate\n"
        << "%% b(i,j,k)      buffer depth (in flits) for k-th input channel at (i,j)\n"
        << "\n\n\n";

    eqf << "%% once you get the mu of each channel, you can estimate the latency indication\n"
        << "%% by calculating sum(lambda(i,j,k) / (mu(i,j,k) - lambda(i,j,k))).\n";
    eqf << "%% This value gives you an indication of the packet latency, and the real\n"
        << "%% estimated value should be devided by the sum of all the packets injection\n"
        << "%% rate.\n\n";

    eqf << "%% buffer depth will be passed from external scripts as global variables\n";
    eqf << "global b;\n\n";

    eqf << "%% we allow service rate to be specified by user\n";
    eqf << "global S;\n\n";

    eqf << "%% lambda variable will also be used by external scripts, so make it global\n";
    eqf << "global lambda;\n\n";

    eqf << "global ro;" << endl << "global mu_eff;" << endl;

    eqf << "%% prob(x,y,d1,d2) is the probability of a packet in channel (x,y,d1) to go to \n" 
        << "%% d2 direction. If channel(x,y,d1) is 0, assign it to be 0 too.\n";
    eqf << "global prob;" << endl;

    eqf << "%% ro value calculation: " << endl;
    for (unsigned int i=0; i<routers.size(); i++) {
        for (Direction dir = local; dir < invalid_dir; dir=(Direction) (((int) dir) + 1)) {
            ch = routers[i]->get_in_channel(dir, 0);
            eqf << "ro";
            eqf << (*ch);
            if (ch->get_port()->get_source()==0)
                eqf << " = 0;\n";
            else {
                eqf << " = lambda";
                eqf << (*ch) << " / mu";
                eqf << (*ch) << ";\n";
            }
        }
    }
    eqf << endl << endl;

    eqf << "%% mu_eff (effective service rate seen by upstream input buffers) calculation: " << endl;
    for (unsigned int i=0; i<routers.size(); i++) {
        for (Direction dir = local; dir < invalid_dir; dir=(Direction) (((int) dir) + 1)) {
            ch = routers[i]->get_in_channel(dir, 0);
            eqf << "mu_eff";
            eqf << (*ch);
            if (dir == local)
                // local input buffer has effective service rate of 1
                eqf << " = 1;" << endl;
            else if (ch->get_port()->get_source() == 0)
                eqf << " = 0;" << endl;   // for channels at the border
            else {
                eqf << " = 1 / S * (1 - ro";
                eqf << (*ch) << " ^ b";
                eqf << (*ch) << ") / (1 - ro";
                eqf << (*ch) << " ^ (b";
                eqf << (*ch) << " + 1));\n";
             }
        }
    }
    eqf << endl << endl;

    eqf << "%% equations describing mu_eff and mu_tmp: " << endl;
    for (unsigned int i=0; i<routers.size(); i++) {
        for (Direction dir = local; dir < invalid_dir; dir=(Direction) (((int) dir) + 1)) {
            ch = routers[i]->get_in_channel(dir);
            eqf << "mu_tmp";
            eqf << (*ch) << " = ";
            if (ch->get_port()->get_source() == 0) {    // these are channels at the border
                eqf << "1;\n";
            }
            else {
                bool begin = true;
                for (Direction out_dir = local; out_dir < invalid_dir; 
                     out_dir=(Direction) ((int) out_dir + 1)) {
                    Position pos = routers[i]->get_position();
                    if (begin) 
                        eqf << "prob(" << pos.x+1 << "," << pos.y+1 << "," 
                            << dir+1 << "," << out_dir+1 << ")" << " * ";
                    else
                        eqf << "+" << "prob(" << pos.x+1 << "," << pos.y+1 << "," 
                            << dir+1 << "," << out_dir+1 << ")" << " * ";
                    if (out_dir == local) {
                        // here is how we represent 1-sinks[i]->get_load()+dir_load
                        eqf << " (1 ";
                        for (Direction tmp_d=local; tmp_d<invalid_dir; tmp_d=(Direction) (((int) tmp_d) + 1)) {
                            if (tmp_d == local)  
                                continue;     // no self loop
                            pInput_channel tmp_ch = routers[i]->get_in_channel(tmp_d);
                            if (tmp_ch->get_port()->get_source() == 0)
                                continue;
                            eqf << " - lambda";
                            eqf << (*tmp_ch) << " * " << "prob(" << pos.x+1
                                << "," << pos.y+1 << "," << tmp_d+1 << "," << local+1 << ")";
                        }
                        eqf << " + lambda";
                        eqf << (*ch) << " * " << "prob(" << pos.x+1 << "," << pos.y+1 << "," 
                            << dir+1 << "," << out_dir+1 << "))";
                    }
                    else {
                        pRouter next_r = routers[i]->get_router(out_dir);
                        if (next_r == 0) {
                            eqf << " 0 ";
                            continue;
                        }
                        pInput_channel next_ch = next_r->get_in_channel(reverse(out_dir));
                        eqf << " (mu_eff";
                        eqf << (*next_ch) << " - lambda";
                        eqf << (*next_ch) << " + lambda";
                        eqf << (*ch) << " * " << "prob(" << pos.x+1 << "," << pos.y+1 << "," 
                            << dir+1 << "," << out_dir+1 << "))";
                    }
                    begin = false;
                }
                eqf << ";\n"; 
            }
        }
    }
    eqf << endl << endl;

    eqf << "%% equations describing mu_tmp and mu: " << endl;
    for (unsigned int i=0; i<routers.size(); i++) {
        for (Direction dir = local; dir < invalid_dir; dir=(Direction) (((int) dir) + 1)) {
            ch = routers[i]->get_in_channel(dir);
            eqf << "y";
            eqf << (*ch) << " = mu";
            eqf << (*ch) << " - mu_tmp";
            eqf << (*ch) << ";\n";
        }
    }

    // this is needed to calculate the queueing time of each packet
    // waiting time in queue calculation
    eqf << "%% waiting time in queue calculation: " << endl;
    eqf << "global W;\n";
    for (unsigned int i=0; i<routers.size(); i++) {
        for (Direction dir = local; dir < invalid_dir; dir=(Direction) (((int) dir) + 1)) {
            ch = routers[i]->get_in_channel(dir);
            eqf << "ES2";
            eqf << (*ch) << " = ";
            if (ch->get_port()->get_source() == 0) {    // these are channels at the border
                eqf << "1";
            }
            else {
                bool begin = true;
                for (Direction out_dir = local; out_dir < invalid_dir; 
                     out_dir=(Direction) ((int) out_dir + 1)) {
                    Position pos = routers[i]->get_position();
                    if (dir == out_dir || routers[i]->get_router(out_dir) == 0)
                        continue;
                    if (!begin) 
                        eqf << " + ";
                    // use this if we model it as M/M/1
                    eqf << "prob(" << pos.x+1 << "," << pos.y+1 << "," 
                        << dir+1 << "," << out_dir+1 << ") * 2 / ((1";

                    // use this if we model it as M/D/1
//                     eqf << "prob(" << pos.x+1 << "," << pos.y+1 << "," 
//                         << dir+1 << "," << out_dir+1 << ") / ((1";

                    for (Direction tmp_d=local; tmp_d<invalid_dir; tmp_d=(Direction) (((int) tmp_d) + 1)) {
                        pInput_channel tmp_ch = routers[i]->get_in_channel(tmp_d);
                        if (tmp_ch->get_port()->get_source() == 0)
                            continue;
                        if (dir == tmp_d) 
                            continue;
                        eqf << " - lambda";
                        eqf << (*tmp_ch) << " * prob(" << pos.x+1
                            << "," << pos.y+1 << "," << tmp_d+1 << "," << out_dir+1 << ")";
                    }
                    eqf << ") * ";
                    if (out_dir == local) 
                        eqf << "1";
                    else {
                        pRouter next_r = routers[i]->get_router(out_dir);
                        pInput_channel next_ch = next_r->get_in_channel(reverse(out_dir));
                        eqf << "mu_eff";
                        eqf << (*next_ch);
                    }
                    eqf << ")^2";
                    begin = false;
                }
            }
            eqf << ";\n";
            eqf << "W";
            eqf << (*ch) << " = ";
            if (ch->get_port()->get_source() == 0) {    // these are channels at the border
                eqf << "0";
            }
            else {
                eqf << "lambda";
                eqf << (*ch) << " * ES2 ";
                eqf << (*ch) << " / 2 / (1-ro";
                eqf << (*ch) << ")";
                for (Direction out_dir = local; out_dir < invalid_dir; 
                     out_dir=(Direction) ((int) out_dir + 1)) {
                    Position pos = routers[i]->get_position();
                    if (dir == out_dir || routers[i]->get_router(out_dir) == 0)
                        continue; 
                    eqf << " + prob(" << pos.x+1 << "," << pos.y+1 << "," 
                        << dir+1 << "," << out_dir+1 << ") / ((1";
                    for (Direction tmp_d=local; tmp_d<invalid_dir; tmp_d=(Direction) (((int) tmp_d) + 1)) {
                        pInput_channel tmp_ch = routers[i]->get_in_channel(tmp_d);
                        if (tmp_ch->get_port()->get_source() == 0)
                            continue;
                        if (dir == tmp_d) 
                            continue;
                        eqf << " - lambda";
                        eqf << (*tmp_ch) << " * prob(" << pos.x+1
                            << "," << pos.y+1 << "," << tmp_d+1 << "," << out_dir+1 << ")";
                    }
                    eqf << ") * ";
                    if (out_dir == local) 
                        eqf << "1";
                    else {
                        pRouter next_r = routers[i]->get_router(out_dir);
                        pInput_channel next_ch = next_r->get_in_channel(reverse(out_dir));
                        eqf << "mu_eff";
                        eqf << (*next_ch);
                    }
                    eqf << ")";
                }
            }
            eqf << ";\n";
        }
    }
    eqf << endl << endl;

    eqf.close();
    return true;
}


// dump the config of the network into a file. This includes the packet injection 
// rate, the buffer sizes, etc. 
bool Network::dump_config_file(void) {
    if (param.verbose) 
        cout << "[I] generating network config file to " << param.config_file_to_dump << "...\n";

    ofstream conf;
    conf.open(param.config_file_to_dump);

    if (!conf.is_open() || !conf.good()) {
        cerr << ERRO_OPEN_FILE;
        return false;
    }

    conf << "# Network config file dumped by worm_sim" << endl
         << "# Traffic configurations" << endl;

    for (vector<pTraffic_source>::const_iterator iter=sources.begin(); iter<sources.end(); iter++) {
        conf << "@NODE\t" << (*iter)->get_id() << endl;
        for (unsigned int i=0; i<sinks.size(); i++) {
            double rate = (*iter)->get_packet_to_destination_rate(i);
            conf << "packet_to_destination_rate\t" << i << "\t" << rate << endl;
        }
    }

    conf.close();
    return true;
}


bool Network::overloaded(double * n_lambda) {
    double service_rate = routers[0]->get_in_channel(north)->get_service_rate();
    double local_service_rate = 1.;
    int id = 0;
    for (unsigned int d=0; d<param.n_of_ports; d++) {
        for (unsigned int y=0; y<param.n_of_rows; y++) {  
            for (unsigned int x=0; x<param.n_of_cols; x++) {
                double s_rate = service_rate;
                if (d == local) 
                    s_rate = local_service_rate;
                if (n_lambda[id++] >= s_rate)
                    return true;
            }
        }
    }
    return false;
}


/*
// analyze the lantecy of the current network configuration through calling the 
double Network::analyze_latency(void) {

    double latency = -1.;

    double * n_lambda = new double[param.n_of_rows*param.n_of_cols*param.n_of_ports];
    double * n_prob = new double[param.n_of_rows*param.n_of_cols*param.n_of_ports*param.n_of_ports];

    calc_channel_load();

    perf_anal_init_lambdas(n_lambda);
    perf_anal_init_probs(n_prob);

    if (overloaded(n_lambda)) {
        delete [] n_lambda;
        delete [] n_prob;
        return -1.;
    }

    // Note the sequence of initialize lambda. The matlab matrix representation
    // is reversed from C/CPP
    int id = 0;
    Position pos;
    for (unsigned int d=0; d<param.n_of_ports; d++) {
        for (unsigned int y=0; y<param.n_of_rows; y++) {  
            for (unsigned int x=0; x<param.n_of_cols; x++) {
                pos.x = x; 
                pos.y = y;
                pRouter router = get_router(pos);
                pInput_channel pch = router->get_in_channel((Direction) d);
                n_lambda[id++] = pch->get_load();
            }
        }
    }

    // Call application and library initialization. Perform this
    // initialization before calling any API functions or
    // Compiler-generated libraries.
    if (!mclInitializeApplication(NULL,0) || !libperfInitialize()) {
        std::cerr << "could not initialize the library properly"
                  << std::endl;
        return -1;
    }

    try {
        // Create input data
        double service_rate = routers[0]->get_in_channel(north)->get_service_rate();
        // you may want to use other service_rate, such as 0.33 here
        //double service_rate = 0.33;
        double n_service_rate[] = {service_rate};

        double n_depth[] = {param.in_channel_buffer_size};    // buffer depth for each channel

        cout << "[I] using service rate " << service_rate << endl;

        mwArray in0(1, 1, mxDOUBLE_CLASS, mxREAL);
        mwArray in1(1, 1, mxDOUBLE_CLASS, mxREAL);

        int in2_dims[3] = {param.n_of_cols, param.n_of_rows, param.n_of_ports};
        mwArray in2(3, in2_dims, mxDOUBLE_CLASS, mxREAL);

        int in3_dims[4] = {param.n_of_cols, param.n_of_rows, param.n_of_ports, param.n_of_ports};
        mwArray in3(4, in3_dims, mxDOUBLE_CLASS, mxREAL);
        
        in0.SetData(n_service_rate, 1);
        in1.SetData(n_depth, 1);
        in2.SetData(n_lambda, param.n_of_rows*param.n_of_cols*param.n_of_ports);
        in3.SetData(n_prob, param.n_of_rows*param.n_of_cols*param.n_of_ports*param.n_of_ports);

        // output paramters
        mwArray out_latency, out_block_prob;

        perf_analysis(2, out_latency, out_block_prob, in0, in1, in2, in3);

        latency = out_latency.Get(1,1);

        // this shows how you can retrieve the specific element in the return array
//         cout << "out_block_prob = " << out_block_prob << endl;

//         // display the blocking probability of channel (0,2,1)
//         double tmp_p = out_block_prob.Get(3, 0+1, 2+1, 1+1);
//         cout << "tmp_p = " << tmp_p << endl;

    }
    catch (const mwException& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    catch (...) {
        std::cerr << "Unexpected error thrown" << std::endl;
        return -1;
    }

    // Call the application and library termination routine
    libperfTerminate();

    mclTerminateApplication();

    delete [] n_lambda;
    delete [] n_prob;

    // we need to divide latency with the total injection rate
    latency = latency / get_total_injection_rate();

    return latency;
}


// initialize the lambda matrix for performance analysis. lambda(x,y,dir) is the 
// load of the channel dir at router (x,y)
void Network::perf_anal_init_lambdas(double * n_lambda) {

    // Note the sequence of initialize lambda. The matlab matrix representation
    // is reversed from C/CPP
    int id = 0;
    Position pos;
    for (unsigned int d=0; d<param.n_of_ports; d++) {
        for (unsigned int y=0; y<param.n_of_rows; y++) {  
            for (unsigned int x=0; x<param.n_of_cols; x++) {
                pos.x = x; 
                pos.y = y;
                pRouter router = get_router(pos);
                pInput_channel pch = router->get_in_channel((Direction) d);
                n_lambda[id++] = pch->get_load();
            }
        }
    }
}


// initialize the probability matrix for performance analysis. prob(x,y,d1,d2) gives
// the probability of a packet at channel (x,y,d1) to be delivered to d2 direction
void Network::perf_anal_init_probs(double * n_prob) {
    // Note the sequence of initialize lambda. The matlab matrix representation
    // is reversed from C/CPP
    int id = 0;
    Position pos;
    for (unsigned int d2=0; d2<param.n_of_ports; d2++) {
        for (unsigned int d1=0; d1<param.n_of_ports; d1++) {
            for (unsigned int y=0; y<param.n_of_rows; y++) {  
                for (unsigned int x=0; x<param.n_of_cols; x++) {
                    pos.x = x; 
                    pos.y = y;
                    pRouter router = get_router(pos);
                    pInput_channel ch = router->get_in_channel((Direction) d1);
                    double load = ch->get_load();
                    if (load == 0) {
                        n_prob[id++] = 1;
                        continue;
                    }
                    double dir_load = ch->get_load((Direction) d2);
                    n_prob[id++] = dir_load/load;
                }
            }
        }
    }
}

*/
