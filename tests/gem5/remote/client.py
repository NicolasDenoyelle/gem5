#!/usr/bin/env python3

import os
from m5.io import Buffer, Client
from m5.message import MemAccess, MBind

print(os.getcwd())

buffer = Buffer(128)
msg = MemAccess(MemAccess.Type.RW, 232830, 0, 1, 4354353)
mb = MBind(0x435436534, 4096*16, 0x2, MBind.Mode.MPOL_INTERLEAVE, 0);
chk = MemAccess()

client = Client()
client.receive(buffer)
client.send(mb);

chk.from_bytes(buffer.ptr())
assert(chk == msg)
