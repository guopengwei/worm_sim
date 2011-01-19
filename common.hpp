/************************************************************************
Author: Jingcao Hu  (jingcao@ece.cmu.edu)
File name: common.hpp
Date Created:  <Tue Oct 14 14:39:29 2003>
Last Modified: <Wed Jan 19 15:43:28 2005>
Description: 


************************************************************************/

#ifndef COMMON_HPP_
#define COMMON_HPP_

#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include <utility>

#define trace_it(s) cout<<#s<<" :"<<s<<endl;
#define MAX_LINE 1024

using namespace std;


typedef struct Position {
	unsigned int x;
	unsigned int y;
} Position, *pPosition; 


//added by Daniel Wei to allow different ATG start running at different time
typedef struct ATG {
	vector<int> nodes;
	vector<char*> config_files;
	long int time_start;
	long int time_end;
	bool running;
} ATG, *pATG;

typedef struct ATGSets{
	vector<pATG> sets;
	int *currentnodes;
	vector<pATG> currentATG;
}ATGSets, *pATGSets;

typedef enum Direction { local = 0, north, south, east, west, extra, invalid_dir } Direction; // UYO: Add extra direction

typedef enum Topology { mesh = 0, torus, invalid_topology } Topology;

typedef enum Routing_scheme { xy = 0, 
	oe, 
	oe_fixed, 
	dyad,
	test,
	predict,
	rtable,   // UYO
	invalid_scheme
} Routing_scheme;

typedef enum Traffic_mode { uniform = 0,      // uniformly distributed destination
	transpose1,       // (i,j) -> (N-j, N-i)
	transpose2,       // (i,j) -> (j->i)
	hotspot,
	invalid_distribution 
} Traffic_mode;

bool operator ==(const Position & pos1, const Position & pos2);
bool operator !=(const Position & pos1, const Position & pos2);

ostream & operator <<(ostream & os, const Position & pos);

Direction reverse(Direction dir);


// these are from popnet project. need to decide what to keep
enum mess_type {EVG_, ROUTER_, WIRE_, CREDIT_};
enum routing_type {XY_ , TXY_ };
enum VC_state_type {INIT_, ROUTING_, VC_AB_, SW_AB_, SW_TR_, HOME_};
enum flit_type {HEADER_, BODY_, TAIL_};
enum vc_share_type {SHARE_, MONO_};
enum VC_usage_type {USED_, FREE_};
typedef double time_type;
typedef vector<long> add_type;
typedef pair<long, long> VC_type;
typedef vector<unsigned long long> Data_type;
typedef unsigned long long Atom_type;
const VC_type VC_NULL = VC_type(-1, -1);
#define BUFF_BOUND_ 100
#define WIRE_DELAY_ 0.9
#define PIPE_DELAY_ 1.0
#define CREDIT_DELAY_ 1.0
#define REPORT_PERIOD_ 2000
#define S_ELPS_ 0.00000001
#define ZERO_ 0
#define MAX_32_ static_cast<unsigned long>(0xffffffff)
#define MAX_64_ static_cast<unsigned long long>(0xffffffffffffffff)
#define CORR_EFF_ 0.8
#define POWER_NOM_ 1e9


#endif  // COMMON_HPP_
