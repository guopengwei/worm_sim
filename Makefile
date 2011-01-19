########################################################################
# Author: Jingcao Hu
# File name: Makefile
# Date Created:  <Mon Oct 13 11:27:05 2003>
# Last Modified: <Wed Jan 19 12:17:21 2005>
# Description: Makefile for worm-sim project
########################################################################

POWER_RELEASE = orion_power_beta_mar2003
LIBPERF = libperf

OBJS = routing_engine.o channel.o pkt.o misc.o worm_sim.o switch_fabric.o \
	arbiter.o link.o router.o network.o traffic.o port.o util.o power.o repeater.o

HDRS = version.hpp msg.hpp channel.hpp misc.hpp pkt.hpp routing_engine.hpp \
	switch_fabric.hpp arbiter.hpp link.hpp router.hpp network.hpp traffic.hpp \
	common.hpp port.hpp util.hpp power.hpp repeater.hpp

SRCS = channel.cpp misc.cpp pkt.cpp routing_engine.cpp worm_sim.cpp \
	switch_fabric.cpp arbiter.cpp link.cpp router.cpp network.cpp traffic.cpp \
	port.cpp util.cpp power.cpp repeater.cpp

#LIBPERF_SRCS = $(LIBPERF)/perf_analysis.m $(LIBPERF)/equations.m $(LIBPERF)/calc_latency.m

EXEC = worm_sim

#CC = g++296
CC = g++ -lrt

OPT = -Wall -O0 -fomit-frame-pointer -fexpensive-optimizations -fschedule-insns -fschedule-insns2 \
	-DPOWER_TEST -I./$(POWER_RELEASE)/power -I./$(POWER_RELEASE)/library -ggdb

# OPT = -Wall -O3 -fomit-frame-pointer -fexpensive-optimizations -fschedule-insns -fschedule-insns2 \
# 	-DPOWER_TEST -I./$(POWER_RELEASE)/power -I./$(POWER_RELEASE)/library -I./$(LIBPERF) \
# 	-I$(MATLAB_ROOT)/extern/include/cpp -I$(MATLAB_ROOT)/extern/include \

# LINKFLAGS = -O -pthread  -L. -L/opt/jingcao/matlab7/bin/glnx86 -L./$(POWER_RELEASE)/power\
# 	-L./$(LIBPERF) -Wl,--rpath-link,/opt/jingcao/matlab7/bin/glnx86 \
# 	-lm -lmwmclmcrrt -lpower -lperf

LINKFLAGS = -O -L. -L./$(POWER_RELEASE)/power -lpower

# LINKFLAGS = -O -L. -L$(MATLAB_ROOT)/bin/glnx86 -L./$(POWER_RELEASE)/power\
# 	-L./$(LIBPERF) -Wl,--rpath-link,$(MATLAB_ROOT)/bin/glnx86 \
# 	-lm -lmwmclmcrrt -lpower -lperf

POWER_LIB = libpower.a

# PERF_LIB = libperf.so


all: TAGS $(EXEC) $(POWER_LIB) # $(PERF_LIB)

TAGS: $(HDRS) $(SRCS) 
	etags $(HDRS) $(SRCS)
	find ./ -name "*.c" -or -name "*.h" -exec etags -a {} \;

channel.o: $(HDRS) channel.cpp
	$(CC) $(OPT) -c channel.cpp

pkt.o: $(HDRS) pkt.cpp
	$(CC) $(OPT) -c pkt.cpp

routing_engine.o: $(HDRS) routing_engine.cpp
	$(CC) $(OPT) -c routing_engine.cpp

misc.o: $(HDRS) misc.cpp
	$(CC) $(OPT) -c misc.cpp

link.o: $(HDRS) link.cpp
	$(CC) $(OPT) -c link.cpp

switch_fabric.o: $(HDRS) switch_fabric.cpp
	$(CC) $(OPT) -c switch_fabric.cpp

arbiter.o: $(HDRS) arbiter.cpp
	$(CC) $(OPT) -c arbiter.cpp

router.o: $(HDRS) router.cpp
	$(CC) $(OPT) -c router.cpp

repeater.o: $(HDRS) repeater.cpp
	$(CC) $(OPT) -c repeater.cpp

network.o: $(HDRS) network.cpp #libperf.h
	$(CC) $(OPT) -c network.cpp

traffic.o: $(HDRS) traffic.cpp
	$(CC) $(OPT) -c traffic.cpp

port.o: $(HDRS) port.cpp
	$(CC) $(OPT) -c port.cpp

util.o: $(HDRS) util.cpp
	$(CC) $(OPT) -c util.cpp

power.o: $(HDRS) power.cpp
	$(CC) $(OPT) -c power.cpp

worm_sim.o: $(HDRS) worm_sim.cpp
	$(CC) $(OPT) -c worm_sim.cpp

# libperf.h: $(PERF_LIB)
# 	cd ./$(LIBPERF); $(MAKE) 

$(POWER_LIB):
	cd ./$(POWER_RELEASE)/power; $(MAKE) test_router

# $(PERF_LIB): $(LIBPERF_SRCS)
# 	cd ./$(LIBPERF); $(MAKE) 

$(EXEC): $(OBJS) $(POWER_LIB) # $(PERF_LIB)
	$(CC) -o $(EXEC) $(OBJS) $(LINKFLAGS) 

.PHONY: clean

clean:
	rm -f $(EXEC) $(OBJS) *~ TAGS
	rm -rf ./$(LIBPERF)_mcr
	cd ./$(POWER_RELEASE)/power; $(MAKE) clean
#	cd ./$(LIBPERF); $(MAKE) clean
