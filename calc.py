#!/usr/bin/env python3

import re
from scapy.all import (
    Ether,
    IntField,
    Packet,
    StrFixedLenField,
    XByteField,
    bind_layers,
    srp1
)

class P4calc(Packet):
    name = "P4calc"
    fields_desc = [ StrFixedLenField("P", "P", length=1),
                    StrFixedLenField("Four", "4", length=1),
                    XByteField("version", 0x01),
                    StrFixedLenField("op", "+", length=1),
                    IntField("operand_a", 0),
                    IntField("operand_b", 0),
                    IntField("result", 0xDEADBABE)]

bind_layers(Ether, P4calc, type=0x1234)

class NumParseError(Exception):
    pass

class OpParseError(Exception):
    pass

class Token:
    def __init__(self, s, kind):
        self.s = s
        self.kind = kind

    def __repr__(self):
        return f'{self.kind}: {self.s}'

    def __str__(self):
        return self.s

def lex(s):
    for m in re.finditer(r'(0b[01]+|[+&|^])', s):
        s = m.group(0)
        if re.match(r'0b[01]+', s):
            yield Token(s, 'NUM')
        elif re.match(r'[+&|^]', s):
            yield Token(s, 'OP')
        else:
            raise ValueError('Unexpected match')

def parse(tokens):
    a = next(tokens)
    if a.kind != 'NUM':
        raise NumParseError(f'Expected NUM, got {a}')
    a = int(a.s, 2)

    op = next(tokens)
    if op.kind != 'OP':
        raise OpParseError(f'Expected OP, got {op}')
    op = str(op)

    b = next(tokens)
    if b.kind != 'NUM':
        raise NumParseError(f'Expected NUM, got {b}')
    b = int(b.s, 2)

    return a, op, b

def main():
    print('Binary Adder')
    while True:
        try:
            s = input('> ')
            if s == 'exit':
                break

            tokens = lex(s)
            a, op, b = parse(tokens)

            op_map = {
                '+': '+',
                '&': '&',
                '|': '|',
                '^': '^',
            }

            ether = Ether(src='00:00:00:00:00:01', dst='ff:ff:ff:ff:ff:ff')
            p4calc = P4calc(op=op_map[op], operand_a=a, operand_b=b)

            pkt = ether / p4calc

            print(f'Sending: {bin(a)} {op} {bin(b)}')
            resp = srp1(pkt, iface='h1-eth0', timeout=1, verbose=False)
            if resp:
                result = resp[P4calc].result
                print(f'Result: {bin(result)}')
            else:
                print("Didn't receive a response")
        except (NumParseError, OpParseError) as e:
            print(f'Error: {e}')
        except Exception as e:
            print(f'Unexpected error: {e}')

if __name__ == '__main__':
    main()
