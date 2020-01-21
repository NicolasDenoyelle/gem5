'''
Test file for numa (mbind get_mempolicy)
'''
import os
import sys
import subprocess

base_dir = os.path.abspath(os.path.dirname(__file__))

from testlib import *

''' building benchmarks '''
class Make:
    cc = 'gcc'
    cflags = ['-O0', '-Wall', '-Wextra', '-Werror']
    # isas and their gcc compile flags.
    cc_isa = { constants.x86_tag: ('-march=x86-64'),
               constants.arm_tag: ('-march=armv8-a'),
               constants.riscv_tag: ('-march=rv32if') }
    srcdir = os.path.join(base_dir, 'progs')
    builddir = os.path.join(base_dir, 'progs')

    ''' Get target name. '''
    @classmethod
    def target_name(cls, workload, isa):
        return '{}-{}'.format(workload,isa)

    ''' Get target bin file. '''
    @classmethod
    def target_bin(cls, workload, isa):
        return os.path.join(cls.builddir, cls.target_name(workload, isa))

    ''' Get target source file. '''
    @classmethod
    def target_source(cls, workload, extension=".c"):
        return os.path.join(cls.srcdir, workload) + extension

    ''' Build target. '''
    @classmethod
    def make(cls, workload=[], isas=[]):
        for w in workload:
            for isa in isas:
                src = cls.target_source(w)
                bin = cls.target_bin(w, isa)
                cmd = [ cls.cc ] + \
                      cls.cflags + \
                      [ '{}'.format(cls.cc_isa[isa]), src, '-o', bin ]
                log_call(log.test_log, cmd, stderr=sys.stderr)

    ''' Clean all targets '''
    @classmethod
    def clean(cls, workload=[], isas=[]):
        for w in workload:
            for isa in isas:
                bin = cls.target_bin(w, isa)
                cmd = [ 'rm', '-f', bin ]
                log_call(log.test_log, cmd, stderr=sys.stderr)

test_script = os.path.join(base_dir, 'machine.py')
workloads = [ 'mbind' ]
valid_isas = [ constants.x86_tag ]

for isa in valid_isas:
    for w in workloads:
        Make.make([ w ], [ isa ])
        gem5_verify_config(
            name=Make.target_name(w, isa),
            verifiers=(),
            config=test_script,
            config_args = [ '--run', Make.target_bin(w, isa) ],
            valid_isas=(isa,),)
# Make.clean([ w ], [ isa ])
