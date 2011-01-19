/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: link.cpp
 Date Created:  <Tue Oct 14 13:41:56 2003>
 Last Modified: <Wed Jan 19 18:25:07 2005>
 Description: 
************************************************************************/

#include "link.hpp"
#include "pkt.hpp"
#include "router.hpp"
#include "repeater.hpp"

Link::Link(pConnector src, pConnector dst, int d) : Connector(src, dst) {
    delay = d;
    e_bit = param.link_ebit;
}

// Link is a dumb device, so it does not initiate the sending itself, instead
// it will just pass whatever data to the receiver of the link
bool Link::can_send(void) const {
    return source->can_send();
}


bool Link::can_receive(void) const {
    return sink->can_receive();
}


// Link is a passive device, so it will not initiate a send action at all by itself.
// Instead, as soon as it receives a flit, it sends it out immediately. So the following
// function should never be invoked
bool Link::send(void) {
#ifdef DEBUG
    assert(0);
#endif
    return true;
}


// Immediately after the link receive the flit, it sends out the flit instanenously.
// When simulating systems with link error, here is the place to inject/modify links.
// Each power module in the router will take care of all the incoming links power 
// consumption together with the power consumption of the outgoing link to the local
// PE
bool Link::receive(class Flit * a_flit) {
  sink->receive(a_flit);
  pRouter router = 0;
  pRepeater repeater = 0;
  if (net_clock.get_clock() > param.warmup_period) {
    int dir = -1;

        if (dynamic_cast<Traffic_source *>(source))  // local link does consume zero energy
            return true;

        if (dynamic_cast<In_port *>(sink)) {
            pIn_port p = (pIn_port) sink;
            router = p->get_router();
	    repeater = p->get_repeater();
            dir = (int) p->get_direction();
        }
        /*
        else if (dynamic_cast<Traffic_source *>(source)) {
            assert(dynamic_cast<Out_port *>(source));
            pOut_port p = (pOut_port) source;
            router = p->get_router();
            dir = param.n_of_ports;
        }
        */

        // the link energy consumption of a flit from local outport to local PE is zero
        if (router || repeater) {
            param.energy.link_energy += e_bit * a_flit->get_flit_size();
            if (param.orion_power.use_orion_power_model) {
	      pPower_module pm ;
	      if (router)
		pm = router->get_power_module();
	      else
		pm = repeater->get_power_module();
	      pm->power_link_traversal(dir, a_flit->raw());
            }
        }
  }

    return true;
}
