/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: router.cpp
 Date Created:  <Tue Oct 14 13:40:48 2003>
 Last Modified: <Thu Dec 16 18:50:06 2004>
 Description: 
************************************************************************/

#include "common.hpp"
#include "router.hpp"
#include "msg.hpp"
#include "global_val.hpp"
#include <iostream>

Router::Router(pNetwork n, Position p) : Addressee(p), 
                                         sw(param.n_of_switch_ports),
                                         arbiter(param.arbitration_delay, 
                                                 param.n_of_switch_ports) {
    if (param.extreme_verbose) 
        cout << "[I] Building router for location " << pos << endl;
    
    network = n;

    in_ports.clear();
    out_ports.clear();
    clear_contention();
    for (unsigned int i=0; i<network->get_num_of_ports(); i++) {
        Direction d = (Direction) i;
        pIn_port a_in_port = new In_port(this, d); 
        pOut_port a_out_port = new Out_port(this, d);
        in_ports.push_back(a_in_port);
        out_ports.push_back(a_out_port);
    }
    arbiter.bind(this);
    sw.bind(this);

    // bind switch ends with the input/output channels
    assert(sw.get_size() == in_ports.size() * in_ports[0]->get_size());
    assert(sw.get_size() == out_ports.size() * out_ports[0]->get_size());

    int index = 0;
    for (vector<pIn_port>::iterator iter=in_ports.begin(); iter<in_ports.end(); iter++) {
        for (unsigned int i=0; i<(*iter)->get_size(); i++) {
            pSw_input_end an_end = sw.get_input_end(index++);
            pInput_channel a_chan = (pInput_channel) (*iter)->get_channel(i);
            an_end->bind(a_chan);
            a_chan->bind(an_end);
        }
    }

    index = 0;
    for (vector<pOut_port>::iterator iter=out_ports.begin(); iter<out_ports.end(); iter++) {
        for (unsigned int i=0; i<(*iter)->get_size(); i++) {
            pSw_output_end an_end = sw.get_output_end(index++);
            pOutput_channel a_chan = (pOutput_channel) (*iter)->get_channel(i);
            an_end->bind(a_chan);
            a_chan->bind(an_end);
        }
    }


    // UYO monitor the routers
    if (param.dump_input_buffer||param.dump_output_buffer) 
      this->set_monitor();
    else
      monitor_router = 0;

    // set up the switch's ebit metric using default value
    sw.set_ebit(param.xbar_ebit);

    // create orion power module
    if (param.orion_power.use_orion_power_model) 
        power_module = new Power_module(param.n_of_ports-param.n_of_extra_links);
    else 
        power_module = NULL;
}


Router::~Router() {
    if (param.extreme_verbose) 
        cout << "[I] Deleting router for location " << pos << endl;

    for (vector<pIn_port>::iterator iter=in_ports.begin(); iter<in_ports.end(); iter++)
        delete (*iter);
    for (vector<pOut_port>::iterator iter=out_ports.begin(); iter<out_ports.end(); iter++)
        delete (*iter);

    if (power_module)
        delete power_module;
}

void Router::clear_contention(void){
	for(int i=0;i<5;i++)
		for(int j=0;j<5;j++)
			contention[i][j]=false;
}

void Router::receivecontention(int srcRouter,int dir,bool con){
	contention[srcRouter][dir]=con;
	if(con==true)
		int k=0;
}

void Router::set_monitor(void) { //UYO
  monitor_router = 1;
  for (vector<pIn_port>::iterator iter=in_ports.begin(); iter<in_ports.end(); iter++) {
    for (unsigned int i=0; i<(*iter)->get_size(); i++) {
      pInput_channel a_channel = (pInput_channel) (*iter)->get_channel(i);
      a_channel->monitoring=1;
    }
  }

  //begin rtg
  for (vector<pOut_port>::iterator iter=out_ports.begin(); iter<out_ports.end(); iter++) {
    for (unsigned int i=0; i<(*iter)->get_size(); i++) {
      pOutput_channel a_channel = (pOutput_channel) (*iter)->get_channel(i);
      a_channel->monitoring=1;
    }
  }
  //end   rtg
}

void Router::update_prediction(void) { // UYO: 
  // First read the input_channels of the down_stream routers to see how much they can accept
  vector <Direction> connection_status;
  unsigned int common_pool = 0;
  Direction dir = (Direction) 0;
  if (param.availability_thresh > 0) {
    for (vector<pOut_port>::iterator iter=out_ports.begin(); iter<out_ports.end(); iter++) {
      dir = (*iter)->get_direction();
      if (dir != local)  {
	for (unsigned int i=0; i<(*iter)->get_size(); i++) {
	  pOutput_channel myOut_chan = (pOutput_channel) (*iter)->get_channel(i); // Output channel
	  pRouter downRouter = get_router(dir);
	  if (downRouter == NULL)
	    break;
	  if (reverse(dir) ==  invalid_dir)
	    break;
	  pInput_channel downIn_chan = downRouter->get_in_channel(reverse(dir));
	  unsigned int acceptable = downIn_chan->get_can_accept();
	  // printf("Acceptable is:%d\n",acceptable);
	  myOut_chan->update_can_output(acceptable);
	  Direction conn_input = arbiter.get_current_granted_channel_dir(dir);
	  if (conn_input != invalid_dir) {
	    connection_status.push_back(conn_input);
	  }
	  else 
	    common_pool += acceptable;
	}
      } // if (dir != local)
    }  // for (vector... 
  }  // ~(param.basic_cont)


  // Knowing how much each out_channel can output, we now update how much each input channel can accept
  for (vector<pIn_port>::iterator iter=in_ports.begin(); iter<in_ports.end(); iter++) {
    for (unsigned int i=0; i<(*iter)->get_size(); i++) {
      dir = (*iter)->get_direction();
      pInput_channel a_channel = (pInput_channel) (*iter)->get_channel(i);
      unsigned int empty_slots = a_channel->num_of_empty_buffer_slots();
      for (vector<Direction>::iterator iter1=connection_status.begin(); iter1<connection_status.end(); iter1++) {
	if ((*iter1)==dir) { // Input channel dir is connected to (*iter)
	  pOutput_channel connect_to = a_channel->get_receiver();
	  empty_slots += connect_to->get_can_output();
	  break;
	}
       }
      unsigned int from_common_pool = (unsigned int) common_pool/(param. n_of_ports-1);
      a_channel->update_can_accept(empty_slots+from_common_pool);
      dir = (Direction) (dir+1);
    }
  } // outer for
  
} // update_prediction
  

void Router::set_power_module(int s) { // UYO 
  power_module = NULL;
  this->power_module = new Power_module(s);
}

void Router::set_input_channel_buffer_size(int s) {
    for (vector<pIn_port>::iterator iter=in_ports.begin(); iter<in_ports.end(); iter++)
        (*iter)->set_channel_buffer_size(s);
}


void Router::set_input_channel_buffer_size(int s, Direction dir) {
    if (dir == local) {
        cerr << "[W] Overriding local input buffer depth is not allowed. Action skipped." << endl;
        return;
    }
    pIn_port p = get_in_port(dir); 
    p->set_channel_buffer_size(s);
}


void Router::set_output_channel_buffer_size(int s) {
    for (vector<pOut_port>::iterator iter=out_ports.begin(); iter<out_ports.end(); iter++)
        (*iter)->set_channel_buffer_size(s);
}


void Router::set_routing_scheme(Routing_scheme rs) {
    for (vector<pIn_port>::iterator iter=in_ports.begin(); iter<in_ports.end(); iter++)
        (*iter)->set_routing_scheme(rs);
}

void Router::set_routing_table(char * table_string) { //UYO
    for(vector<pIn_port>::iterator iter=in_ports.begin(); iter<in_ports.end(); iter++)  
        (*iter)->set_routing_table(table_string);
  
 /* cout <<"@NODE   " << (this->pos.x)*param.n_of_rows+(this->pos.y) <<"\n";
    cout <<"routing_scheme  table ";
    for (int i=0; i<strlen(table_string); i++)
       cout << table_string[i]; //<<" ";
       cout <<"\n";  */
}

// find the index-th input virtual channel in the dir direction of the router
pInput_channel Router::get_in_channel(Direction dir, int index) {
    pIn_port a_port = in_ports[dir];
    return ((pInput_channel) a_port->get_channel(index));
}


pOutput_channel Router::get_out_channel(Direction dir, int index) {
    pOut_port a_port = out_ports[dir];
    return ((pOutput_channel) a_port->get_channel(index));
}


pOutput_channel Router::get_link_user_channel(Direction dir) {
    pOut_port a_port = out_ports[dir];
    printf("You're at:"); print_position(); printf("Direction is: %d\n",dir);
    return ((pOutput_channel) a_port->get_link_user_channel());
}
// return the router which is on the dir-side of the router
// return 0 if there does not exist a router (for instance,
// at the edge of the mesh 
pRouter Router::get_router(Direction dir) {
    Position a_pos = pos;
    switch (dir) {
    case north:
        a_pos.y ++;
        break;
    case south:
        a_pos.y --;
        break;
    case east:
        a_pos.x ++;
        break;
    case west:
        a_pos.x --;
        break;
    case local:
        return this;
    default:
        return 0;
    }

    if (network->get_topology() == mesh) {
        if (a_pos.x >= network->get_num_of_cols() || a_pos.x < 0)
            return 0;
        if (a_pos.y >= network->get_num_of_rows() || a_pos.y < 0)
            return 0;
        return network->get_router(a_pos);
    }
    else if (network->get_topology() == torus) {
        // increase to avoid negative numbers
        a_pos.x += network->get_num_of_cols();
        a_pos.y += network->get_num_of_rows();
        
        a_pos.x = a_pos.x % network->get_num_of_cols();
        a_pos.y = a_pos.y % network->get_num_of_rows();
        return network->get_router(a_pos);
    }
    else {
        cerr << ERRO_ARCH_NOT_SUPPORTED;
        return 0;
    }
}


void Router::in_ports_tick(void) {
    for (vector<pIn_port>::iterator iter=in_ports.begin(); iter<in_ports.end(); iter++) 
        (*iter)->tick();
}

void Router::out_ports_tick(void) {
    for (vector<pOut_port>::iterator iter=out_ports.begin(); iter<out_ports.end(); iter++) 
        (*iter)->tick();
}

void Router::arbiter_tick(void) {
    arbiter.tick();
}

