import os
import sys
import argparse
import subprocess
from math import log
import m5
import ast
from m5.objects import *
from m5.params import *
from m5 import stats


##############################################################
# Command line options
##############################################################

parser = argparse.ArgumentParser(description="Record memory"
                                 "access of an application")
option = parser.add_argument
option('--size', action='append', nargs='+', type=str, default='4GB',
       help='Memory size')
option('--clock', action='append', nargs='+', type=str, default='4GHz',
       help='Processor clock speed')
option('--run', nargs='+', required=True,
       help="The command line of the subprocess Must be aboslute path.")
option("--dump-file", metavar="FILE", default="memtrace.txt",
       help="Sets the output file for statistics [Default: %default]")
args = parser.parse_args()

# Subprocess check
if not os.path.isabs(args.run[0]):
    sys.exit("Suprocess binary must have an absolute path.")

# Set values
clock = args.clock
mem_size = args.size

##############################################################
# Create system
##############################################################

system = System()
system.clk_domain = SrcClockDomain()
system.clk_domain.clock = clock
system.clk_domain.voltage_domain = VoltageDomain()
system.mem_mode = 'timing'
system.membus = SystemXBar()

# Connect CPU
system.cpu = TimingSimpleCPU()
system.cpu[0].createInterruptController()
system.cpu[0].interrupts[0].pio = system.membus.master
system.cpu[0].interrupts[0].int_slave = system.membus.master
system.cpu[0].interrupts[0].int_master = system.membus.slave
system.system_port = system.membus.slave
system.cpu.dcache_port = system.membus.slave
system.cpu.icache_port = system.membus.slave

#Connect comm_monitor
system.monitor = MemTrace()
system.monitor.slave = system.membus.master

#Connect memory
mem = SimpleMemory(range=AddrRange(start = 0, size='4GB'))
mem.port = system.monitor.master
system.memories = [ mem ]
root = Root(full_system = False, system = system)

##############################################################
# Start simulation
##############################################################

process = Process()
process.cmd = args.run
system.cpu.workload = process
system.cpu.createThreads()

m5.instantiate()
print("Beginning simulation!")
exit_event = m5.simulate()
print('Exiting @ tick {} because {}'
      .format(m5.curTick(), exit_event.getCause()))
print('Trace output to "{{m5out}}"/{}'.format(system.monitor.dump_file))
