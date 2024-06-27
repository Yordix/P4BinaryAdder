/* -*- P4_16 -*- */

/*
 * P4 4-bit Binary Adder
 *
 * This program implements a simple 4-bit binary adder protocol. It can be carried over Ethernet
 * (Ethertype 0x1234).
 *
 * The Protocol header looks like this:
 *
 *        0                1                  2              3
 * +----------------+----------------+----------------+---------------+
 * |      P         |       4        |     Version    |     Op        |
 * +----------------+----------------+----------------+---------------+
 * | 0000           | 0000           |      0000      |    0000       |
 * +----------------+----------------+----------------+---------------+
 * |                              Operand A (4 bits)                  |
 * +----------------+----------------+----------------+---------------+
 * |                              Operand B (4 bits)                  |
 * +----------------+----------------+----------------+---------------+
 * |                              Result (4 bits)                     |
 * +----------------+----------------+----------------+---------------+
 *
 * P is an ASCII Letter 'P' (0x50)
 * 4 is an ASCII Letter '4' (0x34)
 * Version is currently 0.1 (0x01)
 * Op is always addition for this example (0x2b)
 *
 * The device receives a packet, performs the addition, fills in the
 * result and sends the packet back out of the same port it came in on, while
 * swapping the source and destination addresses.
 *
 * If the header is not valid, the packet is dropped.
 */

#include <core.p4>
#include <v1model.p4>

/*
 * Define the headers the program will recognize
 */

/*
 * Standard Ethernet header
 */
header ethernet_t {
    bit<48> dstAddr;
    bit<48> srcAddr;
    bit<16> etherType;
}

/*
 * This is a custom protocol header for the 4-bit binary adder.
 * We'll use etherType 0x1234 for it (see parser)
 */
const bit<16> P4CALC_ETYPE = 0x1234;
const bit<8>  P4CALC_P     = 0x50;   // 'P'
const bit<8>  P4CALC_4     = 0x34;   // '4'
const bit<8>  P4CALC_VER   = 0x01;   // v0.1
const bit<8>  P4CALC_OP    = 0x2b;   // '+'

header p4calc_t {
    bit<8>  p;
    bit<8>  four;
    bit<8>  ver;
    bit<8>  op;
    bit<4>  operand_a;
    bit<4>  operand_b;
    bit<4>  res;
}

/*
 * All headers, used in the program needs to be assembled into a single struct.
 * We only need to declare the type, but there is no need to instantiate it,
 * because it is done "by the architecture", i.e. outside of P4 functions
 */
struct headers {
    ethernet_t   ethernet;
    p4calc_t     p4calc;
}

/*
 * All metadata, globally used in the program, also  needs to be assembled
 * into a single struct. As in the case of the headers, we only need to
 * declare the type, but there is no need to instantiate it,
 * because it is done "by the architecture", i.e. outside of P4 functions
 */
struct metadata {
    /* In our case it is empty */
}

/*************************************************************************
 ***********************  P A R S E R  ***********************************
 *************************************************************************/
parser MyParser(packet_in packet,
                out headers hdr,
                inout metadata meta,
                inout standard_metadata_t standard_metadata) {
    state start {
        packet.extract(hdr.ethernet);
        transition select(hdr.ethernet.etherType) {
            P4CALC_ETYPE : check_p4calc;
            default      : accept;
        }
    }

    state check_p4calc {
        transition select(packet.lookahead<p4calc_t>().p,
                          packet.lookahead<p4calc_t>().four,
                          packet.lookahead<p4calc_t>().ver,
                          packet.lookahead<p4calc_t>().op) {
            (P4CALC_P, P4CALC_4, P4CALC_VER, P4CALC_OP) : parse_p4calc;
            default                                     : accept;
        }
    }

    state parse_p4calc {
        packet.extract(hdr.p4calc);
        transition accept;
    }
}

/*************************************************************************
 ************   C H E C K S U M    V E R I F I C A T I O N   *************
 *************************************************************************/
control MyVerifyChecksum(inout headers hdr,
                         inout metadata meta) {
    apply { }
}

/*************************************************************************
 **************  I N G R E S S   P R O C E S S I N G   *******************
 *************************************************************************/
control MyIngress(inout headers hdr,
                  inout metadata meta,
                  inout standard_metadata_t standard_metadata) {
    action send_back(bit<4> result) {
        hdr.p4calc.res = result;

        // Swap MAC addresses
        bit<48> temp = hdr.ethernet.dstAddr;
        hdr.ethernet.dstAddr = hdr.ethernet.srcAddr;
        hdr.ethernet.srcAddr = temp;

        // Send the packet back to the port it came from
        standard_metadata.egress_spec = standard_metadata.ingress_port;
    }

    action operation_add() {
        send_back(hdr.p4calc.operand_a + hdr.p4calc.operand_b);
    }

    action operation_drop() {
        mark_to_drop(standard_metadata);
    }

    table calculate {
        key = {
            hdr.p4calc.op : exact;
        }
        actions = {
            operation_add;
            operation_drop;
        }
        const default_action = operation_drop();
        const entries = {
            P4CALC_OP : operation_add();
        }
    }

    apply {
        if (hdr.p4calc.isValid()) {
            calculate.apply();
        } else {
            operation_drop();
        }
    }
}

/*************************************************************************
 ****************  E G R E S S   P R O C E S S I N G   *******************
 *************************************************************************/
control MyEgress(inout headers hdr,
                 inout metadata meta,
                 inout standard_metadata_t standard_metadata) {
    apply { }
}

/*************************************************************************
 *************   C H E C K S U M    C O M P U T A T I O N   **************
 *************************************************************************/
control MyComputeChecksum(inout headers hdr, inout metadata meta) {
    apply { }
}

/*************************************************************************
 ***********************  D E P A R S E R  *******************************
 *************************************************************************/
control MyDeparser(packet_out packet, in headers hdr) {
    apply {
        packet.emit(hdr.ethernet);
        packet.emit(hdr.p4calc);
    }
}

/*************************************************************************
 ***********************  S W I T T C H **********************************
 *************************************************************************/

V1Switch(
    MyParser(),
    MyVerifyChecksum(),
    MyIngress(),
    MyEgress(),
    MyComputeChecksum(),
    MyDeparser()
) main;
