from trex_stl_lib.api import *
from scapy.contrib.gtp import GTPHeader as GTPHeader

class STLS1(object):

    def __init__ (self):
        self.num_clients  =30000; # max is 16bit
        self.fsize        =64

    def create_stream (self):
        # Create base packet and pad it to size
        size = self.fsize - 4; # HW will add 4 bytes ethernet FCS
        base_pkt = \
            Ether()/\
            IP(src="10.1.0.1", dst="10.2.0.1")/\
            UDP(dport=2152,sport=2152)/\
            GTPHeader(version=1, teid=1111,gtp_type=0xff)/\
            IP(src="10.0.1.1", dst="10.0.1.2")/\
            UDP(sport=10053,dport=10053)
        pad = max(0, size - len(base_pkt)) * 'x'

        vm = STLScVmRaw([
                    STLVmFlowVar(
                        name="ip_src",
                        min_value="10.1.0.1",
                        max_value="10.1.0.255",
                        size=4,
                        step=1,
                        op="inc"
                    ),
                    STLVmWrFlowVar(
                        fv_name="ip_src",
                        pkt_offset= "IP.src",
                    ), # write ip to packet IP.src
                    STLVmFixIpv4(offset = "IP") # fix checksum
            ],
            cache_size = 255 # cache the packets, much better performance
        )

        return STLStream(packet = STLPktBuilder(pkt = base_pkt/pad,vm = vm),
                         mode = STLTXCont( pps=10 ))

    def get_streams (self, direction = 0, **kwargs):
        # create 1 stream
        return [ self.create_stream() ]


# dynamic load - used for trex console or simulator
def register():
    return STLS1()
