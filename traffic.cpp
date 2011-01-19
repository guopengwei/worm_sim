/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: traffic.cpp
 Date Created:  <Tue Oct 14 14:36:26 2003>
 Last Modified: <Sat Dec 18 20:57:28 2004>
 Description: 
************************************************************************/

#include "link.hpp"
#include "traffic.hpp"
#include "pkt.hpp"
#include "misc.hpp"
#include "msg.hpp"
#include "network.hpp"
#include "util.hpp"
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <string>


Traffic_source::Traffic_source(Position p, int buf_sz) : Buffer_owner(buf_sz), Addressee(p) {
    id = id_base ++;
    time_start=0;
    packet_generating_rate = param.packet_generating_rate;
    packet_size = param.flits_per_packet;
    flit_size = param.flit_size;
    traffic_mode = param.traffic_mode;
    period = 0;
    packet_to_destination_rate = 0;
    pkt_id = 0;
    // UYO
    on_off_model = 0;
    // UYO monitor the sources
    if (param.dump_input_buffer)
      src_monitoring = 1;
    else
      src_monitoring = 0;

    src_congestion_monitor.push_back(0);  // time of arrival
    src_congestion_monitor.push_back(0);  // buffer level


    if (param.dump_traffic_source_trace) {
        char file_name[20];
        sprintf(file_name, "%d.trace", id);
        trace_dump.open(file_name);
        if (!trace_dump.is_open() || !trace_dump.good()) {
            cerr << "Error in opening file " << file_name << " for trace dumping" << endl;
            exit(-1);
        }
        trace_dump << "PERIOD\t" << param.simulation_length << endl;
        trace_dump << "#Trace file dumped by worm_sim from node " << id << endl;
        trace_dump << "#Folloing lines are with format as:" << endl
                   << "#timestamp\t" << "destination\t" << "message_size(bits):" << endl;
    }
}


Traffic_source::~Traffic_source() {
    if (trace_file.is_open())
        trace_file.close();
    if (trace_dump.is_open())
        trace_dump.close();
    if (packet_to_destination_rate)
        delete []packet_to_destination_rate;
}


// processing the file_name by removing its \t and spaces at the head
// and attach the file handler with the class
// Return 0 if successful
// Return non zero if fail
int Traffic_source::set_trace_file(char * file_name) {

  trace_file_empty = false;

    string str_file(file_name);
    // lets rip off any space or table key at the beginning of the file_name
    rip_off_space_and_table(str_file);
    trace_file.open(str_file.c_str());

    trace_file_loop_cnt = 0;

    if (!trace_file.is_open() || !trace_file.good())
        return -1;

    // let's preparse the file to get the period, and also verify the trace_file
    period = 0;
    int prev_time = -1;
    char input_line[MAX_LINE];
    char * sub_string;
    while (trace_file.getline(input_line, MAX_LINE, '\n')) {
        // line starting with "#" are coomments
        if (input_line[0] == '#')
            continue;

        if (strstr(input_line, "PERIOD")) {
            sub_string = strstr(input_line, "PERIOD");
            sub_string += strlen("PERIOD");

            if (!sscanf(sub_string, "%d", &period)) {
                trace_file.close();
                cerr << ERRO_BAD_TRACE_FILE << str_file << endl;
                return -1;
            }
            assert(period > 0);
            continue;
        }

        // otherwise, the line should contain 3 integers in the format
        // of:
        // timestamp     destination     message_size
        int timestamp;
        unsigned int destination, message_size;
        istringstream istr(input_line, istringstream::in);
        istr >> timestamp >> destination >> message_size;
        period = max(timestamp+1, period);
        if (timestamp <= prev_time) {
            trace_file.close();
            cerr << ERRO_BAD_TRACE_FILE << str_file << endl;
            return -1;
        }
        prev_time = timestamp;
        continue;
    }

 //   assert(period > 0);

    if (prev_time == -1) {
      // the trace file does not contain any messages
      cout << WARN_EMPTY_TRACE_FILE << str_file << endl;
      trace_file_empty = true;
      return 0;
    }

    // rewind the trace file and pre-fetch one message
    trace_file.clear();
    trace_file.seekg(0, ios::beg);
    trace_file.clear();
    get_next_message(pre_fetched_message);

    return 0;
}

void Traffic_source::set_packet_generating_rate(double r) {
    assert(r >= 0 && r <= 1.0);
    if (packet_to_destination_rate) {
        cerr << WARN_PACKET_RATE;
        return;
    }
    packet_generating_rate = r; 
}

void Traffic_source::set_time_start(unsigned time){
	  time_start=time;
}

void Traffic_source::set_packet_to_destination_rate(unsigned int dst_id, double r) {
    assert(r >= 0 && r <= 1.0);
    if (!packet_to_destination_rate) {
        unsigned int sz = param.network->get_num_of_traffic_sinks();
        // initalize the packet_generating_rate table
        packet_to_destination_rate = new double[sz];
        for (unsigned int i=0; i<sz; i++) 
            packet_to_destination_rate[i] = 0.;

        // if we specify this, this will override the packet_generating_rate value
        packet_generating_rate = 0.;
    }
    packet_to_destination_rate[dst_id] = r;
    packet_generating_rate += r;       // packet_generating_rate will be the sum
    if (packet_generating_rate > 1.0) {
        cerr << ERRO_PACKET_RATE;
        exit(-1);
    }
}

// Return the packet rate from this traffic source to the destination sink dst_id
// Fetch the data from the array packet_to_destination_rate if it exists.
// Otherwise, calculate the rate from packet_generating_rate with the specified 
// traffic mode
double Traffic_source::get_packet_to_destination_rate(unsigned int dst_id) const {
    if (packet_to_destination_rate) 
        return packet_to_destination_rate[dst_id];

    int num = param.network->get_num_of_traffic_sinks();
    if (num == 1) {
        cerr << ERRO_EQUATION_FILE;
        exit(-1);
    }

    if (dst_id == id)  // no internal communication will use network
        return 0;

    unsigned int tmp;
    bool i_am_hotspot;
    int n_of_hotspots_dest, n_of_normal_dest;
    double normal_prob, hotspot_prob;
    vector<unsigned int>::iterator iter;

    switch (param.traffic_mode) {
    case uniform:
        return (packet_generating_rate/(num - 1));
    case transpose1:
        tmp = get_destination_transpose1();
        return (dst_id == tmp) ? packet_generating_rate : 0;
    case transpose2:
        tmp = get_destination_transpose2();
        return (dst_id == tmp) ? packet_generating_rate : 0;

    case hotspot:
        assert(param.hotspots.size());
        i_am_hotspot = false;
        iter = find(param.hotspots.begin(), param.hotspots.end(), id);
        if (iter != param.hotspots.end()) {    // this node is one of the hotspot
            i_am_hotspot = true;
        }
        // now calculate the probability of sending the packet to hotspot nodes versus non hotspot 
        // nodes
        n_of_hotspots_dest = param.hotspots.size();
        if (i_am_hotspot)
            n_of_hotspots_dest --;
        n_of_normal_dest = param.n_of_cols * param.n_of_rows - n_of_hotspots_dest - 1;
        normal_prob = 
            (1.0 - n_of_hotspots_dest*param.hotspot_percentage/100.) / (n_of_normal_dest + n_of_hotspots_dest);
        hotspot_prob = (1.0 - n_of_normal_dest*normal_prob) / n_of_hotspots_dest;
        iter = find(param.hotspots.begin(), param.hotspots.end(), dst_id);
        if (iter != param.hotspots.end()) 
            return hotspot_prob * packet_generating_rate;
        else
            return normal_prob * packet_generating_rate;

    default:
        cerr << ERRO_EQUATION_FILE;
        exit(-1);
    }
}


// SET the On-Off parameters UYO
void Traffic_source::set_OnOff(unsigned int on_per,unsigned int off_per,unsigned int wait_per) {
  on_off_model = 1; 
  on_period = (on_per>1)?on_per:1;
  off_period = (off_per>1)?off_per:1;
  wait_period = (wait_per>1)?wait_per:1;  
  next_generation_time = (int) (off_period-log(drand48())*wait_period); //(off_period
  pkt_to_sent = (int) 75000/(on_period+off_period+wait_period);
  total_pkt_sent = 0;
  // printf("Start time and total number of packets for %d is %d and %d \n",id,next_generation_time,pkt_to_sent);
}


double Traffic_source::get_total_packet_injection_rate(void) const {
    double total = 0.;
    unsigned int sz = param.network->get_num_of_traffic_sinks();
    for (unsigned int i = 0; i < sz; i++) 
        total += get_packet_to_destination_rate(i);
    return total;
}


// Get the next message from the trace file. 
// It skips non-message lines. and when arrives to the end of the file,
  // it will rewind the file pointer
bool Traffic_source::get_next_message(Message & msg) {
    char input_line[MAX_LINE];
 restart:
    while (trace_file.getline(input_line, MAX_LINE, '\n')) {
        // line starting with "#" are coomments
        if (input_line[0] == '#')
            continue;

        if (strstr(input_line, "PERIOD")) {
            continue;
        }

        istringstream istr(input_line, istringstream::in);
        istr >> msg.timestamp >> msg.destination >> msg.size;
        msg.timestamp += trace_file_loop_cnt * period+time_start+param.warmup_period;//Modified by Daniel Wei, add up the warmup_period and ATG start_time
        return true;
    }

    // we reach here if we get to the end of the file, but no message is read, 
    // in this case, we should rewind the file pointer and restart
    if (trace_file.eof()) {
        trace_file.clear();
        trace_file.seekg(0, ios::beg);
        trace_file.clear();
        trace_file_loop_cnt ++;
        goto restart;
    }
    else {
        cerr << ERRO_PARSING_TRACE_FILE;
        return false;
    }
}


void Traffic_source::tick(void) {
    if (trace_file.is_open()) {
        // if we have a trace file, read and generate packet using 
        // the trace file

      if (!trace_file_empty) {
        if (pre_fetched_message.timestamp == net_clock.get_clock()) { // % period) {
            // time to do send the message in the pre-fetched message
            generate_packets(pre_fetched_message);
            Message a_message;
            do {
                get_next_message(a_message);
                if (a_message.timestamp == net_clock.get_clock()) // % period) 
                    generate_packets(a_message);
                else
                    pre_fetched_message = a_message;
            } while (a_message.timestamp == net_clock.get_clock()); // % period);

            // the last message is the one which is not ready to fire yet,
            // we should save it as the pre_fetched_message
            pre_fetched_message = a_message;
        }
      }
    }
    else if (on_off_model)  {
      if (next_generation_time == net_clock.get_clock())  { // Time to generate new message
	// Decide the destination address
	unsigned int dst_id = id;
	if (packet_to_destination_rate) {
	  dst_id = get_destination_customized();
        }
        else if (traffic_mode == uniform) {
	  dst_id = get_destination_uniform();
        }
        else if (traffic_mode == transpose1) {
	  dst_id = get_destination_transpose1();
        }
        else if (traffic_mode == transpose2) {
	  dst_id = get_destination_transpose2();
        }
        else if (traffic_mode == hotspot) {
	  dst_id = get_destination_hotspot();
        }
        else {
	  cerr << ERRO_TRAFFIC_PATTEN_NOT_SUPPORTED;
	  return;
        }

        if ( (dst_id != id) && (total_pkt_sent < pkt_to_sent) ) {
	  bool pkt_generated = 0;
	  if (buffer.empty_slots() >= on_period) { // Finite source buffer
	    // Source flow control implemented here
	    if (param.flow_control) {
	      pRouter localRouter = param.network->get_router(pos);     // Find the local router
	      pIn_port localPort = localRouter->get_in_port(local);     // get the local port
	      int acceptable = localPort->get_total_acceptable();
	 
	      if (acceptable > param.availability_thresh) {
		generate_a_packet(dst_id,on_period);
		pkt_generated = 1;
		total_pkt_sent ++;
	      }
	      //      if (id == 6 || id == 8 || id == 10 || id == 15)
	      //	printf("At %d acceptable:%d \n",id,acceptable);
	    }  // Flow control
	    else {
	      generate_a_packet(dst_id,on_period);
	      pkt_generated = 1;
	      total_pkt_sent ++;
	    }
	  } // buffer.empty_slots
          // Compute the next generate time
	  if (!pkt_generated) {
	    next_generation_time ++;
	    param.gen_delay ++;
	  }
	  else {
	    int wait_time = (int) (-log(drand48())*wait_period);
	    if (wait_time <= 0)
	      wait_time = 0;
	    next_generation_time =  net_clock.get_clock() + on_period + off_period + wait_time;
	    // printf("Next generation time for %d is %d \n",id,next_generation_time);
	    // printf("Empty slots not sufficient \n");
	  }
	} // (dst_id != id)
	//	else {
	//  //printf("At %d, total pkt sent is %d at time %d\n",id,total_pkt_sent, net_clock.get_clock());
	//  param.on_off_complete_time =  net_clock.get_clock();
	//}   
      } // if(next_generation...
    } // else if (on_off_model
    
    else if (drand48() < packet_generating_rate) {  // mimic possion arrival
        unsigned int dst_id = id;

        // if this node has customized rate table, use this one instead of 
        // global traffic mode setup
        if (packet_to_destination_rate) {
            dst_id = get_destination_customized();
        }
        else if (traffic_mode == uniform) {
            dst_id = get_destination_uniform();
        }
        else if (traffic_mode == transpose1) {
            dst_id = get_destination_transpose1();
        }
        else if (traffic_mode == transpose2) {
            dst_id = get_destination_transpose2();
        }
        else if (traffic_mode == hotspot) {
            dst_id = get_destination_hotspot();
        }
        else {
            cerr << ERRO_TRAFFIC_PATTEN_NOT_SUPPORTED;
            return;
        }
        if (dst_id != id)
            generate_a_packet(dst_id);
    }

    // as a sender, do sending
    if (can_send()) 
      send();
}

unsigned int Traffic_source::get_destination_uniform(void) const {
    unsigned dst_id = id;
    while (dst_id == id) 
        dst_id = (unsigned int) (drand48() * param.network->get_num_of_traffic_sinks());
    return dst_id;
}

unsigned int Traffic_source::get_destination_transpose1(void) const {
    Position dst_pos;

    if (param.n_of_cols != param.n_of_rows) {
        cerr << "[E] transpose1 traffic pattern requires a square topology." << endl;
        exit (-1);
    }

    dst_pos.x = param.n_of_cols - 1 - pos.y;
    dst_pos.y = param.n_of_rows - 1 - pos.x;

    pTraffic_sink a_sink = param.network->get_traffic_sink(dst_pos);
    return a_sink->get_id();
}

unsigned int Traffic_source::get_destination_transpose2(void) const {
    Position dst_pos;

    if (param.n_of_cols != param.n_of_rows) {
        cerr << "[E] transpose2 traffic pattern requires a square topology." << endl;
        exit (-1);
    }

    dst_pos.x = pos.y;
    dst_pos.y = pos.x;

    pTraffic_sink a_sink = param.network->get_traffic_sink(dst_pos);
    return a_sink->get_id();
}


unsigned int Traffic_source::get_destination_hotspot(void) const {

    assert(param.hotspots.size());
    bool i_am_hotspot = false;
    vector<unsigned int>::iterator iter = find(param.hotspots.begin(), param.hotspots.end(), id);
    if (iter != param.hotspots.end()) {    // this node is one of the hotspot
        i_am_hotspot = true;
    }

    // now calculate the probability of sending the packet to hotspot nodes versus non hotspot 
    // nodes
    int n_of_hotspots_dest = param.hotspots.size();
    if (i_am_hotspot)
        n_of_hotspots_dest --;
    int n_of_normal_dest = param.n_of_cols * param.n_of_rows - n_of_hotspots_dest - 1;

    double normal_prob = 
        (1.0 - n_of_hotspots_dest*param.hotspot_percentage/100.) / (n_of_normal_dest + n_of_hotspots_dest);

    if (normal_prob < 0) {
        cerr << ERRO_BAD_HOTSPOT_PARAM;
        exit(-1);
    }

    // calculate the probability of choosing a non hotspot node
    double sum_normal_prob = normal_prob * n_of_normal_dest; 

    unsigned dst_id = id;

    if (drand48() < sum_normal_prob) { 
        // set the destination to one of the normal nodes
        while (dst_id == id) {
            unsigned int index = (unsigned int) (drand48() * param.non_hotspots.size());
            dst_id = param.non_hotspots[index];
        }
    }
    else {
        assert(n_of_hotspots_dest);
        // set the destination to one of the hotspots
        while (dst_id == id) {
            unsigned int index = (unsigned int) (drand48() * param.hotspots.size());
            dst_id = param.hotspots[index];
        }
    }

    return dst_id;
}


// generate the packet destination based on `packet_to_destination_rate` table
unsigned int Traffic_source::get_destination_customized(void) const {
    double rnd = packet_generating_rate * drand48();
    double accu_sum = 0.;
    // we traverse packet_to_destination_rate until the accumulated rate sum is 
    // higher than rnd
    unsigned int sz = param.network->get_num_of_traffic_sinks();
    unsigned int last = id;
    for (unsigned int i=0; i<sz; i++) {
        if (packet_to_destination_rate[i] > 0) {
            last = i;
            accu_sum += packet_to_destination_rate[i];
            if (accu_sum >= rnd) 
                return i;
        }
    }
    // if we reach here, the problem can be the accuracy in double value representation
    // we simply return the larget destination node which we can send packet to
    assert(last != id);
    return last;
}


void Traffic_source::generate_a_packet(unsigned int dst_id) {
    if (id == dst_id) 
    	 cerr << "[W] sending packet to a local sink at time "<< net_clock.get_clock() << endl;

    pPacket a_packet = new Packet(pkt_id++, packet_size, flit_size);

    pFlit header_flit = a_packet->get_flit(0);
    header_flit->write_src_id(id);
    header_flit->write_dst_id(dst_id);
    a_packet->age=0;

    if (param.verbose) 
        cout << "[I] generated a packet with id " << a_packet->get_id() << " at source " << id 
             << " (time = " << net_clock.get_clock() << ")\n";

    param.n_of_packets = param.n_of_packets+1; //UYO
    if (param.print_n_of_packets) {
      param.packets_t << net_clock.get_clock() <<"\t" << param.n_of_packets << endl;
    }

    if (param.n_of_packets > param.max_n_of_packets)
      param.max_n_of_packets = (int) param.n_of_packets;

    if (net_clock.get_clock() > param.warmup_period) {
      param.n_of_sent_packets ++;

      param.n_t = ((param.n_t*param.n_cnt)+param.n_of_packets)/(param.n_cnt+1); // UYO
      param.n_cnt = param.n_cnt+1;  //UYO
      //      printf("%f  %f  %f \n",param.n_cnt,param.n_of_packets,param.n_t);
  
      // Congestion monitor
      if (src_monitoring) {
	int len = src_congestion_monitor.size();
	//	printf("Monitoring at %d, 2*sent: %d\n",net_clock.get_clock(),len);
	if (src_congestion_monitor[len-1]==net_clock.get_clock()) { // A flit has been sent during this clock cycle
	  src_congestion_monitor[len-1] += packet_size;             // Do not add a new item, just correct the current occupancy
	} 
	else {
	  int prev = src_congestion_monitor.back();
	  src_congestion_monitor.push_back(net_clock.get_clock());
	  src_congestion_monitor.push_back(prev+packet_size);
	  total_pkt_sent ++;
	  //	  printf("Sent new, total sent:%d, buffer util:%d \n",total_pkt_sent,prev+packet_size);
	} // else
      }
      // Congestion monitor
    }

    // add the packet to my local buffer
    add(a_packet);

    if (src_monitoring && net_clock.get_clock() > param.warmup_period)
    {
      //begin by rtg
      pPacket p = a_packet;
      for (int i = 0; (unsigned)i < packet_size; i++)
        {
          pFlit a_flit = a_packet->get_flit(i);
          int flit_type = 1;  //assume header
          if (a_flit->is_tail())
            flit_type = 3;   //tail
          else if (!a_flit->is_header())
            flit_type = 2;  //body
      
          buffer_monitor.push_back(p->get_id());
          buffer_monitor.push_back(a_flit->get_sequence_id());
          buffer_monitor.push_back(flit_type);
          buffer_monitor.push_back(net_clock.get_clock());
          buffer_monitor.push_back(1);
          buffer_monitor.push_back(p->get_src_position().x);
          buffer_monitor.push_back(p->get_src_position().y);
          buffer_monitor.push_back(p->get_dst_position().x);
          buffer_monitor.push_back(p->get_dst_position().y);
          buffer_monitor.push_back(buffer.get_num_of_flits());
        }
      //end by rtg
    }

    if (param.dump_traffic_source_trace) 
        trace_dump << net_clock.get_clock() << "\t" << dst_id << "\t" << 1 << endl;
}

// UYO Generate a packet OF GIVEN size
void Traffic_source::generate_a_packet(unsigned int dst_id,unsigned int custom_packet_size) {
    if (id == dst_id) 
        cerr << "[W] sending packet to a local sink " << endl;

    pPacket a_packet = new Packet(pkt_id++, custom_packet_size, flit_size);

    pFlit header_flit = a_packet->get_flit(0);
    header_flit->write_src_id(id);
    header_flit->write_dst_id(dst_id);
    a_packet->age=0;

    if (param.verbose) 
        cout << "[I] generated a packet with id " << a_packet->get_id() << " at source " << id 
             << " (time = " << net_clock.get_clock() << ")\n";

    param.n_of_packets = param.n_of_packets+1; //UYO
    if (param.print_n_of_packets) {
      param.packets_t << net_clock.get_clock() <<"\t" << param.n_of_packets << endl;
    }

    if (param.n_of_packets > param.max_n_of_packets)
      param.max_n_of_packets = (int) param.n_of_packets;

    if (net_clock.get_clock() > param.warmup_period) {
      param.n_of_sent_packets ++;
      
      param.n_t = ((param.n_t*param.n_cnt)+param.n_of_packets)/(param.n_cnt+1); // UYO
      param.n_cnt = param.n_cnt+1;  //UYO
      //      printf("%f  %f  %f \n",param.n_cnt,param.n_of_packets,param.n_t);

      // Congestion monitor
      if (src_monitoring) {
	unsigned int len = src_congestion_monitor.size();
	if (src_congestion_monitor[len-2]==net_clock.get_clock()) { // A flit has been sent during this clock cycle
	  src_congestion_monitor[len-1] += packet_size;             // Do not add a new item, just correct the current occupancy
	} 
	else {
	  unsigned int prev = src_congestion_monitor.back();
	  src_congestion_monitor.push_back(net_clock.get_clock());
	  src_congestion_monitor.push_back(prev+packet_size);
	} // else


      }
      // Congestion monitor
    }

    // add the packet to my local buffer
    add(a_packet);

    if (src_monitoring && net_clock.get_clock() > param.warmup_period)
    {
      //begin by rtg
      pPacket p = a_packet;
      for (int i = 0; (unsigned)i < packet_size; i++)
        {
          pFlit a_flit = a_packet->get_flit(i);
          int flit_type = 1;  //assume header
          if (a_flit->is_tail())
            flit_type = 3;   //tail
          else if (!a_flit->is_header())
            flit_type = 2;  //body
      
          buffer_monitor.push_back(p->get_id());
          buffer_monitor.push_back(a_flit->get_sequence_id());
          buffer_monitor.push_back(flit_type);
          buffer_monitor.push_back(net_clock.get_clock());
          buffer_monitor.push_back(1);
          buffer_monitor.push_back(p->get_src_position().x);
          buffer_monitor.push_back(p->get_src_position().y);
          buffer_monitor.push_back(p->get_dst_position().x);
          buffer_monitor.push_back(p->get_dst_position().y);
          buffer_monitor.push_back(buffer.get_num_of_flits());
        }
      //end by rtg
    }

    if (param.dump_traffic_source_trace) 
        trace_dump << net_clock.get_clock() << "\t" << dst_id << "\t" << 1 << endl;
}



// Generate one or more packet based on the given record
void Traffic_source::generate_packets(const Message & msg) {
    int num_of_packets = (int)
        ceil(((double) msg.size)/(param.flit_size * param.flits_per_packet - param.overhead_per_packet));
    for (int i=0; i<num_of_packets; i++) 
        generate_a_packet(msg.destination);
}

// traffic source/sinks owns link exclusively, so it does not need to check
// whether it has access to link or not
bool Traffic_source::can_send(void) const {
    if (buffer.is_empty())
        return false;

    // This is somehow trick, we need to verify whether the first flit in the fifo
    // is received right in this clock cycle. If so, we can not send it
    const Flit * first_flit = buffer.peek_flit();
    if (first_flit->arrived_in_this_cycle())
        return false;

    pConnector receiver = get_receiver();

    if (receiver) 
        return receiver->can_receive();
    else 
        return false;
}

bool Traffic_source::send(void) {
#ifdef DEBUG
    assert(can_send());
#endif 

    pLink link = (pLink) get_sink();
    pFlit a_flit = get_flit();
    link->receive(a_flit);
    if ( a_flit->is_header() ) {
    // Mark the time at which the packet leaves the source queue
    pPacket current_packet = a_flit->get_packet();
    current_packet->set_sent_time(net_clock.get_clock());
    }

    // Congestion monitor
    if ( (src_monitoring) && (net_clock.get_clock() > param.warmup_period) )  {
      if (a_flit->is_tail()) {
	unsigned int prev = src_congestion_monitor.back();
	src_congestion_monitor.push_back(net_clock.get_clock());
	src_congestion_monitor.push_back(prev-packet_size);
      } // if (a_flit..


      pPacket p = a_flit->get_packet();
      int flit_type = 1;  //assume header
      if (a_flit->is_tail())
        flit_type = 3;   //tail
      else if (!a_flit->is_header())
        flit_type = 2;  //body
      
      buffer_monitor.push_back(p->get_id());
      buffer_monitor.push_back(a_flit->get_sequence_id());
      buffer_monitor.push_back(flit_type);
      buffer_monitor.push_back(net_clock.get_clock());
      buffer_monitor.push_back(0);
      buffer_monitor.push_back(p->get_src_position().x);
      buffer_monitor.push_back(p->get_src_position().y);
      buffer_monitor.push_back(p->get_dst_position().x);
      buffer_monitor.push_back(p->get_dst_position().y);
      buffer_monitor.push_back(buffer.get_num_of_flits());
    }      // Congestion monitor
    
    return true;
}


pConnector Traffic_source::get_receiver(void) const {
    pLink link = (pLink) get_sink();
    return link->get_sink();
}


Traffic_sink::Traffic_sink(Position p, int buf_sz) : Buffer_owner(buf_sz), Addressee(p) {
    id = id_base++;
    packet_consuming_rate = param.packet_consuming_rate;

    if (param.dump_output_buffer)
      monitoring = 1;
    else
      monitoring = 0;
}


bool Traffic_sink::can_receive(void) const {
    if (!buffer.is_full())
        return true;
    else
        return false;
}

bool Traffic_sink::receive(class Flit * a_flit) {
    add(a_flit);

    if (monitoring && net_clock.get_clock() > param.warmup_period)
    {
      pPacket p = a_flit->get_packet();
      int flit_type = 1;  //assume header
      if (a_flit->is_tail())
        flit_type = 3;   //tail
      else if (!a_flit->is_header())
        flit_type = 2;  //body
      
      buffer_monitor.push_back(p->get_id());
      buffer_monitor.push_back(a_flit->get_sequence_id());
      buffer_monitor.push_back(flit_type);
      buffer_monitor.push_back(net_clock.get_clock());
      buffer_monitor.push_back(1);
      buffer_monitor.push_back(p->get_src_position().x);
      buffer_monitor.push_back(p->get_src_position().y);
      buffer_monitor.push_back(p->get_dst_position().x);
      buffer_monitor.push_back(p->get_dst_position().y);
      buffer_monitor.push_back(buffer.get_num_of_flits());

    }

    return true;
}

void Traffic_sink::tick(void) {
    // TODO: add CBR support

    // double rnd = drand48();

    if (packet_consuming_rate == 0) {
        // the consuming speed is infinite, we should remove all the packets 
        while (consume_a_packet());
    }
    //    else if (rnd < packet_consuming_rate) {
    else if (drand48() < packet_consuming_rate) {
        consume_a_packet();
    }
}


// consume a packet in the buffer. If there is no packet to consume, return 0
bool Traffic_sink::consume_a_packet(void) {
    pPacket packet = get_packet();

    if (packet) {
        packet->set_death_time(net_clock.get_clock());

        if (param.verbose)
            cout << "[I] Consumed a packet with id " << packet->get_id() 
                 << " from " << packet->get_src_position() << " to " << packet->get_dst_position() 
                 << " at sink " << id << " latency = " << packet->get_latency() 
		 << " network_latency = " << packet->get_network_latency() 
                 << " (time = " << net_clock.get_clock() << ")\n";

        // Performance data collection
	param.n_of_packets = param.n_of_packets-1; //UYO
	if (param.print_n_of_packets) {
	  param.packets_t << net_clock.get_clock() <<"\t" << param.n_of_packets << endl;
	}

        if (net_clock.get_clock() > param.warmup_period) {
            int latency = packet->get_latency();
            assert(latency > 0);
            
            param.n_of_received_packets ++;
            param.total_latency += latency;
	 
	    param.n_t = ((param.n_t*param.n_cnt)+param.n_of_packets)/(param.n_cnt+1); // UYO
	    param.n_cnt = param.n_cnt+1;  //UYO
	    //	    printf("%f  %f  %f \n",param.n_cnt,param.n_of_packets,param.n_t);

  //begin by rtg
  pPacket p = packet;
  for (int i = 0; (unsigned)i < p->size(); i++)
  {
    pFlit a_flit = p->get_flit(i);
    int flit_type = 1;  //assume header
    if (a_flit->is_tail())
      flit_type = 3;   //tail
    else if (!a_flit->is_header())
      flit_type = 2;  //body
      
    buffer_monitor.push_back(p->get_id());
    buffer_monitor.push_back(a_flit->get_sequence_id());
    buffer_monitor.push_back(flit_type);
    buffer_monitor.push_back(net_clock.get_clock());
    buffer_monitor.push_back(2);
    buffer_monitor.push_back(p->get_src_position().x);
    buffer_monitor.push_back(p->get_src_position().y);
    buffer_monitor.push_back(p->get_dst_position().x);
    buffer_monitor.push_back(p->get_dst_position().y);
    buffer_monitor.push_back(buffer.get_num_of_flits());
  }
  //end by rtg
        }

        param.on_off_complete_time =  net_clock.get_clock();
        
        delete packet;
        return true;
    }
    else
        return false;
}
