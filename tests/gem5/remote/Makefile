CC=g++
CPPFLAGS=-Wall -Wextra -I../../../build/X86 -I../../../build/ARM
.PHONY:= all clean

SRC=../../../src/mem/remote/io.cc ../../../src/mem/remote/message.cc
OBJ=$(patsubst %.cc, %.o, $(SRC))
TESTS=server.cc tests.cc
BIN=$(patsubst %.cc, %, $(TESTS))

all: $(OBJ) $(BIN)

%.o: %.cc
	$(CC) $(CPPFLAGS) -c $< -o $@

%: %.cc $(OBJ)
	$(CC) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJ) $(BIN)
