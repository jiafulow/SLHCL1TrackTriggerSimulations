#!/usr/bin/env python

import json

def parse_tower(t):
    return (int(t[1])-1) * 8 + (int(t[3])-1)

mymap = {}
with open("../data/module_connections.csv", "r") as f:
    for line in f:
        if not line[0].isdigit():
            continue
        values = line.split(",")
        assert(len(values) >= 5)

        # Convert to int
        values = [parse_tower(x) if x[0] == "t" else int(x) for x in values]

        key = values[4]
        values = sorted(values[5:])
        mymap[key] = values

assert(len(mymap) == 15428)

json.dump(mymap, open("../data/module_connections.json", "w"), sort_keys=True)

