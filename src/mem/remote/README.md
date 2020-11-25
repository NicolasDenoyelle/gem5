# Requirements:

* Unlike gem5, python3 is used to compile and run the bindings.
* You need to install pybind11:

``` sh
python -m pip install pybind11
```

# Compiling:

``` sh
make
```

You will also need to install the created python module to be able to 
run the bindings:

``` sh
make install
```

# Testing:

## Implementation
The C++ implementation of the communication layer tests are in
tests.cc. Once compiled you can run them with:

``` sh
./tests
```

## Bindings

You need to make sure you were able to compile and install the bindings.
The server is in C++ and meant to be integreated into gem5 C++ side.
You have to start it first with 

``` sh
./server
```

Once the server is started, you can start the client:

``` sh
./client.py
```

There's no output to expect on success.
Both program should terminate without aborting.

## Gem5

There is now an option in se.py to spawn a server and send all memory access
to a client.

From gem5 root directory, start the simulation with:
``` sh
./build/X86/gem5.debug ./configs/example/se.py --server-id 0 ...
```

The server should be waiting on a client to connect:
> System server waiting on a client.

Then start a client from the same directory (because this is where the socket)
is connected with the same identifier (`0`). For instance:

``` sh
src/mem/remote/readMemAccess.py
```
