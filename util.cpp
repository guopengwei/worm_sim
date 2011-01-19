/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: util.cpp
 Date Created:  <Sun Oct 19 19:16:26 2003>
 Last Modified: <Wed Dec 07 15:44:01 2005>
 Description: Dec 07, 2005 now parses the on-off traffic source info from a config file
************************************************************************/

#include "util.hpp"
#include "version.hpp"
#include "router.hpp"
#include "repeater.hpp"   // UYO
#include "msg.hpp"
#include <iomanip>
#include <algorithm>

using namespace std;

// given the input string, rip off its empty space and the table key
string rip_off_space_and_table(string & s) {
  int i = 0;
  while(i < s.length())
    if (s[i] == ' ' || s[i] == '\r' || s[i] == '\n')
      s.erase(i, 1);
    else
      i++;

//     unsigned int token;
//     while ((token = s.find(" ")) != string::npos)
//         s.erase(token, strlen(" "));
//     while ((token = s.find("\t")) != string::npos)
//         s.erase(token, strlen("\t"));
    return s;
}

// Print the time used for the simulation
void print_times(clock_t real, struct tms *tmsstart, struct tms *tmsend) {
    static long clktck = 0;
    if (clktck == 0) {
        if ((clktck = sysconf(_SC_CLK_TCK)) < 0) 
            cerr << "sysconf error" << endl;
    }
    cout << "Used time for this simulation:" << endl;
    cout << "\tReal:\t" << real/((double) clktck) << endl;
    cout << "\tUser:\t" << (tmsend->tms_utime - tmsstart->tms_utime)/(double) clktck << endl;
    cout << "\tSys:\t" << (tmsend->tms_stime-tmsstart->tms_stime)/(double) clktck << endl;
    cout << "\tCPU:\t"
         << (tmsend->tms_utime-tmsstart->tms_utime+tmsend->tms_stime-tmsstart->tms_stime)/(double) clktck
         << endl;
}

void print_statistics(Network & network) {
    param.avg_latency = param.total_latency / param.n_of_received_packets;
    param.ave_gen_delay = (double) param.gen_delay / param.n_of_sent_packets;

    compile_energy_info(network);

    cout << endl << endl; 
    cout << "============================================================" << endl;
    cout << "Performance report for the period of [" << param.warmup_period << ", " 
         << param.simulation_length << "]: \n" << endl;
    cout << "Total sent packets number:         " << setw(8) << param.n_of_sent_packets << endl;
    cout << "Total received packets number:     " << setw(8) << param.n_of_received_packets << endl;
    cout << "Average latency for a packet:      " << setw(8) << param.avg_latency << endl << endl;
    cout << "Average src delay for a packet:      " << setw(8) << param.ave_gen_delay << " " <<  param.gen_delay <<endl;
    cout << "Average number of packets at t:    " << setw(8) << param.n_t <<endl;
    cout << "Maximum number of packets:         " << setw(8) << param.max_n_of_packets <<endl;
    cout << "Power report use ebit model (J): \n" << endl;
 

    cout.setf(ios::scientific);

    cout << "Link energy consumption:           " << setw(8) << param.energy.link_energy << endl;
    cout << "Crossbar energy consumption:       " << setw(8) << param.energy.xbar_energy << endl;
    cout << "Routing engine energy consumption: " << setw(8) << param.energy.re_energy << endl;
    cout << "Arbiter energy consumption:        " << setw(8) << param.energy.arbiter_energy << endl;
    cout << "Buffer read energy consumption:    " << setw(8) << param.energy.buf_read_energy << endl;
    cout << "Buffer write energy consumption:   " << setw(8) << param.energy.buf_write_energy << endl;
    cout << "Total network energy consumption:  " << setw(8) << param.energy.total_energy << endl;

    if (param.orion_power.use_orion_power_model) {
        cout << endl;
        cout << "Power report use Orion model (J): \n" << endl;
        cout << "Link energy consumption:           " << setw(8) 
             << param.orion_power.link_energy << endl;
        cout << "Crossbar energy consumption:       " << setw(8) 
             << param.orion_power.xbar_energy << endl;
        cout << "Routing engine energy consumption: " << setw(8) 
             << "N/A" << endl;
        cout << "Arbiter energy consumption:        " << setw(8)
             << param.orion_power.arbiter_energy << endl;
        cout << "Buffer energy consumption:         " << setw(8) 
             << param.orion_power.buf_energy << endl;
        cout << "Total network energy consumption:  " << setw(8) 
             << param.orion_power.total_energy << endl;
    }

    cout.unsetf(ios::scientific);

    cout << "============================================================" << endl;
    cout << endl << endl;

    if(param.extreme_verbose){
    	cerr << endl << endl;
    	    cerr << "============================================================" << endl;
    	    cerr << "Performance report for the period of [" << param.warmup_period << ", "
    	         << param.simulation_length << "]: \n" << endl;
    	    cerr << "Total sent packets number:         " << setw(8) << param.n_of_sent_packets << endl;
    	    cerr << "Total received packets number:     " << setw(8) << param.n_of_received_packets << endl;
    	    cerr << "Average latency for a packet:      " << setw(8) << param.avg_latency << endl << endl;
    	    cerr << "Average src delay for a packet:      " << setw(8) << param.ave_gen_delay << " " <<  param.gen_delay <<endl;
    	    cerr << "Average number of packets at t:    " << setw(8) << param.n_t <<endl;
    	    cerr << "Maximum number of packets:         " << setw(8) << param.max_n_of_packets <<endl;
    	    cerr << "Power report use ebit model (J): \n" << endl;


    	    cerr.setf(ios::scientific);

    	    cerr << "Link energy consumption:           " << setw(8) << param.energy.link_energy << endl;
    	    cerr << "Crossbar energy consumption:       " << setw(8) << param.energy.xbar_energy << endl;
    	    cerr << "Routing engine energy consumption: " << setw(8) << param.energy.re_energy << endl;
    	    cerr << "Arbiter energy consumption:        " << setw(8) << param.energy.arbiter_energy << endl;
    	    cerr << "Buffer read energy consumption:    " << setw(8) << param.energy.buf_read_energy << endl;
    	    cerr << "Buffer write energy consumption:   " << setw(8) << param.energy.buf_write_energy << endl;
    	    cerr << "Total network energy consumption:  " << setw(8) << param.energy.total_energy << endl;

    	    if (param.orion_power.use_orion_power_model) {
    	        cerr << endl;
    	        cerr << "Power report use Orion model (J): \n" << endl;
    	        cerr << "Link energy consumption:           " << setw(8)
    	             << param.orion_power.link_energy << endl;
    	        cerr << "Crossbar energy consumption:       " << setw(8)
    	             << param.orion_power.xbar_energy << endl;
    	        cerr << "Routing engine energy consumption: " << setw(8)
    	             << "N/A" << endl;
    	        cerr << "Arbiter energy consumption:        " << setw(8)
    	             << param.orion_power.arbiter_energy << endl;
    	        cerr << "Buffer energy consumption:         " << setw(8)
    	             << param.orion_power.buf_energy << endl;
    	        cerr << "Total network energy consumption:  " << setw(8)
    	             << param.orion_power.total_energy << endl;
    	    }

    	    cerr.unsetf(ios::scientific);

    	    cerr << "============================================================" << endl;
    	    cerr << endl << endl;
    }
}


void compile_energy_info(Network & network) {
    if (param.orion_power.use_orion_power_model) {
        param.orion_power.link_energy = 0;
        param.orion_power.buf_energy = 0;
        param.orion_power.re_energy = 0;
        param.orion_power.arbiter_energy = 0;
        param.orion_power.xbar_energy = 0;

        int nrouters = network.get_num_of_rows() * network.get_num_of_cols();
        for(int i=0; i<nrouters; i++) {
            pRouter r = network.get_router(i);
            pPower_module pm = r->get_power_module();
            param.orion_power.link_energy += pm->power_link_report();
            param.orion_power.buf_energy += pm->power_buffer_report();
            //            param.orion_power.re_energy += pm->power_arbiter_report();
            param.orion_power.arbiter_energy += pm->power_arbiter_report();
            param.orion_power.xbar_energy += pm->power_crossbar_report();
        }
        param.orion_power.total_energy = param.orion_power.link_energy 
            + param.orion_power.buf_energy + param.orion_power.re_energy 
            + param.orion_power.arbiter_energy + param.orion_power.xbar_energy;

    } 

    param.energy.total_energy = param.energy.link_energy + param.energy.xbar_energy 
        + param.energy.re_energy + param.energy.arbiter_energy
        + param.energy.buf_read_energy + param.energy.buf_write_energy;

    return;
}

void print_parameters(void) {
    cout << endl;
    cout << "Parameters used in this simulation: " << endl;

    cout << "Configuration files        = ";
    if (param.config_files.size()) {
        for (vector<char*>::iterator f = param.config_files.begin();
             f < param.config_files.end(); f++) 
            cout << *f << "," << endl;
    }
    else
        cout << "nil" << endl;

    cout << "Network size               = (" << param.n_of_rows << "x" 
         << param.n_of_cols << ")" << "\t"
         << "Routing scheme             = " << param.get_routing_scheme_in_string() << endl;

    cout << "Input channel buffer size  = " << std::setiosflags(std::ios::left) 
         << setw(2) << param.in_channel_buffer_size << "\t\t" 
         << "Output channel buffer size = " << std::setiosflags(std::ios::left) 
         << setw(2) << param.out_channel_buffer_size << endl;

    cout << "Routing engine delay       = " << std::setiosflags(std::ios::left) 
         << setw(2) << param.routing_engine_delay_xy << "\t\t"
         << "Routing engine delay OE    = " << std::setiosflags(std::ios::left) 
         << setw(2) << param.routing_engine_delay_oe << endl;

    cout << "Arbitration delay          = " << std::setiosflags(std::ios::left) 
         << setw(2) << param.arbitration_delay << "\t\t"
         << "Traffic mode               = " << param.get_traffic_mode_in_string() 
         << endl;

    if (param.traffic_mode == hotspot) {
        cout << "Hotspot percentage         = " << std::setiosflags(std::ios::left) 
             << setw(2) << param.hotspot_percentage << "\t\t"
             << "Hostspots                  = ";
        for (vector<unsigned int>::iterator iter = param.hotspots.begin();
             iter < param.hotspots.end(); iter++ ) 
            cout << *iter << " ";
        cout << endl;
    }

    cout << "Packet generation rate     = " << std::setiosflags(std::ios::left) 
         << setw(8) << param.packet_generating_rate << "\t"
         << "Number of flits per packet = " << std::setiosflags(std::ios::left) 
         << setw(8) << param.flits_per_packet << endl;

    cout << "Simulation length          = " << std::setiosflags(std::ios::left) 
         << setw(8) << param.simulation_length << "\t"
         << "Warm up period             = " << std::setiosflags(std::ios::left)
         << setw(8) << param.warmup_period << endl;

    cout << "Mode switch threshold      = " << std::setiosflags(std::ios::left) 
         << setw(8) << param.switch_mode_threshold << "\t" 
	 << "LR link adaptive routing   = " << std::setiosflags(std::ios::left) 
         << setw(8) << param.adaptive_LR1 << endl;

    cout << "On_off completion time     = " << std::setiosflags(std::ios::left) 
         << setw(8) << param.on_off_complete_time << "\t"
         << "Flow controller            = " << std::setiosflags(std::ios::left) 
         << setw(8) << param.flow_control << endl;

    if(param.extreme_verbose){
        cerr << endl;
        cerr << "Parameters used in this simulation: " << endl;

        cerr << "Configuration files        = ";
        if (param.config_files.size()) {
            for (vector<char*>::iterator f = param.config_files.begin();
                 f < param.config_files.end(); f++)
                cerr << *f << "," << endl;
        }
        else
            cerr << "nil" << endl;

        cerr << "Network size               = (" << param.n_of_rows << "x"
             << param.n_of_cols << ")" << "\t"
             << "Routing scheme             = " << param.get_routing_scheme_in_string() << endl;

        cerr << "Input channel buffer size  = " << std::setiosflags(std::ios::left)
             << setw(2) << param.in_channel_buffer_size << "\t\t"
             << "Output channel buffer size = " << std::setiosflags(std::ios::left)
             << setw(2) << param.out_channel_buffer_size << endl;

        cerr << "Routing engine delay       = " << std::setiosflags(std::ios::left)
             << setw(2) << param.routing_engine_delay_xy << "\t\t"
             << "Routing engine delay OE    = " << std::setiosflags(std::ios::left)
             << setw(2) << param.routing_engine_delay_oe << endl;

        cerr << "Arbitration delay          = " << std::setiosflags(std::ios::left)
             << setw(2) << param.arbitration_delay << "\t\t"
             << "Traffic mode               = " << param.get_traffic_mode_in_string()
             << endl;

        if (param.traffic_mode == hotspot) {
            cerr << "Hotspot percentage         = " << std::setiosflags(std::ios::left)
                 << setw(2) << param.hotspot_percentage << "\t\t"
                 << "Hostspots                  = ";
            for (vector<unsigned int>::iterator iter = param.hotspots.begin();
                 iter < param.hotspots.end(); iter++ )
                cerr << *iter << " ";
            cerr << endl;
        }

        cerr << "Packet generation rate     = " << std::setiosflags(std::ios::left)
             << setw(8) << param.packet_generating_rate << "\t"
             << "Number of flits per packet = " << std::setiosflags(std::ios::left)
             << setw(8) << param.flits_per_packet << endl;

        cerr << "Simulation length          = " << std::setiosflags(std::ios::left)
             << setw(8) << param.simulation_length << "\t"
             << "Warm up period             = " << std::setiosflags(std::ios::left)
             << setw(8) << param.warmup_period << endl;

        cerr << "Mode switch threshold      = " << std::setiosflags(std::ios::left)
             << setw(8) << param.switch_mode_threshold << "\t"
    	 << "LR link adaptive routing   = " << std::setiosflags(std::ios::left)
             << setw(8) << param.adaptive_LR1 << endl;

        cerr << "On_off completion time     = " << std::setiosflags(std::ios::left)
             << setw(8) << param.on_off_complete_time << "\t"
             << "Flow controller            = " << std::setiosflags(std::ios::left)
             << setw(8) << param.flow_control << endl;
    }
}

void print_help(void) {
    cout << "Usage: " << endl;

    cout << "-h:" << endl;
    cout << "\tprint out this help." << endl;

    cout << "-v:" << endl;
    cout << "\trun in verbose mode." << endl;

    cout << "-ev:" << endl;
    cout << "\trun in extremely verbose mode." << endl;

    cout << "-t:" << endl;
    cout << "\tprint out CPU time in running this simulation." << endl;

    cout << "-V:" << endl;
    cout << "\tdisplay version of this program." << endl;

    cout << "-config string1 string2 ...:" << endl;
    cout << "\tstring gives the configuration file(s) where you can specify each router's property."
         << endl;

    cout << "-simulation_length int:" << endl;
    cout << "\tsetup the simulation length (default = " << param.simulation_length 
         << ")." << endl;

    cout << "-warmup_period int:" << endl;
    cout << "\tsetup the warmup period within which performance data will not be collected." << endl
         << "\t(default = " << param.warmup_period << ")." << endl;

    cout << "-random_seed int:" << endl;
    cout << "\tsetup the seed for the random number generator." << endl
         << "\t(default = " << param.rand_seed << ")." << endl;

    cout << "-num_of_rows int:" << endl;
    cout << "\tsetup the row size of the network under simulation (default = "
         << param.n_of_rows << ")." << endl;

    cout << "-num_of_columns int:" << endl;
    cout << "\tsetup the column size of the network under simulation (default = "
         << param.n_of_cols << ")." << endl;

    cout << "-input_channel_buffer_size int:" << endl;
    cout << "\tsetup the buffer size for the input channels of the network (default = "
         << param.in_channel_buffer_size << ")." << endl;

    cout << "-output_channel_buffer_size int:" << endl;
    cout << "\tsetup the buffer size for the output channels of the network (default = "
         << param.out_channel_buffer_size << ")." << endl;

    cout << "-source_buffer_size int:" << endl;
    cout << "\tsetup the buffer size for the sources of the network (default = "
         << param.source_buffer_size << ")." << endl;

    cout << "-sink_buffer_size int:" << endl;
    cout << "\tsetup the buffer size for the sinks of the network (default = "
         << param.sink_buffer_size << ")." << endl;

    cout << "-arbitration_delay int:" << endl;
    cout << "\tsetup the arbitration delay for the switching fabric (default = "
         << param.arbitration_delay << ")." << endl;

    cout << "-routing_scheme xy|oe|oe_fixed|dyad|predict:" << endl;
    cout << "\tsetup what routing scheme is to be used for the network (default = "
         << param.get_routing_scheme_in_string() << ")." << endl;

    cout << "-traffic_mode uniform|transpose1|transpose2|hotspot:" << endl;
    cout << "\tsetup what traffic pattern to be used for network simulation (default = "
         << param.get_traffic_mode_in_string() << ")." << endl;

    cout << "-hotspot_percentage double:" << endl;
    cout << "\tA newly generated message is directed to each hot spot node with an additional "
         << "\t'double' percent probability (default = " << param.hotspot_percentage << ")." 
         << endl;

    cout << "-hotspots int int ...:" << endl;
    cout << "\tA list of integers which gives the hotspot nodes in the network "
         << "(default = one center node)." << endl;

    cout << "-routing_engine_delay_xy int:" << endl;
    cout << "\tsetup the routing engine delay for XY and OE-fixed routing engines (default = "
         << param.routing_engine_delay_xy << ")." << endl;

    cout << "-routing_engine_delay_oe int:" << endl;
    cout << "\tsetup the routing engine delay for OE and predicted routing engines (default = "
         << param.routing_engine_delay_oe << ")." << endl;

    cout << "-switch_mode_threshold double:" << endl;
    cout << "\tsetup the threshold for routing mode switch (default = " 
         << param.switch_mode_threshold << ")." << endl;

    cout << "-link_ebit double:" << endl;
    cout << "\tsetup the energy consumption per bit for link (default = "
         << param.link_ebit << ")." << endl;

    cout << "-xbar_ebit double:" << endl;
    cout << "\tsetup the energy cosumption per bit for switch fabric (default = "
         << param.xbar_ebit << ")." << endl;

    cout << "-fixed_routing_engine_epacket double:" << endl;
    cout << "\tsetup the energy consumption per packet for non-adaptive routing engine (default = "
         << param.fixed_routing_engine_epacket << ")." << endl;

    cout << "-oe_routing_engine_epacket double:" << endl;
    cout << "\tsetup the energy consumption per packet for oe routing engine (default = "
         << param.oe_routing_engine_epacket << ")." << endl;

    cout << "-arbiter_epacket double:" << endl;
    cout << "\tsetup the energy consumption per packet for arbiter request (default = "
         << param.arbiter_epacket << ")." << endl;

    cout << "-buffer_read_ebit double:" << endl;
    cout << "\tsetup the energy consumption per bit for buffer reading (default = "
         << param.buf_read_ebit << ")." << endl;

    cout << "-buffer_write_ebit double:" << endl;
    cout << "\tsetup the energy consumption per bit for buffer writing (default = "
         << param.buf_write_ebit << ")." << endl;

    cout << "-use_orion_power_model:" << endl;
    cout << "\tuse orion power model to derive energy consumption (default = "
         << param.orion_power.use_orion_power_model << ")." << endl;

    cout << "-flits_per_packet int:" << endl;
    cout << "\tsetup how many flits each packet contains (default = "
         << param.flits_per_packet << ")." << endl;

    cout << "-packet_generating_rate double:" << endl;
    cout << "\tsetup on average how many packet to be generated per cycle (default = "
         << param.packet_generating_rate << ")." << endl;

//     cout << "-analyze_latency:" << endl;
//     cout << "\tapply performance analysis on the network and display the estimated latency." 
//          << endl;

    cout << "-flow_control:" << endl;
    cout << "\tActivate the prediction-based flow controller" << endl;

    cout << "-dump_equations string:" << endl;
    cout << "\tdump the equations to the file specified by the string for Matlab fsolve" << endl;

    cout << "-dump_perf_anal_lib_equations string:" << endl;
    cout << "\tdump the equations to the file specified by the string for Matlab to compile " 
         << "our latency analysis library." << endl;

    cout << "-dump_config string:" << endl;
    cout << "\tdump the configuration of the network to the file specified by the string." << endl;

    cout << "-dump_traffic_source_trace:" << endl;
    cout << "\tdump the traffic source trace (with name of 0.trace -- 15.trace assuming a 4x4 NoC)" <<endl;
    
    cout << "-dump_input_buffer:" << endl;
    cout << "\tdump the occupancy for each input buffer" << endl;

    cout << "-dump_output_buffer:" << endl;
    cout << "\tdump the occupancy for each output buffer" << endl;
}

// Print buffer occupanciess
bool print_input_buffer_occupancy(Network & network) {
  cout << "Printing the buffer occupancy for each input buffer \n";
    
  Position pos;
  for (pos.x=0; pos.x<param.n_of_cols; pos.x++) {
    for (pos.y=0; pos.y<param.n_of_rows; pos.y++) {
	
      // Print source buffers first
      pTraffic_source a_source = network.get_traffic_source(pos);
      if (a_source->src_monitoring) {
        char file_name[100];
        sprintf(file_name, "wormsim_data/src__%d_%d.m",pos.x, pos.y);
        ofstream src_buffer_dump;
        src_buffer_dump.open(file_name);
        if (!src_buffer_dump.is_open() || !src_buffer_dump.good()) {
          cerr << "Error in opening file " << file_name << " for buffer dumping" << endl;
          cerr << "Buffer occupancy has not been able to be saved in a file" << endl;
          return false;
        }

        src_buffer_dump << "%pkt_id seq_id head=1/body=2/tail=3 timestamp received=1/sent=0/consumed=2 src_x src_y dst_x dst_y buf_size\n" << endl;
     //   src_buffer_dump << "src_buffer_data= [\n";
        unsigned int len = a_source->buffer_monitor.size();
        if(param.extreme_verbose)
               	cerr<<"At ("<<pos.x<<","<<pos.y<<") total packet sent:"<<a_source->get_total_pkt_sent()<<", vector size:"<<len<<"\n";
               cout<<"At ("<<pos.x<<","<<pos.y<<") total packet sent:"<<a_source->get_total_pkt_sent()<<", vector size:"<<len<<"\n";
        for (unsigned int i=0; i<len; i=i+10) {
          src_buffer_dump << a_source->buffer_monitor[i]   << " ";   //pkt_id
          src_buffer_dump << a_source->buffer_monitor[i+1] << " ";   //pkt->flit->seq_id
          src_buffer_dump << a_source->buffer_monitor[i+2] << " ";   //head/body/tail(pkt->flit)
          src_buffer_dump << a_source->buffer_monitor[i+3] << " ";   //timestamp
          src_buffer_dump << a_source->buffer_monitor[i+4] << " ";   //received = 1/sent = 0
          src_buffer_dump << a_source->buffer_monitor[i+5] << " ";   //pkt->src_x
          src_buffer_dump << a_source->buffer_monitor[i+6] << " ";   //pkt->src_y
          src_buffer_dump << a_source->buffer_monitor[i+7] << " ";   //pkt->dst_x
          src_buffer_dump << a_source->buffer_monitor[i+8] << " ";   //pkt->dst_y
          src_buffer_dump << a_source->buffer_monitor[i+9] << endl;  //buffer size
        }
  //      src_buffer_dump <<"];\n";
      }
   
      // Next print router buffers
      pRouter a_router = network.get_router(pos);
      if (a_router->get_monitor()) {
        unsigned int curr_port=0;
        unsigned int num_of_ports=a_router->get_num_of_ports();
        
        for (curr_port=0; curr_port<num_of_ports; curr_port++) {
          pChannel a_channel=a_router->get_in_channel((Direction) curr_port,0);

          //begin rtg
          char file_name[100];
          ofstream buffer_dump;
          sprintf(file_name, "wormsim_data/b__router_%d_%d__inport_%d.m",pos.x,pos.y,curr_port);
          buffer_dump.open(file_name);
          if (!buffer_dump.is_open() || !buffer_dump.good() ) {
            cerr << "Error in opening file " << file_name << " for buffer dumping" << endl;
            cerr << "channel 0 of inport " << curr_port << " at router " << pos << " has not been dumped" << endl;
            return false;
          }

          buffer_dump << "%pkt_id seq_id head=1/body=2/tail=3 timestamp received=1/sent=0/consumed=2 src_x src_y dst_x dst_y buf_size\n" << endl;
      //    buffer_dump << "buffer__router_" << pos.x << "_" << pos.y << "__inport_" << curr_port << "__data= [\n";
          unsigned int len = a_channel->buffer_monitor.size();
          for (unsigned int i=0; i<len; i=i+10) {
            buffer_dump << a_channel->buffer_monitor[i]   << " ";   //pkt_id
            buffer_dump << a_channel->buffer_monitor[i+1] << " ";   //pkt->flit->seq_id
            buffer_dump << a_channel->buffer_monitor[i+2] << " ";   //head = 1/body = 2/tail = 3
            buffer_dump << a_channel->buffer_monitor[i+3] << " ";   //timestamp
            buffer_dump << a_channel->buffer_monitor[i+4] << " ";   //received = 1/sent = 0
            buffer_dump << a_channel->buffer_monitor[i+5] << " ";   //pkt->src_x
            buffer_dump << a_channel->buffer_monitor[i+6] << " ";   //pkt->src_y
            buffer_dump << a_channel->buffer_monitor[i+7] << " ";   //pkt->dst_x
            buffer_dump << a_channel->buffer_monitor[i+8] << " ";   //pkt->dst_y
            buffer_dump << a_channel->buffer_monitor[i+9] << endl;  //buffer size
          }
    //      buffer_dump <<"];\n";
          buffer_dump.close();
          //end rtg
        }  //for vector,pChannel...
      }   // if
    }    // pos.y
  }     // pos.x
  
  return 1;
} // main


bool print_output_buffer_occupancy(Network & network) {
  cout << "Printing the buffer occupancy for each output buffer \n";
    
  Position pos;
  for (pos.x=0; pos.x<param.n_of_cols; pos.x++) {
    for (pos.y=0; pos.y<param.n_of_rows; pos.y++) {
	
      // Print sink buffers first
      pTraffic_sink a_sink = network.get_traffic_sink(pos);
      if (a_sink->monitoring) {
        char file_name[100];
        sprintf(file_name, "wormsim_data/sink__%d_%d.m",pos.x, pos.y);
        ofstream sink_buffer_dump;
        sink_buffer_dump.open(file_name);
        if (!sink_buffer_dump.is_open() || !sink_buffer_dump.good()) {
          cerr << "Error in opening file " << file_name << " for buffer dumping" << endl;
          cerr << "Buffer occupancy has not been able to be saved in a file" << endl;
          return false;
        }

        sink_buffer_dump << "%pkt_id seq_id head=1/body=2/tail=3 timestamp received=1/sent=0/consumed=2 src_x src_y dst_x dst_y buf_size\n" << endl;
   //     sink_buffer_dump << "sink__" << pos.x << "_" << pos.y << "_buffer_data= [\n";
        unsigned int len = a_sink->buffer_monitor.size();
        if(param.extreme_verbose)
        	cerr<<"At ("<<pos.x<<","<<pos.y<<") total packet consumed:"<<param.n_of_received_packets<<", vector size:"<<len<<"\n";
        cout<<"At ("<<pos.x<<","<<pos.y<<") total packet consumed:"<<param.n_of_received_packets<<", vector size:"<<len<<"\n";
        for (unsigned int i=0; i<len; i=i+10) {
          sink_buffer_dump << a_sink->buffer_monitor[i]   << " ";   //pkt_id
          sink_buffer_dump << a_sink->buffer_monitor[i+1] << " ";   //pkt->flit->seq_id
          sink_buffer_dump << a_sink->buffer_monitor[i+2] << " ";   //head/body/tail(pkt->flit)
          sink_buffer_dump << a_sink->buffer_monitor[i+3] << " ";   //timestamp
          sink_buffer_dump << a_sink->buffer_monitor[i+4] << " ";   //received = 1/sent = 0/consumed = 2
          sink_buffer_dump << a_sink->buffer_monitor[i+5] << " ";   //pkt->src_x
          sink_buffer_dump << a_sink->buffer_monitor[i+6] << " ";   //pkt->src_y
          sink_buffer_dump << a_sink->buffer_monitor[i+7] << " ";   //pkt->dst_x
          sink_buffer_dump << a_sink->buffer_monitor[i+8] << " ";   //pkt->dst_y
          sink_buffer_dump << a_sink->buffer_monitor[i+9] << endl;  //buffer size
        }
   //     sink_buffer_dump <<"];\n";
      }
   
      // Next print router buffers
      pRouter a_router = network.get_router(pos);
      if (a_router->get_monitor()) {
        unsigned int curr_port=0;
        unsigned int num_of_ports=a_router->get_num_of_ports();
        
        for (curr_port=0; curr_port<num_of_ports; curr_port++) {
          pChannel a_channel=a_router->get_out_channel((Direction) curr_port,0);

          //begin rtg
          char file_name[100];
          ofstream buffer_dump;
          sprintf(file_name, "wormsim_data/b__router_%d_%d__outport_%d.m",pos.x,pos.y,curr_port);
          buffer_dump.open(file_name);
          if (!buffer_dump.is_open() || !buffer_dump.good() ) {
            cerr << "Error in opening file " << file_name << " for buffer dumping" << endl;
            cerr << "channel 0 of outport " << curr_port << " at router " << pos << " has not been dumped" << endl;
            return false;
          }

          buffer_dump << "%pkt_id seq_id head=1/body=2/tail=3 timestamp received=1/sent=0/consumed=2 src_x src_y dst_x dst_y buf_size\n" << endl;
   //       buffer_dump << "buffer__router_" << pos.x << "_" << pos.y << "__outport_" << curr_port << "__data= [\n";
          unsigned int len = a_channel->buffer_monitor.size();
          for (unsigned int i=0; i<len; i=i+10) {
            string action = "received";
            if (a_channel->buffer_monitor[i+4] == 0)
              action = "sent";

            buffer_dump << a_channel->buffer_monitor[i]   << " ";   //pkt_id
            buffer_dump << a_channel->buffer_monitor[i+1] << " ";   //pkt->flit->seq_id
            buffer_dump << a_channel->buffer_monitor[i+2] << " ";   //head/body/tail(pkt->flit)
            buffer_dump << a_channel->buffer_monitor[i+3] << " ";   //timestamp
            buffer_dump << a_channel->buffer_monitor[i+4] << " ";   //received = 1/sent = 0
            buffer_dump << a_channel->buffer_monitor[i+5] << " ";   //pkt->src_x
            buffer_dump << a_channel->buffer_monitor[i+6] << " ";   //pkt->src_y
            buffer_dump << a_channel->buffer_monitor[i+7] << " ";   //pkt->dst_x
            buffer_dump << a_channel->buffer_monitor[i+8] << " ";   //pkt->dst_y
            buffer_dump << a_channel->buffer_monitor[i+9] << endl;  //buffer size
          }
  //        buffer_dump <<"];\n";
          buffer_dump.close();
          //end rtg
        }  //for vector,pChannel...
      }   // if
    }    // pos.y
  }     // pos.x
  
  return 1;
} // main


// return true if error in the parsing
bool parse_options(int argn, char ** argv) {

    if (argn == 1) {    // no arguments, then print help and quit
        print_help();
        param.quit_flag = true;
        return true;
    }

    for (int i=1; i<argn; i++) {

        if (strcmp(argv[i], "-simulation_length") == 0) {
            if (!sscanf(argv[++i], "%d", &param.simulation_length)) {
                param.parse_error = true;
                return true;
            }
            continue;
        }
//Daniel Wei modified to allow dynamic loading
        if (strcmp(argv[i], "-config") == 0) {

        	pATG p_newATG=new ATG;
        	(p_newATG)->time_start=-1;
        	(p_newATG)->time_end=-1;
            while (++i < argn) {
                if (argv[i][0] == '-') {
                    --i;
                    break; 
                }
                if(argv[i][0]== 't') {
                	sscanf(argv[++i], "%ld", &(p_newATG)->time_start);
                	if(argv[++i][0]== 't')
                	sscanf(argv[++i], "%ld", &(p_newATG)->time_end);
                	else
                	{
                	   --i;
                	}
                	param.ATGSETs.sets.push_back(p_newATG);
                	p_newATG=new ATG;
                	(p_newATG)->time_start=-1;
                	(p_newATG)->time_end=-1;
                	continue;
                }
//parse the config file first to get positions the ATG is going to use
                ifstream in_file(argv[i]);
                   if (!in_file.is_open() || !in_file.good())
                       return -1;

                   int id = -1;
                   char input_line[MAX_LINE];
                   char * sub_string;

                while (in_file.getline(input_line, MAX_LINE, '\n')) {
                       // line starting with "#" are comments
                       if (input_line[0] == '#')
                           continue;

                       if (strstr(input_line, "@NODE")) {
                           sub_string = strstr(input_line, "@NODE");
                           sub_string += strlen("@NODE");

                           if (!sscanf(sub_string, "%d", &id))
                               return -1;

                           (p_newATG)->nodes.push_back(id);

                           continue;
                       }
                       continue;
                }

                (p_newATG)->config_files.push_back(argv[i]);
                param.config_files.push_back(argv[i]);
            }
            continue;
        }

        if (strcmp(argv[i], "-random_seed") == 0) {
            if (!sscanf(argv[++i], "%ld", &param.rand_seed)) {
                param.parse_error = true;
                return true;
            }
            continue;
        }

        if (strcmp(argv[i], "-warmup_period") == 0) {
            if (!sscanf(argv[++i], "%d", &param.warmup_period)) {
                param.parse_error = true;
                return true;
            }
            continue;
        }

        if (strcmp(argv[i], "-num_of_rows") == 0) {
            if (!sscanf(argv[++i], "%d", &param.n_of_rows)) {
                param.parse_error = true;
                return true;
            }
            continue;
        }

        if (strcmp(argv[i], "-num_of_columns") == 0) {
            if (!sscanf(argv[++i], "%d", &param.n_of_cols)) {
                param.parse_error = true;
                return true;
            }
            continue;
        }

        if (strcmp(argv[i], "-input_channel_buffer_size") == 0) {
            if (!sscanf(argv[++i], "%d", &param.in_channel_buffer_size)) {
                param.parse_error = true;
                return true;
            }
            continue;
        }

        if (strcmp(argv[i], "-output_channel_buffer_size") == 0) {
            if (!sscanf(argv[++i], "%d", &param.out_channel_buffer_size)) {
                param.parse_error = true;
                return true;
            }
            continue;
        }

	// UYO: source and sink buffer sizes
	if (strcmp(argv[i], "-source_buffer_size") == 0) {
	  if (!sscanf(argv[++i], "%d", &param.source_buffer_size)) {
	    param.parse_error = true;
	    return true;
	  }
	  continue;
        }
	
	if (strcmp(argv[i], "-sink_buffer_size") == 0) {
	  if (!sscanf(argv[++i], "%d", &param.sink_buffer_size)) {
	    param.parse_error = true;
	    return true;
	  }
	  param.packet_consuming_rate = 0.5;   // UYO if finite sink buffer, finite consuming rate
	  continue;
        }

	// UYO: Flow controller setup
	if (strcmp(argv[i], "-flow_control") == 0) {
	  if (!sscanf(argv[++i], "%d", &param.availability_thresh)) {
	    param.parse_error = true;
	  return true;
	  }
	  param.flow_control = 1;
	  continue;
        }

	if (strcmp(argv[i], "-look_ahead") == 0) {
	  if (!sscanf(argv[++i], "%d", &param.look_ahead)) {
	    param.parse_error = true;
	    return true;
	  }
	  continue;
        }

	if (strcmp(argv[i], "-print_n_of_packets") == 0) {
	    param.print_n_of_packets = true;
	    param.file_to_print_n_of_packets = argv[++i];
	    continue;
        }

        if (strcmp(argv[i], "-arbitration_delay") == 0) {
            if (!sscanf(argv[++i], "%d", &param.arbitration_delay)) {
                param.parse_error = true;
                return true;
            }
            if (param.arbitration_delay < 1) {
                cerr << "[E] minimum allowed arbitration_delay is 1 " << endl;
                param.parse_error = true;
                return true;
            }
            continue;
        }

        if (strcmp(argv[i], "-flits_per_packet") == 0) {
            if (!sscanf(argv[++i], "%d", &param.flits_per_packet)) {
                param.parse_error = true;
                return true;
            }
            continue;
        }

        if (strcmp(argv[i], "-routing_scheme") == 0) {
            i ++;
            if (strcmp(argv[i], "xy") == 0) 
                param.routing_scheme = xy;
            else if (strcmp(argv[i], "oe") == 0)
                param.routing_scheme = oe;
            else if (strcmp(argv[i], "test") == 0)
                            param.routing_scheme = test;
            else if (strcmp(argv[i], "oe_fixed") == 0)
                param.routing_scheme = oe_fixed;
            else if (strcmp(argv[i], "dyad") == 0)
                param.routing_scheme = dyad;
	    else if (strcmp(argv[i], "predict") == 0)
                param.routing_scheme = predict;
            else {
                param.parse_error = true;
                return true;
            }
            continue;
        }

        if (strcmp(argv[i], "-traffic_mode") == 0) {
            i ++;
            if (strcmp(argv[i], "uniform") == 0) 
                param.traffic_mode = uniform;
            else if (strcmp(argv[i], "transpose1") == 0)
                param.traffic_mode = transpose1;
            else if (strcmp(argv[i], "transpose2") == 0)
                param.traffic_mode = transpose2;
            else if (strcmp(argv[i], "hotspot") == 0)
                param.traffic_mode = hotspot;
            else {
                param.parse_error = true;
                return true;
            }
            continue;
        }

        if (strcmp(argv[i], "-hotspot_percentage") == 0) {
            if (!sscanf(argv[++i], "%lf", &param.hotspot_percentage)) {
                param.parse_error = true;
                return true;
            }
            continue;
        }

        if (strcmp(argv[i], "-hotspots") == 0) {
            unsigned int a_spot = 0;
            while (++i < argn) { 
                if (strstr(argv[i], "-")) {
                    --i;
                    break; 
                }
                if (!sscanf(argv[i], "%d", &a_spot)) {
                    param.parse_error = true;
                    return true;
                }
                param.hotspots.push_back(a_spot);
            }
            continue;
        }

        if (strcmp(argv[i], "-routing_engine_delay_xy") == 0) {
            if (!sscanf(argv[++i], "%d", &param.routing_engine_delay_xy)) {
                param.parse_error = true;
                return true;
            }
            continue;
        }

        if (strcmp(argv[i], "-routing_engine_delay_oe") == 0) {
            if (!sscanf(argv[++i], "%d", &param.routing_engine_delay_oe)) {
                param.parse_error = true;
                return true;
            }
            continue;
        }

        if (strcmp(argv[i], "-packet_generating_rate") == 0) {
            if (!sscanf(argv[++i], "%lf", &param.packet_generating_rate)) {
                param.parse_error = true;
                return true;
            }
            continue;
        }

        if (strcmp(argv[i], "-link_ebit") == 0) {
            if (!sscanf(argv[++i], "%lf", &param.link_ebit)) {
                param.parse_error = true;
                return true;
            }
            continue;
        }

        if (strcmp(argv[i], "-xbar_ebit") == 0) {
            if (!sscanf(argv[++i], "%lf", &param.xbar_ebit)) {
                param.parse_error = true;
                return true;
            }
            continue;
        }

        if (strcmp(argv[i], "-fixed_routing_engine_epacket") == 0) {
            if (!sscanf(argv[++i], "%lf", &param.fixed_routing_engine_epacket)) {
                param.parse_error = true;
                return true;
            }
            continue;
        }

        if (strcmp(argv[i], "-oe_routing_engine_epacket") == 0) {
            if (!sscanf(argv[++i], "%lf", &param.oe_routing_engine_epacket)) {
                param.parse_error = true;
                return true;
            }
            continue;
        }

        if (strcmp(argv[i], "-arbiter_epacket") == 0) {
            if (!sscanf(argv[++i], "%lf", &param.arbiter_epacket)) {
                param.parse_error = true;
                return true;
            }
            continue;
        }

        if (strcmp(argv[i], "-buffer_read_ebit") == 0) {
            if (!sscanf(argv[++i], "%lf", &param.buf_read_ebit)) {
                param.parse_error = true;
                return true;
            }
            continue;
        }

        if (strcmp(argv[i], "-buffer_write_ebit") == 0) {
            if (!sscanf(argv[++i], "%lf", &param.buf_write_ebit)) {
                param.parse_error = true;
                return true;
            }
            continue;
        }

        if (strcmp(argv[i], "-use_orion_power_model") == 0) {
            param.orion_power.use_orion_power_model = true;
            continue;
        }

//         if (strcmp(argv[i], "-analyze_latency") == 0) {
//             param.analyze_latency = true;
//             continue;
//         }

        if (strcmp(argv[i], "-dump_equations") == 0) {
            param.equation_file = argv[++i];
            continue;
        }

        if (strcmp(argv[i], "-dump_perf_anal_lib_equations") == 0) {
            param.perf_anal_lib_equation_file = argv[++i];
            continue;
        }

        if (strcmp(argv[i], "-dump_config") == 0) {
            param.config_file_to_dump = argv[++i];
            continue;
        }

        if (strcmp(argv[i], "-dump_traffic_source_trace") == 0) {
            param.dump_traffic_source_trace = true;
            continue;
        }

        if (strcmp(argv[i], "-h") == 0) {
            print_help();
            param.quit_flag = true;
            continue;
        }

        if (strcmp(argv[i], "-v") == 0) {
            param.verbose = true;
            if (argn == 2) 
                param.quit_flag = true;
            continue;
        }

        if (strcmp(argv[i], "-ev") == 0) {

            param.verbose = true;
            param.extreme_verbose = true;
            if (argn == 2) 
                param.quit_flag = true;
            continue;
        }

        if (strcmp(argv[i], "-V") == 0) {
            cout << argv[0] << " version " << VERSION << endl;
            if (argn == 2) 
                param.quit_flag = true;
            continue;
        }

        if (strcmp(argv[i], "-t") == 0) {
            param.print_simulation_time = true;
            continue;
        }

	// Dump buffer utilizations
	if (strcmp(argv[i], "-dump_input_buffer") == 0) {
	  param.dump_input_buffer=1;
	  continue;
        }

	if (strcmp(argv[i], "-dump_output_buffer") == 0) {
	  param.dump_output_buffer=1;
	  continue;
	}

	if (strcmp(argv[i], "-redirect_output") == 0) {
		  std::cout.rdbuf(param.logFile.rdbuf());
		  continue;
	}

        cerr << ERRO_UNRECOGNIZED_OPTION << argv[i] << endl;
        param.parse_error = true;
        return true;
    }

    return false;
}


// Parse the config file for the network. The switches used as the
// command line parameters are the global default parameters for all
// the routers, while here in the config file you get chance to
// control each router one by one.
// return 0 if successfully,
// return -1 if fail
int apply_config_file(const char * file_name) {
    if (param.verbose) 
        cout << "[I] parsing configuration file " << file_name << "..." << endl;

    char input_line[MAX_LINE];
    char * sub_string;

    ifstream in_file(file_name);

    if (!in_file.is_open() || !in_file.good())
        return -1;

    int id = -1;

    while (in_file.getline(input_line, MAX_LINE, '\n')) {

        // line starting with "#" are comments
        if (input_line[0] == '#')
            continue;

        if (strstr(input_line, "@NODE")) {
            sub_string = strstr(input_line, "@NODE");
            sub_string += strlen("@NODE");

            if (!sscanf(sub_string, "%d", &id))
                return -1;

            if (param.verbose) 
                cout << INFO_CONFIG_NODE << id << endl;

            continue;
        }

        if (strstr(input_line, "north_input_channel_buffer_size")) {
            sub_string = strstr(input_line, "north_input_channel_buffer_size");
            sub_string += strlen("north_input_channel_buffer_size");
            pRouter router = param.network->get_router(id);
            if (!router) 
                return -1;
            int buf_size;
            if (!sscanf(sub_string, "%d", &buf_size))
                return -1;
            if (buf_size < 0)
                return -1;
            router->set_input_channel_buffer_size(buf_size, north);
            continue;
        }

        if (strstr(input_line, "south_input_channel_buffer_size")) {
            sub_string = strstr(input_line, "south_input_channel_buffer_size");
            sub_string += strlen("south_input_channel_buffer_size");
            pRouter router = param.network->get_router(id);
            if (!router) 
                return -1;
            int buf_size;
            if (!sscanf(sub_string, "%d", &buf_size))
                return -1;
            if (buf_size < 0)
                return -1;
            router->set_input_channel_buffer_size(buf_size, south);
            continue;
        }

        if (strstr(input_line, "east_input_channel_buffer_size")) {
            sub_string = strstr(input_line, "east_input_channel_buffer_size");
            sub_string += strlen("east_input_channel_buffer_size");
            pRouter router = param.network->get_router(id);
            if (!router) 
                return -1;
            int buf_size;
            if (!sscanf(sub_string, "%d", &buf_size))
                return -1;
            if (buf_size < 0)
                return -1;
            router->set_input_channel_buffer_size(buf_size, east);
            continue;
        }

        if (strstr(input_line, "west_input_channel_buffer_size")) {
            sub_string = strstr(input_line, "west_input_channel_buffer_size");
            sub_string += strlen("west_input_channel_buffer_size");
            pRouter router = param.network->get_router(id);
            if (!router) 
                return -1;
            int buf_size;
            if (!sscanf(sub_string, "%d", &buf_size))
                return -1;
            if (buf_size < 0)
                return -1;
            router->set_input_channel_buffer_size(buf_size, west);
            continue;
        }

        if (strstr(input_line, "input_channel_buffer_size")) {
            sub_string = strstr(input_line, "input_channel_buffer_size");
            sub_string += strlen("input_channel_buffer_size");
            pRouter router = param.network->get_router(id);
            if (!router) 
                return -1;
            int buf_size;
            if (!sscanf(sub_string, "%d", &buf_size))
                return -1;
            if (buf_size < 0)
                return -1;
            router->set_input_channel_buffer_size(buf_size);
            continue;
        }

        if (strstr(input_line, "output_channel_buffer_size")) {
            sub_string = strstr(input_line, "output_channel_buffer_size");
            sub_string += strlen("output_channel_buffer_size");
            pRouter router = param.network->get_router(id);
            if (!router) 
                return -1;
            int buf_size;
            if (!sscanf(sub_string, "%d", &buf_size))
                return -1;
            if (buf_size < 0)
                return -1;
            router->set_output_channel_buffer_size(buf_size);
            continue;
        }

        if (strstr(input_line, "routing_scheme")) {
            sub_string = strstr(input_line, "routing_scheme");
            sub_string += strlen("routing_scheme");
            pRouter router = param.network->get_router(id);
            if (!router) 
                return -1;
            Routing_scheme scheme = invalid_scheme;
            if (strstr(sub_string, "xy")) 
                scheme = xy;
            else if (strstr(sub_string, "dyad"))
                scheme = dyad;
            else if (strstr(sub_string, "oe_fixed"))
                scheme = oe_fixed;
            else if (strstr(sub_string, "oe"))
                scheme = oe;
	    else if (strstr(sub_string, "predict"))
	      scheme = predict;
	    else if (strstr(sub_string, "table")) { //UYO
	      scheme = rtable; //xy;
	      sub_string = strstr(sub_string,"table");
	      sub_string += strlen("table ");
	      router->set_routing_table(sub_string);
	      param.routing_scheme = rtable;
	    }
            router->set_routing_scheme(scheme);
            continue;
        }

	if ( strstr(input_line, "adaptive_long_link") ) {
	  param.adaptive_LR1 = true;
	  continue;
	}

	// UYO: Add random links as specified by the .config file
	if ( strstr(input_line, "random_link") ) {
	  // cout <<"Found random link \n";
	  sub_string = strstr(input_line, "random_link");
	  sub_string += strlen("random_link");
	  pRouter curr_router = param.network->get_router(id);
	  if (!curr_router) // No router exists with this id
	    return -1;
          // Update the orion power model
	  curr_router->set_power_module(param.n_of_ports);
	  // Update the xbar Ebit
	  curr_router->set_xbar_ebit(param.xbar_ebit*6/5);
	  unsigned int dst_id;
	  if ( !sscanf(sub_string,"%d",&dst_id) )
	    return -1;
	  // cout << "From router: " <<id <<", to destination: " <<dst_id <<endl;
	  pRouter dst_router = param.network->get_router(dst_id);
	  if (!dst_router)
	    return -1;
	  // Update the orion power model
	  dst_router->set_power_module(param.n_of_ports);
	  // Update the xbar Ebit
	  dst_router->set_xbar_ebit(param.xbar_ebit*6/5);
	  // We need a put a link from curr_router->dst_router
	  Direction dir = (Direction) (param.n_of_ports-1); // The extra (6.) port, it's reverse is also itself
	  // cout << "Add port to direction: " <<dir <<endl;
	  pOut_port c_out_port = curr_router->get_out_port(dir);
	  pIn_port c_in_port = curr_router->get_in_port(dir);
	  // cout << "Outport is generated \n";
	  pIn_port d_in_port = dst_router->get_in_port(dir);
	  pOut_port d_out_port = dst_router->get_out_port(dir);
	  // cout << "Inport is generated \n";
	  Position curr_pos = curr_router->get_position();
	  Position dst_pos = dst_router->get_position();
	  Position repeater_pos;
	  int x_dev = (int)(dst_pos.x-curr_pos.x);
	  int y_dev = (int)(dst_pos.y-curr_pos.y);
	  // cout << "(curr_x,curr_y) = " <<curr_pos.x <<"," <<curr_pos.y <<endl;
	  // cout << "(dest_x,dest_y) = " <<dst_pos.x <<"," <<dst_pos.y <<endl;
	  bool conn_curr=false;
	  bool conn_dest=false;
	  pRepeater prev_repeater=NULL;
	  for (int i=0; i<abs(x_dev); i++) {
	    if ( x_dev>0 )
	      repeater_pos.x = curr_pos.x+i+1;
	    else
	      repeater_pos.x = curr_pos.x-i-1;
	    repeater_pos.y = curr_pos.y; 
	    pRepeater a_repeater = new Repeater(param.network,repeater_pos); // Generate the repeater
	    a_repeater->set_input_channel_buffer_size(1); // Input and output buffers both have one slot: total 2
	    a_repeater->set_output_channel_buffer_size(1);
	    param.network->repeaters.push_back(a_repeater);
	    //cout <<"Repeater is generated at: " <<repeater_pos.x <<", " << repeater_pos.y <<endl;
	    if ( i==0 ) {
	      param.network->connect(c_out_port,a_repeater->get_in_port((Direction) 0));
	      curr_router->set_sink(a_repeater->get_in_port((Direction) 0));
	      // printf("Router: "); curr_router->print_position(); cout<<" is connected to the repeater sink\n";
	      param.network->connect(a_repeater->get_out_port((Direction) 0),c_in_port);
	      conn_curr=true;
	      //cout <<"Made connection with the src router \n";
	      if ((y_dev==0) && (abs(x_dev)==2) ) {
		param.network->connect(a_repeater->get_out_port((Direction) 1),d_in_port);
		param.network->connect(d_out_port,a_repeater->get_in_port((Direction) 1));
		dst_router->set_sink(a_repeater->get_in_port((Direction) 1));
		// printf("Router: "); dst_router->print_position(); cout<<" is connected to the repeater sink\n";
		//cout <<"Made also connection with the dest router \n";
		conn_dest=true;
		break;
	      }
	    }
	    else if ( (y_dev==0) && (i==abs(x_dev)-2) )  { // Connect to the destination
	      param.network->connect(prev_repeater->get_out_port((Direction) 1),a_repeater->get_in_port((Direction) 0));
	      param.network->connect(a_repeater->get_out_port((Direction) 0),prev_repeater->get_in_port((Direction) 1));
	      param.network->connect(a_repeater->get_out_port((Direction) 1),d_in_port);
	      param.network->connect(d_out_port,a_repeater->get_in_port((Direction) 1));
	      dst_router->set_sink(a_repeater->get_in_port((Direction) 1));
	      // printf("Router: "); dst_router->print_position(); cout<<" is connected to the repeater sink\n";
	      //cout << "Connected to the destination in the same column \n";
  	      conn_dest=true;
	      break;
	    }
	    else if ( (y_dev==1) && (i==abs(x_dev)-1) )  { // Connect to the destination
	      param.network->connect(prev_repeater->get_out_port((Direction) 1),a_repeater->get_in_port((Direction) 0));
	      param.network->connect(a_repeater->get_out_port((Direction) 0),prev_repeater->get_in_port((Direction) 1));
	      param.network->connect(a_repeater->get_out_port((Direction) 1),d_in_port);
	      param.network->connect(d_out_port,a_repeater->get_in_port((Direction) 1));
	      dst_router->set_sink(a_repeater->get_in_port((Direction) 1));
	      // printf("Router: "); dst_router->print_position(); cout<<" is connected to the repeater sink\n";
	      conn_dest=true;
	      //cout << "Connected to dest to neighbor column \n";
	    }
	    else {
	      param.network->connect(prev_repeater->get_out_port((Direction) 1),a_repeater->get_in_port((Direction) 0));
	      param.network->connect(a_repeater->get_out_port((Direction) 0),prev_repeater->get_in_port((Direction) 1));
	      //cout << "Repeater connected: " <<repeater_pos.x <<", " << repeater_pos.y <<endl;
	    }
	    prev_repeater = a_repeater;
	    Position pp=prev_repeater->get_position();
	    //cout <<"Prev. repeater is: " <<pp.x <<", " << pp.y <<endl;
	  } 	  
          //cout <<"Continue to y direction \n";
	  for (int i=0;i<abs(y_dev)-1;i++) {
	    if ( y_dev>0 )
	      repeater_pos.y = curr_pos.y+i+1;
	    else
	      repeater_pos.y = curr_pos.y-i-1;
	    repeater_pos.x = dst_pos.x; 
	    pRepeater a_repeater = new Repeater(param.network,repeater_pos); // Generate the repeater
	    a_repeater->set_input_channel_buffer_size(1); // Input and output buffers both have one slot: total 2
	    a_repeater->set_output_channel_buffer_size(1);
	    param.network->repeaters.push_back(a_repeater);
	    //cout << "Repeater is generated at: " <<repeater_pos.x <<", " << repeater_pos.y <<endl;
	    if (!conn_curr) { // Make the connection to the current router
	      param.network->connect(c_out_port,a_repeater->get_in_port((Direction) 0));
	      curr_router->set_sink(a_repeater->get_in_port((Direction) 0));
	      // printf("Router: "); curr_router->print_position(); cout<<" is connected to the repeater sink\n";
	      param.network->connect(a_repeater->get_out_port((Direction) 0),c_in_port);
	      conn_curr=true;
	      // cout <<"Made connection with the src router \n";
	    }
	    else {
	      param.network->connect(prev_repeater->get_out_port((Direction) 1),a_repeater->get_in_port((Direction) 0));
	      param.network->connect(a_repeater->get_out_port((Direction) 0),prev_repeater->get_in_port((Direction) 1));
	     // cout << "Repeater connected: " <<repeater_pos.x <<", " << repeater_pos.y <<endl;
	    }
	    prev_repeater = a_repeater;
	    // Position pp=prev_repeater->get_position();
	    // cout <<"Prev. repeater is: " <<pp.x <<", " << pp.y <<endl;
	  }
	  // Now connect the last repeater to the destination router
	  if (!conn_dest) {
	    param.network->connect(prev_repeater->get_out_port((Direction) 1),d_in_port);
	    param.network->connect(d_out_port,prev_repeater->get_in_port((Direction) 1));
	    dst_router->set_sink(prev_repeater->get_in_port((Direction) 1));
	    // printf("Router: "); dst_router->print_position(); cout<<" is connected to the repeater sink\n";
	    // cout << "Connection is made although it shouldn't \n";
	  } //else cout <<"Fine cont...\n";
	  continue;
	}

        // Note that if both packet generating rate and the trace_file are
        // specified for a traffic_source, the traffic-source will use trace_file
        // in generating packet. 
        if (strstr(input_line, "packet_generating_rate")) {
            sub_string = strstr(input_line, "packet_generating_rate");
            sub_string += strlen("packet_generating_rate");
            pTraffic_source source = param.network->get_traffic_source(id);
            if (!source)
                return -1;
            double rate = 0;
            if (!sscanf(sub_string, "%lf", &rate))
                return -1;
            if (rate > 1) {
                cerr << ERRO_PACKET_RATE;
                return -1;
            }
            source->set_packet_generating_rate(rate);
            continue;
        }

        if (strstr(input_line, "trace_file")) {
            sub_string = strstr(input_line, "trace_file");
            sub_string += strlen("trace_file");
            pTraffic_source source = param.network->get_traffic_source(id);
/*Daniel added codes*/
            for(vector<pATG>::iterator iter=param.ATGSETs.sets.begin(); iter<param.ATGSETs.sets.end(); iter++)
            {
            	for(vector<int>::iterator itersources=(*iter)->nodes.begin(); itersources<(*iter)->nodes.end(); itersources++)
            		if(*itersources==id)
            		{
            			source->set_time_start((*iter)->time_start);break;
            		}
            }
/*Daniel added end*/
            if (!source)
                return -1;
            if (source->set_trace_file(sub_string))
                return -1;
            continue;
        }

        if (strstr(input_line, "packet_to_destination_rate")) {
            pTraffic_source source = param.network->get_traffic_source(id);
            if (!source)
                return -1;
            if (source->has_trace_file()) {
                cerr << WARN_PACKET_RATE;
                continue;
            }
            sub_string = strstr(input_line, "packet_to_destination_rate");
            sub_string +=strlen("packet_to_destination_rate");
            unsigned int dst_id;
            double rate = 0;
            if (!sscanf(sub_string, "%d\t%lf", &dst_id, &rate)) 
                return -1;
            if (rate > 1.) {
                cerr << ERRO_PACKET_RATE;
                return -1;
            }
            source->set_packet_to_destination_rate(dst_id, rate);
            continue;
        }

	// UYO: ON-OFF traffic source info from the config files
	if (strstr(input_line, "OnOff")) {
	  pTraffic_source source = param.network->get_traffic_source(id);
	  if (!source)
	    return -1;
	  if (source->has_trace_file()) {
	    cerr << WARN_PACKET_RATE;
	    continue;
	  }
          
          pRouter curr_router = param.network->get_router(id);
          if (!curr_router)
            return -1;

	  sub_string = strstr(input_line, "OnOff");
	  sub_string +=strlen("OnOff");
	  unsigned int on_per,off_per,wait_per,monitor_on=0;
	  if (!sscanf(sub_string, "%d\t%d\t%d\t%d", &on_per,&off_per,&wait_per,&monitor_on) )
	    return -1;
	  source->set_OnOff(on_per,off_per,wait_per);
          if (monitor_on) {
	    curr_router->set_monitor();
	    param.dump_input_buffer=1;
	    source->src_monitoring = 1;
	  }

	  continue;
	}

        if (strstr(input_line, "packet_consuming_rate")) {
            sub_string = strstr(input_line, "packet_consuming_rate");
            sub_string += strlen("packet_consuming_rate");
            pTraffic_sink sink = param.network->get_traffic_sink(id);
            if (!sink)
                return -1;
            double rate = 0;
            if (!sscanf(sub_string, "%lf", &rate))
                return -1;
            sink->set_packet_consuming_rate(rate);
            continue;
        }

        // un-recognized options
        if (strlen(input_line) > 0) {
            cerr << WARN_OPTION_CONFIG_FILE << input_line << endl;
            continue;
        }
    }

    in_file.close();
    return 0;
}


// A hook you can use to configure your network before simulation. At this stage
// the network has been allocated already. 
// Return false: success
//        true:  fail
bool post_initialization_configure(Network & network) {
    // verify some parameters and fill in default one
    if (param.traffic_mode == hotspot && param.hotspots.empty()) {
        unsigned int default_hotspot = param.n_of_rows/2 * param.n_of_cols + param.n_of_cols/2;
        param.hotspots.push_back(default_hotspot);
    }

    // calculate some other parameters
    // initialize the non_hotspots vector for param
    param.non_hotspots.clear();
    if (param.traffic_mode == hotspot) {
        for (unsigned int id=0; id<param.n_of_rows*param.n_of_cols; id++) {
            if (find(param.hotspots.begin(), param.hotspots.end(), id) == param.hotspots.end()) {
                param.non_hotspots.push_back(id);
            }
        }
    }

    // change some router parameters if you want here

    return false;
}


