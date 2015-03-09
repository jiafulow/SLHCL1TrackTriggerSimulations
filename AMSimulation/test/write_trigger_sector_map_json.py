#!/usr/bin/env python

import json

mymap = {}
with open("../data/trigger_sector_map.csv", "r") as f:
    for line in f:
        if not line[0].isdigit():
            continue
        values = line.split(",")

        # Convert to int
        values = [int(x) for x in values]

        key = (values[0]-1)*8 + (values[1]-1)
        values = sorted(values[2:])
        mymap[key] = values

assert(len(mymap) == 6*8)

#for k in sorted(mymap.keys()):
#    print "Tower", k, "has", len(mymap[k]), "modules."
#
#from collections import defaultdict
#reversemap = defaultdict(list)
#for k, v in mymap.iteritems():
#    for e in v:
#        reversemap[e].append(k)
#
#for k in sorted(reversemap.keys()):
#    print "Module", k, "is in", len(reversemap[k]), "towers."

json.dump(mymap, open("../data/trigger_sector_map.json", "w"), sort_keys=True)

