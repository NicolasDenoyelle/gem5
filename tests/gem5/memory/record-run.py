import os
import sys
import argparse
import subprocess
from math import log
import m5
import ast
from m5.objects import *
from m5.params import *

"""
Instanciate memories and connect them to the system membus.
"""
class SystemFactory:
    addr = 0
    page_size = 4096
    valid_mem = { 'SimpleMemory': SimpleMemory,
                  'DDR3_1600_8x8': DDR3_1600_8x8,
                  'HBM_1000_4H_1x64': HBM_1000_4H_1x64 }

    @staticmethod
    def _mem_from_attr_(type = 'SimpleMemory',
                      size = '8MB',
                      **kwargs):
        if type not in SystemFactory.valid_mem.keys():
            print("type {} must be a valid memory class in {}."\
                  .format(cls, str(SystemFactory.valid_mem.keys())))
            raise ValueError;
        cls = SystemFactory.valid_mem[type]

        size = MemorySize(size).getValue()
        if size % SystemFactory.page_size:
            print("Size must be a multiple of page_size: {}"\
                  .format(SystemFactory.page_size))
            raise ValueError;
        SystemFactory.addr += size
        return cls(range = AddrRange(start = (SystemFactory.addr-size),
                                     size = size),
                   **kwargs)

    @staticmethod
    def new(memories = [ 'SimpleMemory' ], clock='2GHz'):
        num_hw_threads = 1
        system = System(multi_thread = (num_hw_threads > 1))
        system.clk_domain = SrcClockDomain()
        system.clk_domain.clock = clock
        system.clk_domain.voltage_domain = VoltageDomain()
        system.mem_mode = 'timing'
        system.membus = SystemXBar()
        system.record = RecordMemory()
        system.record.slave = system.membus.master

        # Connect CPU
        system.cpu = TimingSimpleCPU(numThreads=num_hw_threads)
        system.cpu[0].createInterruptController()
        for i in range(num_hw_threads):
            system.cpu[0].interrupts[i].pio = system.membus.master
            system.cpu[0].interrupts[i].int_slave = system.membus.master
            system.cpu[0].interrupts[i].int_master = system.membus.slave
        system.system_port = system.membus.slave
        system.cpu.dcache_port = system.membus.slave
        system.cpu.icache_port = system.membus.slave

        # Connect memories
        mems = []
        for string in memories:
            print(string)
            if "=" in string:
                cls_name, cls_attr = string.split("=")
                attr = ast.literal_eval(cls_attr)
                mem = SystemFactory._mem_from_attr_(cls_name, **attr)
            else:
                mem = SystemFactory._mem_from_attr_(string)
            mem.port = system.record.master
            mems.append(mem)
        system.memories = mems
        return system

##############################################################
# Command line options
##############################################################

parser = argparse.ArgumentParser(description="Run a subprocess through a " +
                                 "simulated NUMA architecture.")
parser.add_argument('--mem',
                    action='append',
                    nargs='+',
                    type=str,
                    help="""Configure the memory subsystem with a list of
                    memories. Memory can be either be a memory name: """ +
                    str(SystemFactory.valid_mem.keys()) + """Or, a memory
                    with attributes name={'attr0':'val0',...}. Attributes
                    can be 'size' or any valid attribute for the memory kind
                    used""")
parser.add_argument('--run', nargs='+', required=True,
                    help="""The command line of the subprocess.
                    Must be aboslute path.""")

args = parser.parse_args()

# Subprocess check
if not os.path.isabs(args.run[0]):
    sys.exit("Suprocess binary must have an absolute path.")
# parse memories
if args.mem is None:
    args.mem = [[ 'DDR3_1600_8x8', 'HBM_1000_4H_1x64' ]]

##############################################################
# Create system
##############################################################

system = SystemFactory.new(memories = [ m for mems in args.mem for m in mems ])
root = Root(full_system = False, system = system)

process = Process()
process.cmd = args.run
system.cpu.workload = process
system.cpu.createThreads()

m5.instantiate()
print("Beginning simulation!")
exit_event = m5.simulate()
print('Exiting @ tick {} because {}'
      .format(m5.curTick(), exit_event.getCause()))
