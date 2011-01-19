/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: power.hpp
 Date Created:  <Tue Oct  5 01:10:47 2004>
 Last Modified: <Thu Oct  7 17:10:45 2004>
 Description: Routing engine power consumption calculation is not supported
 yet, since orion does not allow ALU with arbitrary data width
************************************************************************/

#ifndef POWER_HPP_
#define POWER_HPP_

#include <vector>
#include <iostream>
#include <fstream>
#include "common.hpp"

extern "C" {
#include "SIM_power.h"
#include "SIM_router_power.h"
#include "SIM_power_router.h"
#include "SIM_power_ALU.h"
}

typedef class Power_module {
    SIM_power_router_info_t router_info_;
    SIM_power_router_t router_power_;
    SIM_power_arbiter_t arbiter_power_;
    SIM_power_bus_t link_power_;
//     SIM_power_ALU_t alu_power_dir_x_;
//     SIM_power_ALU_t alu_power_dir_y_;
//     SIM_power_ALU_t alu_power_buf_cmp_;

    // data/status from the last transaction
    vector<Atom_type> buffer_write_;
    vector<Atom_type> crossbar_read_;
    vector<Atom_type> crossbar_write_;
    vector<Atom_type> link_traversal_;
    vector<int> crossbar_input_;
    vector<Atom_type> arbiter_req_;
    vector<unsigned int> arbiter_grant_;

public:
    Power_module(int a);
    void power_buffer_read(int in_port, Atom_type read_d);
    void power_buffer_write(int in_port, Atom_type write_d);
    void power_crossbar_trav(int in_port, int out_port, Atom_type trav_d);
    void power_arbiter(int pc, Atom_type req, unsigned int gra);
    void power_link_traversal(int in_port, Atom_type read_d);
    double power_buffer_report();
    double power_link_report();
    double power_crossbar_report();
    double power_arbiter_report();
//     void power_alu_x(int pc, Atom_type d1, Atom_type d2);
//     void power_alu_y(int pc, Atom_type d1, Atom_type d2);
//     void power_alu_buf(int pc, Atom_type d1, Atom_type d2);
//     double power_alu_report();
};

typedef Power_module *pPower_module;

#endif // POWER_HPP_
