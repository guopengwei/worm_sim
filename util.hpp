/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: util.hpp
 Date Created:  <Sun Oct 19 19:12:00 2003>
 Last Modified: <Wed Oct  6 02:04:45 2004>
 Description: 
************************************************************************/

#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <iostream>
#include <iomanip>
#include <sys/types.h>
#include <sys/times.h>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include "common.hpp"
#include "misc.hpp"
#include "global_val.hpp"


void print_statistics(Network & network);
void compile_energy_info(Network & network);
void print_parameters(void);
void print_help(void);
string rip_off_space_and_table(string & s);
bool parse_options(int argn, char ** argv);
int apply_config_file(const char * file_name);
void print_times(clock_t real, struct tms *tmsstart, struct tms *tmsend);
bool post_initialization_configure(Network & network);
bool print_buffer_occupancy(Network & network); // UYO
bool print_input_buffer_occupancy(Network & network); 
bool print_output_buffer_occupancy(Network & network); 

#endif // UTIL_HPP_




