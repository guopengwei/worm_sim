./worm_sim -traffic_mode transpose1 -input_channel_buffer_size 5 -output_channel_buffer_size 5 -routing_scheme xy -packet_generating_rate 0.05 -flits_per_packet 5 -simulation_length 5000

./worm_sim -traffic_mode transpose1 -input_channel_buffer_size 5 -output_channel_buffer_size 5 -routing_scheme oe -packet_generating_rate 0.06 -flits_per_packet 5 -simulation_length 5000

./worm_sim -traffic_mode transpose1 -input_channel_buffer_size 5 -output_channel_buffer_size 5 -routing_scheme dyad -packet_generating_rate 0.02 -flits_per_packet 5 -simulation_length 5000

./worm_sim -traffic_mode transpose1 -input_channel_buffer_size 5 -output_channel_buffer_size 5 -routing_scheme test -packet_generating_rate 0.03 -flits_per_packet 5 -simulation_length 5000
