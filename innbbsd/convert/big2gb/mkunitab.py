#/usr/bin/env python

import sys, string, os, getopt

def make(mapfile, mbname, lcrange, unochar, mbnochar):
    f = open(mapfile)
    mb = open(mbname + ".mb.c", "w")
    mapl2u = {}
    mapu2l = {}
    s = f.readline()
    while s:
        s = string.strip(s)
        i = string.find(s, '#')
        if i != -1:
            s = s[:i]
            m = string.split(s)
            if len(m) > 1 and eval(m[0]) > 0xff :
                mapl2u[eval(m[0])] = eval(m[1])
                mapu2l[eval(m[1])] = eval(m[0])
        s = f.readline()
    mb.write("#define unochar " + hex(unochar) + "\n")
    mb.write("#define mbnochar " + hex(mbnochar) + "\n")
    mb.write("static unsigned int ltab[][" + str(lcrange[1][1] - lcrange[1][0]) + "]=\n{\n")
    for lead in range(lcrange[0][0], lcrange[0][1]):
        mb.write("\t{\n\t\t");
        for trail in range(lcrange[1][0], lcrange[1][1]):
            try:
                val = hex(mapl2u[(lead << 8) | trail])
            except:
                val = 'unochar'
            mb.write(val + ",")
            if (trail & 7) == 7:
                mb.write("\n\t\t")
        mb.write("},\n")
    mb.write("};\n")
    mb.write("/* --- FROM local encoding to unicode --- */\n")
    mb.write("unsigned int from" + mbname + "(unsigned int dbchar)\n{\n")
    mb.write("\tconst unsigned char lead_low = " + hex(lcrange[0][0]) + ";\n")
    mb.write("\tconst unsigned char lead_high = " + hex(lcrange[0][1] - 1) + ";\n")
    mb.write("\tconst unsigned char trail_low = " + hex(lcrange[1][0]) + ";\n")
    mb.write("\tconst unsigned char trail_high = " + hex(lcrange[1][1] - 1) + ";\n")
    mb.write("""
    unsigned char lead = (dbchar >> 8) & 0xff;
    unsigned char trail = dbchar & 0xff;
    if (lead < lead_low || lead > lead_high)
        return unochar;
    if (trail < trail_low || trail > trail_high)
        return unochar;
    return ltab[lead - lead_low][trail - trail_low];
}\n\n\n""")
    mapupane = {}
    for hibyte in range(0, 0x100):
        emptypane = 1
        for lowbyte in range(0, 0x100):
            try:
                val = hex(mapu2l[(hibyte << 8) | lowbyte])
                emptypane = 0
            except:
                pass
        if emptypane:
            pass
        else:
            mapupane[hibyte] = 1
            mb.write("static unsigned int upane" + hex(hibyte)[2:] + "[] =\n{\n\t")
            for lowbyte in range(0, 0xff):
                try:
                    val = hex(mapu2l[(hibyte << 8) | lowbyte])
                except:
                    val = 'mbnochar';
                mb.write(val + ',')
                if (lowbyte & 7) == 7:
                    mb.write("\n\t")
            mb.write("};\n\n")
    mb.write("\n\nstatic unsigned int *utab[] =\n{\n")
    for hibyte in range(0, 0x100):
        if mapupane.has_key(hibyte):
            mb.write("\tupane" + hex(hibyte)[2:] + ",\n")
        else:
            mb.write("\t0,\n")
    mb.write("};\n\n")
    mb.write("/* --- FROM unicode to local encoding --- */\n")
    mb.write("unsigned int to" + mbname + "(unsigned int uchar)\n{\n")
    mb.write("""
    unsigned char hibyte = (uchar >> 8) & 0xff;
    unsigned char lowbyte = uchar & 0xff;
    if (!utab[hibyte])
        return mbnochar;
    return utab[hibyte][lowbyte];
}\n""")
    f.close()
    mb.close()

