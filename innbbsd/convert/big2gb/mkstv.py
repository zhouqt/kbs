#/usr/bin/env python

import sys, os, string

def mkstv(name, mapping):
    f = open(name + ".mb.c", "w")
    mappane = {}
    for hibyte in range(0, 0xff):
        for lowbyte in range(0, 0xff):
            if (mapping.has_key(hibyte << 8 | lowbyte)):
                mappane[hibyte] = 1
                break
        if mappane.has_key(hibyte):
            f.write("static unsigned int pane" + hex(hibyte)[2:] + "[] =\n{\n")
            for lowbyte in range(0, 0xff):
                try:
                    f.write(hex(mapping[hibyte << 8 | lowbyte]) + ",")
                except:
                    f.write("0,")
                if lowbyte & 7 == 7:
                    f.write("\n")
            f.write("};\n\n")
    f.write("static unsigned int * convtab[] =\n{\n")
    for hibyte in range(0, 0xff):
        if mappane.has_key(hibyte):
            f.write("pane" + hex(hibyte)[2:] + ",\n")
        else:
            f.write("0,\n")
    f.write("};\n\n\n")
    f.write("unsigned int " + name + "(unsigned int origchar)\n")
    f.write("""
{
    unsigned char hibyte = (origchar >> 8 ) & 0xff;
    unsigned char lowbyte = origchar & 0xff;
    if (!convtab[hibyte])
        return origchar;
    if (!convtab[hibyte][lowbyte])
        return origchar;
    return convtab[hibyte][lowbyte];
}\n""")

# main

han = sys.stdin

maps2t = {}
mapt2s = {}

s = han.readline()
while s:
    s = string.strip(s)
    if s[0] != '#':
        if string.find(s, "kSimplifiedVariant")!= -1:
            a = string.split(s)
            mapt2s[eval('0x' + a[0][2:6])] = eval('0x' + a[2][2:6])
        elif string.find(s, "kTraditionalVariant") != -1:
            a = string.split(s)
            maps2t[eval('0x' + a[0][2:6])] = eval('0x' + a[2][2:6])
    s = han.readline()
han.close()

mkstv("s2t", maps2t)
mkstv("t2s", mapt2s)
