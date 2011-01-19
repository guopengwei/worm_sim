/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: worm_sim.cpp
 Date Created:  <Mon Oct 13 11:44:30 2003>
 Last Modified: <Wed Jan 19 11:36:22 2005>
 Description: 
************************************************************************/

#include <iostream>
#include <sys/types.h>
#include <cstdlib>
#include <cstdio>
#include <sys/times.h>
//#include <sys/time.h>

#include "version.hpp"
#include "misc.hpp"
#include "channel.hpp"
#include "pkt.hpp"
#include "routing_engine.hpp"
#include "version.hpp"
#include "msg.hpp"
#include "util.hpp"

#include <time.h>

using namespace std;

Net_clock net_clock;
Param param;
unsigned int Traffic_source::id_base = 0;
unsigned int Traffic_sink::id_base = 0;


int main(int argn, char ** argv) {

  struct timespec cur_time, new_time; 
  clock_gettime(CLOCK_REALTIME, &cur_time);
  //struct timeval cur_time, new_time; 
  //gettimeofday(&cur_time, NULL);

    parse_options(argn, argv);

    if (param.parse_error) {
        cerr << ERRO_PARSING_OPTIONS;
        return -1;
    }

    if (param.quit_flag)
        return 0;

    srand(param.rand_seed);

    Network network;

    if (!network.get_success_flag()) {
        cerr << ERRO_NETWORK_BUILD_ERROR;
        return -1;
    }

//    for (vector<char*>::iterator f = param.config_files.begin();
//         f < param.config_files.end(); f++) {
//        if (apply_config_file(*f)) {
//            cerr << ERRO_CONFIG_FILE;
//            return -1;
//        }
//    }
        
    if (post_initialization_configure(network)) {
        cerr << ERRO_NETWORK_BUILD_ERROR;
        return -1;
    }

    if (param.equation_file) {
        network.dump_equation_file();
        return 1;
    }

    if (param.perf_anal_lib_equation_file) {
        network.dump_perf_anal_lib_equation_file();
        return 1;
    }

    if (param.config_file_to_dump) {
        network.dump_config_file();
        return 1;
    }

    clock_t start, end;
    struct tms tmsstart, tmsend;
    start = times(&tmsstart);

    net_clock.reset();
    while (net_clock.tick() < param.simulation_length) {
        network.tick();
    }

    end = times(&tmsend);

    cout << INFO_SIM_DONE;

    if(param.extreme_verbose)
    	cerr << INFO_SIM_DONE;

    print_parameters();
    print_statistics(network);

    if (param.dump_input_buffer)
      print_input_buffer_occupancy(network);

    if (param.dump_output_buffer)
      print_output_buffer_occupancy(network);

    if (param.print_simulation_time)
        print_times(end-start, &tmsstart, &tmsend);

    if (param.print_n_of_packets) {
      param.packets_t <<"];";
      param.packets_t.close(); 
    }

    clock_gettime(CLOCK_REALTIME, &new_time);
    double elapsed_time = (double) 1000.0*(new_time.tv_sec-cur_time.tv_sec) + (double)(new_time.tv_nsec-cur_time.tv_nsec)/1000000.0;
    	cout<< "The total run time is "<<elapsed_time<<" msec \n";
    if(param.extreme_verbose)
    	cerr << "The total run time is "<<elapsed_time<<" msec \n";
    return 1;
}
