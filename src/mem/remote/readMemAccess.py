#!/usr/bin/env python3

from gem5 import Buffer
from gem5.io import Client
from gem5.message import MemAccess, MBind

sim = Client()

## In python default argument are like static variables.
## One instance of buffer exists somewhere for the lifetime of
## the process.
def get_message(buffer = Buffer(128)):
    uid = sim.receive(buffer)

    if uid == MemAccess.uid():
        msg = MemAccess()
        msg.from_bytes(buffer.ptr())
        return msg
    else:
        raise ValueError("Unexpected message id: {}".format(uid))

def send_message(msg):
    sim.send(msg)
    sim.flush()

n = 0
while True:
    try:
        msg = get_message()
    except ValueError:
        break
    n+=1
    if isinstance(msg, MemAccess):
        print('received {}: \n'.format(msg.__class__.__name__) +
              '\ttick {}\n'.format(msg.tick()) +
              '\taddress {}\n'.format(hex(msg.address())) +
              '\tnuma node {}\n'.format(msg.numa_node()) +
              '\thardware thread {}\n'.format(msg.threadid()) +
              '\ttype {!s}\n'.format(msg.type()))

    if n % 100 == 0:
        send_message(MBind(msg.address(), 4096, 1, MBind.Mode.MPOL_BIND))
