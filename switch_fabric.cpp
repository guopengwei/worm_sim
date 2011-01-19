/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: switch_fabric.cpp
 Date Created:  <Tue Oct 14 13:32:47 2003>
 Last Modified: <Sun Oct 24 01:51:21 2004>
 Description: Implementation for switching fabric
************************************************************************/

#include "switch_fabric.hpp"

Switch_fabric::Switch_fabric(int sz) {
    size = sz;
    inputs.clear(); 
    outputs.clear();
    for (unsigned int i=0; i<size; i++) {
        pSw_input_end an_input = new Sw_input_end();
        inputs.push_back(an_input);
        pSw_output_end an_output = new Sw_output_end();
        outputs.push_back(an_output);
    }
}


void Switch_fabric::setup_connection(pSw_input_end swi, pSw_output_end swo) { 
  assert(!swi->is_busy());
  assert(!swo->is_busy());
  swi->connect(swo);
  swo->connect(swi);
  return;
}


void Switch_fabric::teardown_connection(pSw_input_end swi, pSw_output_end swo) { 
    assert(swi->is_busy());
    assert(swo->is_busy());
    swi->disconnect();
    swo->disconnect();
    return;
}

