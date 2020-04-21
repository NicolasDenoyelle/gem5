
from m5.params import *
from m5.proxy import *
from m5.objects.System import System
from m5.SimObject import SimObject

# The communication monitor will most typically be used in combination
# with periodic dumping and resetting of stats using schedStatEvent
class MemTrace(SimObject):
    type = 'MemTrace'
    cxx_header = "mem/memtrace.hh"

    system = Param.System(Parent.any, "System that the monitor belongs to.")

    # one port in each direction
    master = MasterPort("Master port")
    slave = SlavePort("Slave port")

    dump_file = Param.String('memtrace.txt', '# Trace output file.')
