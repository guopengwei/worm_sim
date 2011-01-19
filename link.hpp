/************************************************************************
 Author: Jingcao Hu  (jingcao@ece.cmu.edu)
 File name: link.hpp
 Date Created:  <Tue Oct 14 13:41:20 2003>
 Last Modified: <Wed Oct  6 16:21:14 2004>
 Description: Link class declaration. A link is one connect two buffer-owners
************************************************************************/

#ifndef LINK_HPP_
#define LINK_HPP_

#include "misc.hpp"

typedef class Link : public Timer_owner, public Connector {
private:
    int   delay;           // how many clock cycles does it take to traverse the link
    double e_bit;           // average energy consumption of sending one bit of data

public:
//    Link(int d = 0) { delay = d; source = sink = 0; e_bit = param.link_ebit; }
  Link(class Connector * src, class Connector * dst, int d = 0);
  ~Link() {}
  bool can_send(void) const;
  bool can_receive(void) const;
  bool send(void);
  bool receive(class Flit * a_flit);
  void set_ebit(double e) { e_bit = e; }

} Link, *pLink;


#endif  // LINK_HPP_


