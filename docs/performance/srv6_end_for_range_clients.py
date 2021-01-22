from trex_stl_lib.api import *

class STLS1(object):

    def __init__ (self):
        self.num_clients  =30000; # max is 16bit
        self.fsize        =64

    def create_stream (self):
        # Create base packet and pad it to size
        size = self.fsize - 4; # HW will add 4 bytes ethernet FCS
        addresses=["fc00:3::3", "fc00:2::2"]
        base_pkt = \
            Ether()/\
            IPv6(src="fc00:1::1", dst="fc00:2::2")/\
            IPv6ExtHdrSegmentRouting(len=(16 * len(addresses)) // 8, segleft=len(addresses)-1, lastentry=len(addresses)-1, addresses=addresses)/ \
            IP(src="192.168.1.1", dst="192.168.1.2")/\
            TCP(dport=80,flags="S")
        pad = max(0, size - len(base_pkt)) * 'x'

        vm = STLScVmRaw([
                STLVmFlowVar(
                    name="ipv6_flow",
                    min_value=0,
                    max_value=255,
                    size=2,
                    op="random",
                ),
                STLVmFlowVar(
                    name="ipv6_src",
                    min_value="16.0.0.0",
                    max_value="18.0.0.254",
                    size=4,
                    op="random",
                ),
                STLVmFlowVar(name="src_port",
                        min_value=1025,
                        max_value=65000,
                        size=2, op="random"),
                STLVmWrFlowVar(
                    fv_name="ipv6_flow",
                    pkt_offset= "IPv6.fl",
                ),
                STLVmWrFlowVar(
                    fv_name="ipv6_src",
                    pkt_offset= "IPv6.src",
                    offset_fixup=12,
                ),

                STLVmWrFlowVar(fv_name="src_port",
                                pkt_offset= "TCP.sport"), # fix udp len
                STLVmFixChecksumHw(l3_offset = "IPv6",
                    l4_offset = "TCP",
                    l4_type  = CTRexVmInsFixHwCs.L4_TYPE_TCP )# hint, TRex can know that
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
