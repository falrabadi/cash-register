CC = g++
CFLAGS = -std=c++11 -Wall
LIBS = -lsqlite3

SRCS = cashier.cpp
OBJS = $(SRCS:.cpp=.o)
EXEC = cashier

.PHONY: all clean

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC) $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)