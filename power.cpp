/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: power.cpp
 Date Created:  <Tue Oct  5 01:15:00 2004>
 Last Modified: <Thu Oct  7 17:09:51 2004>
 Description: Part of the code adapted from Popnet project of Princeton, 
 which is written by Shang Li.
************************************************************************/

#include <iostream>
#include <iomanip>

#include "global_val.hpp"
#include "power.hpp"

//a: physical ports
Power_module::Power_module(int a):
    router_info_(),
    router_power_(),
    arbiter_power_(),
    link_power_(),
//     alu_power_dir_x_(),         // alu to determine routing direction
//     alu_power_dir_y_(),
//     alu_power_buf_cmp_(),     // alu to compare buffer depth in adaptive routing
    buffer_write_(),
    crossbar_read_(),
    crossbar_write_(),
    link_traversal_(),
    crossbar_input_(),
    arbiter_req_(),
    arbiter_grant_()
{
    router_info_.in_buf_info.data_width = param.flit_size;
    router_info_.in_buf_info.blk_bits = param.flit_size;
    router_info_.in_buf_info.eff_data_cols = param.flit_size;
    router_info_.in_buf_info.n_set = param.in_channel_buffer_size;
    FUNC(SIM_router_power_init, &router_info_, & router_power_);
    buffer_write_.resize(a);
    crossbar_read_.resize(a);
    crossbar_write_.resize(a);
    link_traversal_.resize(a+1);     // the last one is for the output to PE
    crossbar_input_.resize(a);

    // FIXME: double check the init of arbiter. What type of arbiter to use?
    // orion supports: RR_ARBITER, MATRIX_ARBITER, QUEUE_ARBITER
    // Queue arbiter is not implemented yet in Orion, so we temporarily use
    // RR instead, which is not that accurate though
    //    SIM_arbiter_init(& arbiter_power_, QUEUE_ARBITER, 1, a, 0, &arbiter_info_);
    SIM_arbiter_init(&arbiter_power_, RR_ARBITER, 1, a, 0, NULL);
    arbiter_req_.resize(a);
    arbiter_grant_.resize(a);
    SIM_bus_init(&link_power_, GENERIC_BUS, IDENT_ENC, param.flit_size, 
                 0, 1, 1, param.orion_power.link_length, 0);

    // calculate the data_width for alu
//     int data_width = (int) (ceil(log2((double) param.n_of_rows)));
//     SIM_power_ALU_init(&alu_power_dir_x_, PLX_ALU, data_width);
//     data_width = (int) (ceil(log2((double) param.n_of_cols)));
//     SIM_power_ALU_init(&alu_power_dir_y_, PLX_ALU, data_width);
//     data_width = (int) (ceil(log2((double) param.in_channel_buffer_size)));
//     SIM_power_ALU_init(&alu_power_buf_cmp_, PLX_ALU, data_width);
}

void Power_module::power_buffer_read(int in_port, Atom_type read_d)
{
    FUNC(SIM_buf_power_data_read, &(router_info_.in_buf_info), 
         &(router_power_.in_buf), read_d);
}

void Power_module::power_link_traversal(int in_port, Atom_type read_d)
{
    SIM_bus_record(&link_power_, link_traversal_[in_port], read_d);
    link_traversal_[in_port] = read_d;
}


void Power_module::power_buffer_write(int in_port, Atom_type write_d)
{
    Atom_type old_d = buffer_write_[in_port];
    Atom_type new_d = write_d;
    FUNC(SIM_buf_power_data_write, &(router_info_.in_buf_info), 
         &(router_power_.in_buf), (char *) (&old_d), 
         (char *) (&old_d), (char *) (&new_d));
    buffer_write_[in_port] = write_d;
}


void Power_module::power_crossbar_trav(int in_port, int out_port, 
                                       Atom_type trav_d)
{
    // sending flit energy consumption
    SIM_crossbar_record(&(router_power_.crossbar), 1, trav_d, 
                        crossbar_read_[in_port], 1, 1);

    // receiving flit energy consumption
    SIM_crossbar_record(&(router_power_.crossbar), 0, trav_d, 
                        crossbar_write_[out_port], crossbar_input_[out_port], 
                        in_port);

    crossbar_read_[in_port] = trav_d;
    crossbar_write_[out_port] = trav_d;
    crossbar_input_[out_port] = in_port;
}


void Power_module::power_arbiter(int pc, Atom_type req, unsigned int gra){
    // previously outport is idle, so we assume arbiter preserve
    // the old state
    if (gra == invalid_dir) 
        gra = arbiter_grant_[pc];

    SIM_arbiter_record(& arbiter_power_, req, arbiter_req_[pc],
                       gra, arbiter_grant_[pc]);
    arbiter_req_[pc] = req;
    arbiter_grant_[pc] = gra;
}



double Power_module::power_arbiter_report()
{
    return SIM_arbiter_report(& arbiter_power_);
}


double Power_module::power_buffer_report()
{
    return SIM_array_power_report(&(router_info_.in_buf_info), 
                                  &(router_power_.in_buf));
}


double Power_module::power_crossbar_report()
{
    return SIM_crossbar_report(&(router_power_.crossbar));
}


double Power_module::power_link_report()
{
    return SIM_bus_report(& link_power_);
}

// void Power_module::power_alu_x(int pc, Atom_type d1, Atom_type d2) {
//     SIM_power_ALU_record(&alu_power_dir_x_, d1, d2, 0);
// }

// void Power_module::power_alu_y(int pc, Atom_type d1, Atom_type d2) {
//     SIM_power_ALU_record(&alu_power_dir_y_, d1, d2, 0);
// }

// void Power_module::power_alu_buf(int pc, Atom_type d1, Atom_type d2) {
//     SIM_power_ALU_record(&alu_power_buf_cmp_, d1, d2, 0);
// }

// double Power_module::power_alu_report() {
//     double res = 0.;
//     res += SIM_power_ALU_report(&alu_power_dir_x_);
//     res += SIM_power_ALU_report(&alu_power_dir_y_);
//     res += SIM_power_ALU_report(&alu_power_buf_cmp_);
//     return res;
// }

