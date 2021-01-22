from trex_stl_lib.api import *

import ipaddress as ipa

def emb_v6_in_v4(v6str, v4str, prefix):
    v6 = ipa.ip_address(v6str).packed
    v4 = ipa.ip_address(v4str).packed
    off = prefix // 8
    shf = prefix % 8
    return emb_byte_in_byte(bytearray(v6), v4, len(v4), off, shf)

def emb_byte_in_byte(dbyte, sbyte, size, offset, shift):
    for i in range(size):
        if shift !=0 :
            dbyte[offset+i] |= (sbyte[i] >> shift)&0xff
            dbyte[offset+1+i] |= (sbyte[i] << (8 - shift))&0xff
        else:
            dbyte[offset+i] = sbyte[i]
    return dbyte


class STLS1(object):

    def __init__ (self):
        self.num_clients  =30000; # max is 16bit
        self.fsize        =64

    def create_stream (self):
        size = self.fsize - 4; # HW will add 4 bytes ethernet FCS
        saddr = ipa.ip_address(bytes(emb_v6_in_v4("fc00:1::", "10.1.0.1", 64)))
        daddr = ipa.ip_address(bytes(emb_v6_in_v4("fc00:2::", "10.2.0.1", 48)))

        addresses=[str(daddr)]
        base_pkt = \
            Ether()/\
            IPv6(src=str(saddr), dst=addresses[-1])/\
            IPv6ExtHdrSegmentRouting(len=(16 * len(addresses)) // 8, segleft=len(addresses)-1, lastentry=len(addresses)-1, addresses=addresses)/ \
            IP(src="192.168.1.1", dst="192.168.1.2")/\
            UDP(dport=80)

        pad = max(0, size - len(base_pkt)) * 'x'

        vm = STLScVmRaw([
                STLVmFlowVar(
                    name="ipv6_src",
                    min_value="16.0.0.0",
                    max_value="18.0.0.254",
                    size=4,
                    op="random",
                ),
                STLVmWrFlowVar(
                    fv_name="ipv6_src",
                    pkt_offset= "IPv6.src",
                    offset_fixup=12,
                ),
            ],
        )

        return STLStream(packet = STLPktBuilder(pkt = base_pkt/pad,vm = vm),
                         mode = STLTXCont( pps=10 ))

    def get_streams (self, direction = 0, **kwargs):
        # create 1 stream
        return [ self.create_stream() ]


# dynamic load - used for trex console or simulator
def register():
    return STLS1()
