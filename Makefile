# compiler to use
CC = g++

# flags to pass compiler
CFLAGS = -g -Wall -Werror
CXXFLAGS = -g -Wall
# -Werror

# name for executable
BIN = enable2in1screen

# space-separated list of source files
SRCS =  transform.cpp config.cpp enable2in1screen.cpp
#orientation.cpp
LIBS = -Wall -ldl -linput -ludev -ljsoncpp -liio -lX11 -lXi -lXrandr -lXinerama

# automatically generated list of object files
OBJS = $(SRCS:.cpp=.o)

all: $(BIN)
	chmod a+x $(BIN)

# binarie
$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)
	
# dependencies
$(OBJS): Makefile

clean: 
	rm -f $(OBJS)

mrproper: clean
	rm -f $(BIN)