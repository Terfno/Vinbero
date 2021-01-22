from trex_stl_lib.api import *
from scapy.contrib.gtp import GTPHeader as GTPHeader

class STLS1(object):
    def create_stream (self, packet_len, stream_count):
        base_pkt = \
            Ether()/\
            IP(src="10.1.0.1", dst="10.2.0.1")/\
            UDP(dport=2152,sport=2152)/\
            GTPHeader(version=1, teid=1111,gtp_type=0xff)/\
            IP(src="10.0.1.1", dst="10.0.1.2")/\
            UDP(sport=10053,dport=10053)


        base_pkt_len = len(base_pkt)
        base_pkt /= 'x' * max(0, packet_len - base_pkt_len)
        packets = []
        for i in range(stream_count):
            packets.append(STLStream(
                packet = STLPktBuilder(pkt = base_pkt),
                mode = STLTXCont()
                ))
        return packets

    def get_streams (self, direction = 0, packet_len = 64, stream_count = 1, **kwargs):
        # create 1 stream
        return self.create_stream(packet_len - 4, stream_count)


# dynamic load - used for trex console or simulator
def register():
    return STLS1()

