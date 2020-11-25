#!/usr/bin/env python3

from gem5 import Buffer
from gem5.io import Client
from gem5.message import MemAccess, MBind

buffer = Buffer(128)
msg = MemAccess(MemAccess.Type.RW, 232830, 0, 1, 4354353)
mb = MBind(0x435436534, 4096*16, 0x2, MBind.Mode.MPOL_INTERLEAVE, 0);
chk = MemAccess()

client = Client()
client.receive(buffer)
client.send(mb);

chk.from_bytes(buffer.ptr())
assert(chk == msg)
