/************************************************************************
 Author: Umit Ogras  (uogras@ece.cmu.edu)
 File name: add_long)link.cpp
 Date Written:  <Mon Feb 28 19:16:26 2005>
 Last Modified:
 Description: Add long link to mesh network based on input traffic
 Usage:
 ./add_lang_link rows cols wiring_constraint traffic.config arch.config print_out
************************************************************************/

#include <iostream>
#include <fstream>
#include <cstdio>
#include <vector>

#define MAX_LINE 1024
using namespace std;

int gen_arch_config(int rows,int cols,int count,vector<int> long_links,char *arch_file);
double free_packet_delay(int row, int col, char *traf_config, char *arch_config);

int main(int argn, char **argv) {

    int *output=0;
    int row,col;
    int N = 0;
    int constraint=0;
    int utilization=0;
    char * arch_file=0;
    char input_line[MAX_LINE];
    char * sub_string;
    int * RLink; 
    bool print_out = 0;

    vector <int> long_links;
    if (argn == 7)
      print_out = 1;
    else
      print_out = 0;

    if ( !sscanf(argv[1],"%d",&row) || !sscanf(argv[2],"%d",&col) )
      cerr <<"Network size is not specified! \n";
    N = row*col;
    
    if ( !sscanf(argv[3],"%d",&constraint) )
      cerr <<"Constraint is not specified! \n";

    /*ofstream tr_config(argv[4]);
    if (!tr_config.is_open() || !tr_config.good())
      { cerr <<"Traffic file cannot be opened \n";
        return -1;
	}*/
 
    arch_file = argv[5];  // The file for the architecture & routing info

    double min_rate=0;
    double curr_rate=0;
    int best_id[2]={0,0};
    
    
    // Main loop: Continue until the utilization is less tha the
    // constraint.
    // Each time find the most beneficial link to be added
    bool link_add [N];
    for (unsigned int r=0; r<N; r++)
      //	for (unsigned int c=0; c<N; c++)
	  link_add[r]=0;    // No link is added initially
    
    unsigned int count = 2;    // Number of long links already added, each node can have at most one long link

    while ( utilization < constraint ) {
      min_rate = 1000;
      // Try all possible additions of links
      for (unsigned int m=0; m<N; m++) {
	for (int n=0; n<N; n++) {
	  if ( link_add[m] || link_add[n] || (m==n) )
	    continue;                      // Already added 
     
	  // Generate config file with this link added
	  long_links.push_back(m);
	  long_links.push_back(n);
          // Write architecture config file with routing info to argv[5]
	  //printf("Now trying (%d,%d):\n",m,n);
	  gen_arch_config(row,col,count,long_links,argv[5]);
	  //cout <<"Architecture generated \n";
	  // Evaluate current configuration
	  curr_rate = free_packet_delay(row,col,argv[4],argv[5]); 
	  //	  printf("The free packet delay is found as (%d,%d):%f\n",m,n,curr_rate);
	  if ( curr_rate < min_rate ) { // A better configuration is found
	    best_id[0] = m;
	    best_id[1] = n;
	    min_rate = curr_rate;
	  } // if
	  long_links.pop_back();
	  long_links.pop_back();
	} // for c
      } // for r
      cout <<"cekirge 1:" << best_id[0] <<" " << best_id[1] <<endl;
      long_links.push_back(best_id[0]);
      long_links.push_back(best_id[1]);
      link_add[best_id[0]] = 1;
      link_add[best_id[1]] = 1;
      count += 2;
      int dy = abs( (best_id[0] % col)-(best_id[1] % col));
      int dx =abs( ((int) (best_id[0]/row))-((int) (best_id[1]/row)) );
      utilization = utilization + dy + dx;
    } // while	  
    for (int h=0;h<long_links.size();h++)
      printf("%d ",long_links[h]);
    printf("%d \n",count);
    gen_arch_config(row,col,count-2,long_links,argv[5]);
}
