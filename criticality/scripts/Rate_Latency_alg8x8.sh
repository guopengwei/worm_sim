#!/bin/bash
########################################################################
# Author: Umit Ogras  (uogras@ece.cmu.edu)
# File name: motivation_exp.sh
# Date Written:  <Wed, Jan, 12 13:56:25 2004>
# Last Modified: ...
# Description: Generate mesh with random mappings and simulate these
# architectures.
########################################################################

# Total number of configurations/runs
total_runs=17;
rows=8;
cols=8;
rate_control=0.001;
thresh_hold=80;

i=0;
while [ $i -lt $total_runs ]
  do
  echo "Running iteration $i with rate $rate_control;"  
#  echo "Generating new config file for hotspot traffic..."
 ./hotspot_config.pl $rows $cols 10 30 50 0.5 0.65 0.39 0.1 $rate_control >../config_files/hotspot_$rows.config

#  echo "Running worm sim"
  ../../worm_sim -flits_per_packet 5 -input_channel_buffer_size 2 \
      -num_of_rows $rows -num_of_columns $cols -packet_generating_rate 0 \
      -config ../config_files/hotspot_$rows.config ../config_files/free_links_8x8.config -random_seed $i -simulation_length 50000 -use_orion_power_model
 
   i=`echo "$i+1" | bc`;

  if [ $i -lt $thresh_hold ]
      then
        rate_control=`echo "$rate_control+0.01" | bc`;
      else
        rate_control=`echo "$rate_control+0.002" | bc`;
  fi

done
