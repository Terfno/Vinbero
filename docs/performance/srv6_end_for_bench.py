from trex_stl_lib.api import *


class STLS1(object):
    def create_stream (self, packet_len, stream_count):
        addresses=["fc00:3::3", "fc00:2::2"]
        base_pkt = \
            Ether()/\
            IPv6(src="fc00:1::1", dst="fc00:2::2")/\
            IPv6ExtHdrSegmentRouting(len=(16 * len(addresses)) // 8, segleft=len(addresses)-1, lastentry=len(addresses)-1, addresses=addresses)/ \
            IP(src="192.168.1.1", dst="192.168.1.2")/
            TCP(dport=80,flags=10000)

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
