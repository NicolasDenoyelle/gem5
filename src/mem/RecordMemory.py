from m5.params import *
from m5.objects.AbstractMemory import *

class RecordMemory(AbstractMemory):
    type = 'RecordMemory'
    cxx_header = "mem/record_mem.hh"
