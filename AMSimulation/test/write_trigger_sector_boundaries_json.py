#!/usr/bin/env python

import json
from math import sqrt, atan2

# Functions
convert_key_to_int = lambda pairs: dict([(int(k),v) for (k,v) in pairs])

quadsum = lambda x,y: sqrt(x*x + y*y)

def convert_xyz_to_cyl(xyz):
    cyl = []
    for i in xrange(4):
        r = quadsum(xyz[3*i], xyz[3*i+1])
        phi = atan2(xyz[3*i+1], xyz[3*i])
        z = xyz[3*i+2]
        cyl.append(r)
        cyl.append(phi)
        cyl.append(z)
    return cyl


# Load existing maps
ttmap = json.load(open("../data/trigger_sector_map.json"), object_pairs_hook=convert_key_to_int)
vertexmap = json.load(open("../data/module_vertices.json"), object_pairs_hook=convert_key_to_int)

# Find boundaries
writeme = []

for tt in sorted(ttmap.keys()):
    # Store min, max values for each layer
    phimins, phimaxs = [], []
    zmins, zmaxs = [], []  # z for barrel, r for endcap
    for i in xrange(23):
        phimins.append(9999.); phimaxs.append(-9999.)
        zmins.append(9999.); zmaxs.append(-9999.)

    # Check vertex positions of each module
    for moduleId in ttmap[tt]:
        xyz = vertexmap[moduleId]
        assert(len(xyz) == 12)

        cyl = convert_xyz_to_cyl(xyz)

        lay = int(moduleId / 10000)
        if 5 <= lay <= 10:
            # Barrel rectangle modules
            assert(xyz[0+0] == xyz[9+0])
            assert(xyz[0+1] == xyz[9+1])
            assert(xyz[0+2] == xyz[3+2])
            assert(xyz[3+0] == xyz[6+0])
            assert(xyz[3+1] == xyz[6+1])
            assert(xyz[6+2] == xyz[9+2])

            phis = sorted([round(cyl[3*i+1],6) for i in xrange(4)])
            phi0, phi1 = phis[0], phis[-1]

            zs = sorted([round(cyl[3*i+2],6) for i in xrange(4)])
            z0, z1 = zs[0], zs[-1]

            if phimins[lay] > phi0:  phimins[lay] = phi0
            if phimaxs[lay] < phi1:  phimaxs[lay] = phi1
            if zmins[lay] > z0:  zmins[lay] = z0
            if zmaxs[lay] < z1:  zmaxs[lay] = z1

        elif 11 <= lay <= 15 or 18 <= lay <= 22:
            # Endcap rectangle modules
            eps = 1.1e-4
            assert(abs((xyz[0+0]-xyz[3+0]) - (xyz[9+0]-xyz[6+0])) < eps)
            assert(abs((xyz[0+0]-xyz[9+0]) - (xyz[3+0]-xyz[6+0])) < eps)
            assert(abs((xyz[0+1]-xyz[3+1]) - (xyz[9+1]-xyz[6+1])) < eps)
            assert(abs((xyz[0+1]-xyz[9+1]) - (xyz[3+1]-xyz[6+1])) < eps)
            assert(xyz[0+2] == xyz[3+2])
            assert(xyz[6+2] == xyz[9+2])
            assert(xyz[0+2] == xyz[9+2])

            phis = sorted([round(cyl[3*i+1],6) for i in xrange(4)])
            phi0, phi1 = phis[0], phis[-1]

            rs = sorted([round(cyl[3*i+0],6) for i in xrange(4)])
            r0, r1 = rs[0], rs[-1]

            if phimins[lay] > phi0:  phimins[lay] = phi0
            if phimaxs[lay] < phi1:  phimaxs[lay] = phi1
            if zmins[lay] > r0:  zmins[lay] = r0
            if zmaxs[lay] < r1:  zmaxs[lay] = r1

    # Save into a map
    for i in xrange(23):
        if phimins[i] < 9999. or phimaxs[i] > 9999.:
            s = "{0},{1},{2:.6f},{3:.6f},{4:.4f},{5:.4f}".format(tt,i,phimins[i],phimaxs[i],zmins[i],zmaxs[i])
            #print s

            writeme.append(s)


# Write .csv file
with open("../data/trigger_sector_boundaries.csv", "w") as f:
    writeme = ["tt/I,layer/I,phimin/D,phimax/D,zmin_cm/D,zmax_cm/D"] + writeme
    f.write("\n".join(writeme))


# Write .json file
mymap = {}
with open("../data/trigger_sector_boundaries.csv", "r") as f:
    for line in f:
        if not line[0].isdigit():
            continue
        values = line.split(",")
        assert(len(values) == 6)

        # Convert to int or float
        values = [float(x) if "." in x else int(x) for x in values]

        key = values[0]*100 + values[1]
        values = values[2:]
        mymap[key] = values

json.dump(mymap, open("../data/trigger_sector_boundaries.json", "w"), sort_keys=True)

