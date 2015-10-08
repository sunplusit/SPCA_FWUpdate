##############################
# Makefile
##############################

# compiler
CC = gcc
CPP = g++
ARMCC = arm-linux-gnueabihf-gcc

# include path
INCLUDE_PATH = ./include
# library path
LIBRARY_PATH = ./lib/x64 
# use library
USE_LIB = -lusb-1.0 -lspca

# compliler option
CFLAGS = -Wall -O2 -c -I$(INCLUDE_PATH)
CPPFLAGS = $(CFLAGS)
# linker option	
LFLAGS = -L$(LIBRARY_PATH)

# build shared library
LDFLAGS = -fPIC 
LDADD = -lrt -pthread -ludev

# source 
SRC = main.cpp

ifeq ($(shell uname -m), x86_64)
	LFLAGS = -L./lib/x64
else
	LFLAGS = -L./lib/x86
endif

USE_LIB = -lspca -lusb-1.0

APP_NAME=SPCA_FWUpdate

all:
	$(CPP) $(SRC) $(LFLAGS) $(USE_LIB) $(LDADD) -o $(APP_NAME)
	chmod 755 $(APP_NAME)
	strip $(APP_NAME)
	
clean:
	rm -f *.a *.o *.so core $(APP_NAME)
