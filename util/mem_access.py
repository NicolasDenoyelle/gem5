#!/usr/bin/python2
import os
import protolib
import subprocess

util_dir = os.path.dirname(os.path.realpath(__file__))
# Make sure the proto definitions are up to date.
subprocess.check_call(['make', '--quiet', '-C', util_dir, 'packet_pb2.py'])
import packet_pb2

# Import the packet proto definitions
try:
    import inst_pb2
except:
    print "Did not find protobuf inst definitions, attempting to generate"
    from subprocess import call
    error = call(['protoc', '--python_out=util', '--proto_path=src/proto',
                  'src/proto/inst.proto'])
    if not error:
        print "Generated inst proto definitions"

        try:
            import google.protobuf
        except:
            print "Please install Python protobuf module"
            exit(-1)

        import inst_pb2
    else:
        print "Failed to import inst proto definitions"
        exit(-1)

try:
    import inst_dep_record_pb2
except:
    print "Did not find proto definition, attempting to generate"
    from subprocess import call
    error = call(['protoc', '--python_out=util', '--proto_path=src/proto',
                  'src/proto/inst_dep_record.proto'])
    if not error:
        import inst_dep_record_pb2
        print "Generated proto definitions for instruction dependency record"
    else:
        print "Failed to import proto definitions"
        exit(-1)

CMDS = {
    1: 'InvalidCmd',
    2: 'ReadReq',
    3: 'ReadResp',
    4: 'ReadRespWithInvalidate',
    5: 'WriteReq',
    6: 'WriteResp',
    7: 'WriteCompleteResp',
    8: 'WritebackDirty',
    9: 'WritebackClean',
    10: 'WriteClean',
    11: 'CleanEvict',
    12: 'SoftPFReq',
    13: 'SoftPFExReq',
    14: 'HardPFReq',
    15: 'SoftPFResp',
    16: 'HardPFResp',
    17: 'WriteLineReq',
    18: 'UpgradeReq',
    19: 'SCUpgradeReq',
    20: 'UpgradeResp',
    21: 'SCUpgradeFailReq',
    22: 'UpgradeFailResp',
    23: 'ReadExReq',
    24: 'ReadExResp',
    25: 'ReadCleanReq',
    26: 'ReadSharedReq',
    27: 'LoadLockedReq',
    28: 'StoreCondReq',
    29: 'StoreCondFailReq',
    30: 'StoreCondResp',
    31: 'SwapReq',
    32: 'SwapResp',
    35: 'MemFenceReq',
    33: 'MemSyncReq',
    34: 'MemSyncResp',
    36: 'MemFenceResp',
    37: 'CleanSharedReq',
    38: 'CleanSharedResp',
    39: 'CleanInvalidReq',
    40: 'CleanInvalidResp',
    41: 'InvalidDestError',
    42: 'BadAddressError',
    43: 'FunctionalReadError',
    44: 'FunctionalWriteError',
    45: 'PrintReq',
    46: 'FlushReq',
    47: 'InvalidateReq',
    48: 'InvalidateResp',
    49: 'HTMReq',
    50: 'HTMReqResp',
    51: 'HTMAbort',
    52: 'NUM_MEM_CMDS',
}

def make_cmd(cmd):
        f = next((i for i,c in CMDS.items() if c == cmd), None)
        if f is None:
            raise ValueError('Invalid Packet Command: {}'.format(cmd))
        return f

FLAGS = {
        int('0x000000FF', 16): 'COPY_FLAGS',
        int('0x00000009', 16): 'RESPONDER_FLAGS',
        int('0x00000001', 16): 'HAS_SHARERS',
        int('0x00000002', 16): 'EXPRESS_SNOOP',
        int('0x00000004', 16): 'RESPONDER_HAD_WRITABLE',
        int('0x00000008', 16): 'CACHE_RESPONDING',
        int('0x00000010', 16): 'WRITE_THROUGH',
        int('0x00000020', 16): 'SATISFIED',
        int('0x00000040', 16): 'FAILS_TRANSACTION',
        int('0x00000080', 16): 'FROM_TRANSACTION',
        int('0x00000100', 16): 'VALID_ADDR',
        int('0x00000200', 16): 'VALID_SIZE',
        int('0x00001000', 16): 'STATIC_DATA',
        int('0x00002000', 16): 'DYNAMIC_DATA',
        int('0x00008000', 16): 'SUPPRESS_FUNC_ERROR',
        int('0x00010000', 16): 'BLOCK_CACHED',
        int('0x00020000', 16): 'FROM_MEMORY',
        int('0x000000FF', 16): 'COPY_FLAGS',
        int('0x00000009', 16): 'RESPONDER_FLAGS',
        int('0x00000001', 16): 'HAS_SHARERS',
        int('0x00000002', 16): 'EXPRESS_SNOOP',
        int('0x00000004', 16): 'RESPONDER_HAD_WRITABLE',
        int('0x00000008', 16): 'CACHE_RESPONDING',
        int('0x00000010', 16): 'WRITE_THROUGH',
        int('0x00000020', 16): 'SATISFIED',
        int('0x00000040', 16): 'FAILS_TRANSACTION',
        int('0x00000080', 16): 'FROM_TRANSACTION',
        int('0x00000100', 16): 'VALID_ADDR',
        int('0x00000200', 16): 'VALID_SIZE',
        int('0x00001000', 16): 'STATIC_DATA',
        int('0x00002000', 16): 'DYNAMIC_DATA',
        int('0x00008000', 16): 'SUPPRESS_FUNC_ERROR',
        int('0x00010000', 16): 'BLOCK_CACHED',
        int('0x00020000', 16): 'FROM_MEMORY',
}

def make_flag(flag_str):
    try:
        return int(flag_str, 16)
    except ValueError:
        f = next((f for f,n in FLAGS.items() if n == flag_str), None)
        if f is None:
            raise ValueError('Invalid Packet Type: {}'.format(flag_str))
        return f

INST_TYPE = {
    0: 'None',
    1: 'IntAlu',
    2: 'IntMul',
    3: 'IntDiv',
    4: 'FloatAdd',
    5: 'FloatCmp',
    6: 'FloatCvt',
    7: 'FloatMult',
    8: 'FloatDiv',
    9: 'FloatSqrt',
    10: 'SIMDIntAdd',
    11: 'SIMDIntAddAcc',
    12: 'SIMDIntAlu',
    13: 'SIMDIntCmp',
    14: 'SIMDIntCvt',
    15: 'SIMDMisc',
    16: 'SIMDIntMult',
    17: 'SIMDIntMultAcc',
    18: 'SIMDIntShift',
    19: 'SIMDIntShiftAcc',
    20: 'SIMDSqrt',
    21: 'SIMDFloatAdd',
    22: 'SIMDFloatAlu',
    23: 'SIMDFloatCmp',
    24: 'SIMDFloatCvt',
    25: 'SIMDFloatDiv',
    26: 'SIMDFloatMisc',
    27: 'SIMDFloatMult',
    28: 'SIMDFloatMultAdd',
    29: 'SIMDFloatSqrt',
    30: 'MemRead',
    31: 'MemWrite',
    32: 'IprAccess',
    33: 'InstPrefetch',
}

def decode_packets(proto_in, flags=[], cmds=[]):
    print 'decode packet trace: %s' % args.input

    # Read the magic number in 4-byte Little Endian
    magic_number = proto_in.read(4)

    # Add the packet header
    header = packet_pb2.PacketHeader()
    protolib.decodeMessage(proto_in, header)

    packet = packet_pb2.Packet()
    all_flags = set()
    all_cmds = set()

    # Decode the packet messages until we hit the end of the file
    while protolib.decodeMessage(proto_in, packet):
        # ReadReq is 1 and WriteReq is 4 in src/mem/packet.hh Command enum
        line = ''
        if len(cmds) > 0 and \
           not next((True for c in cmds if c == packet.cmd), False):
            continue

        cmd = ','.join([v for i,v in CMDS.items() if i == packet.cmd])
        all_cmds.add(cmd)

        # if packet.HasField('pkt_id'):
        #     line += '%s ' % packet.pkt_id
        if packet.HasField('flags'):
            if len(flags) > 0 and \
               not next((True for f in flags if (f & packet.flags) == f),
                        False):
                continue
            f = ','.join([v for k,v in FLAGS.items() if k & packet.flags == k])
            all_flags.add(f)
            line += '%s %s %s %s %s ' % (packet.tick, cmd, hex(packet.addr),
                                         packet.size, f)
        else:
            if len(flags) > 0:
                continue
            line += '%s %s %s %s ' % (packet.tick, cmd, hex(packet.addr),
                                      packet.size)
        print '%s' % line

    print 'Encountered flags: %s' % str(all_flags)
    print 'Encountered commands: %s' % str(all_cmds)

def decode_insts(proto_in, flags=[]):
    print 'decode instruction trace: %s' % args.input
    # Read the magic number in 4-byte Little Endian
    magic_number = proto_in.read(4)

    # Add the packet header
    header = packet_pb2.PacketHeader()
    protolib.decodeMessage(proto_in, header)

    inst = inst_pb2.Inst()
    num_insts = 0
    while protolib.decodeMessage(proto_in,  inst):
        if not inst.HasField('type'):
            continue

        # If we have a tick use it, otherwise count instructions
        if inst.HasField('tick'):
            tick = inst.tick
        else:
            tick = num_insts
        inst_type = INST_TYPE[inst.type]

        for mem_acc in inst.mem_access:
            if len(flags) > 0 and \
               not next((True for f in flags \
                         if (f & mem_acc.mem_flags) == f), False):
                continue
            f = ','.join([v for k,v in FLAGS.items() \
                          if (f & mem_acc.mem_flags) == f])

            print('{} {} {} {} {}', tick, inst_type,
                  hex(mem_acc.addr), mem_acc.size, f)
        num_insts += 1

def decode_deps(proto_in, flags=[]):
    print 'decode dependencies trace: %s' % args.input

    # Read the magic number in 4-byte Little Endian
    magic_number = proto_in.read(4)

    if magic_number != "gem5":
        print "Unrecognized file"
        exit(-1)

    # Add the packet header
    header = inst_dep_record_pb2.InstDepRecordHeader()
    protolib.decodeMessage(proto_in, header)

    enumNames = {}
    desc = inst_dep_record_pb2.InstDepRecord.DESCRIPTOR
    for namestr, valdesc in desc.enum_values_by_name.items():
        enumNames[valdesc.number] = namestr

    packet = inst_dep_record_pb2.InstDepRecord()

    while protolib.decodeMessage(proto_in, packet):
        if not packet.HasField('p_addr'):
            continue
        # Not present in dependency trace.
        if not packet.HasField('v_addr'):
            continue
        if not packet.HasField('size'):
            continue
        if not packet.HasField('latency'):
            continue
        if not packet.HasField('tick'):
            continue

        print('{} {} {} {} {} {} {}'.format(
            packet.tick,
            enumNames[packet.type],
            hex(packet.p_addr),
            hex(packet.v_addr),
            packet.size,
            packet.latency,
            packet.comp_delay))

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input', type=str,
                        help='Protobuf input file ', required=True)
    parser.add_argument('-f', '--filter-flags', type=str, default=[],
                        action='append',
                        help='Add a set of hex flags and output only packets '
                        'with these flags set')
    parser.add_argument('-c', '--filter-cmd', type=str, default=[],
                        action='append',
                        help='Only show packets of selected commands.')
    parser.add_argument('-m', '--mode', type=str, choices=['inst', 'data',
                                                           'deps'],
                        default='deps',
                        help='Set type of trace to decode: either instruction'
                        'trace (trace) or data packets trace (data), or '
                        'instruction dependencies trace (deps).')
    args = parser.parse_args()
    flags = [ make_flag(f) for f in args.filter_flags ]
    cmds = [ make_cmd(f) for f in args.filter_cmd ]
    # Open the file in read mode
    proto_in = protolib.openFileRd(args.input)
    if args.mode == 'data':
        decode_packets(proto_in, flags, cmds)
    if args.mode == 'inst':
        decode_insts(proto_in, flags)
    if args.mode == 'deps':
        decode_deps(proto_in, flags)

    proto_in.close()

