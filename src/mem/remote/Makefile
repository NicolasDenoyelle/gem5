##
# Remote Memory Controller
#
# @file
# @version 0.1

CC=g++
CPPFLAGS=-Wall -g -O0 -I../..
LDFLAGS=-pthread

SRC=message.cc io.cc
OBJ=$(patsubst %.cc, %.o, $(SRC))

TESTS=tests.cc server.cc
BIN=$(patsubst %.cc, %, $(TESTS))

.PHONY: all clean pybind install

all: $(OBJ) $(BIN) pybind

%.o: %.cc
	$(CC) $(CPPFLAGS) -c $< -o $@

%: %.cc $(OBJ)
	$(CC) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

pybind:
	python setup.py build

install:
	python setup.py install --user

check: $(BIN) pybind install
		@echo "Unit tests..."
		@./tests
		@echo "Python wrapper tests..."
		@./server & sleep 1 && python3 client.py

clean:
	python setup.py clean
	rm -rf $(OBJ) $(BIN) tmp build dist *.egg-info

# end
