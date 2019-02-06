#!/usr/bin/env python
#Author: Patrick Knauth (patrick.w.knauth@nasa.gov)
#Run: python lagr2hdf5.py "your_lgr_file".lgr
'''
Usage:
    lagr2hdf5.py LAGER
'''

import docopt
import struct
import xml.etree.ElementTree as et
import h5py
import numpy as np
from collections import defaultdict
import binascii

args = docopt.docopt(__doc__)

#from pysmt/defs.py
type_map = {
    'int8_t': 'b',
    'uint8_t': 'B',
    'int16_t': 'h',
    'uint16_t': 'H',
    'int32_t': 'i',
    'uint32_t': 'I',
    'int64_t': 'q',
    'uint64_t': 'Q',
    'float32': 'f',
    'float64': 'd',
}

fmt_suffix_map = {
    'int8_t': 'd',
    'uint8_t': 'd',
    'int16_t': 'd',
    'uint16_t': 'd',
    'int32_t': 'd',
    'uint32_t': 'd',
    'int64_t': 'd',
    'uint64_t': 'd',
    'float32': 'g',
    'float64': 'g',
}

#uuid is in Item so that it knows where it comes from when adding to the dict
#key is so that it knows which key to add when seperating it in a hdf5 file
class Item(object):
    def __init__(self, name, offset, size, dtype, uuid, key):
        self.name = name
        self.offset = int(offset)
        self.size = int(size)
        self.dtype = dtype
        self.uuid = uuid
        self.key = key

    def __repr__(self):
        return '<Item:{} offset:{} size:{} type:{} uuid:{} key:{}>'.format(name, offset, size, dtype, uuid, key)

class Format(object):
    def __init__(self, uuid, version, key):
        self.uuid = uuid
        self.version = version
        self.key = key

    def __repr__(self):
        return '<uuid:{} version:{} key:{}>'.format(uuid, version, key)


with open(args['LAGER'], 'rb') as f:

    # uint16
    b = bytes(f.read(2))
    version = struct.unpack('!H', b)[0]
    # uint64
    b = bytes(f.read(8))
    dataoffset = struct.unpack('!Q', b)[0]
    print('Version: {version}, Offset: {offset}'.format(version=version, offset=dataoffset))

    #** keep in case of bad/new data added
    # while True:
    #     uuid = f.read(16).hex()
    #     groupname = struct.unpack('!Q', bytes(f.read(8)))[0]
    #     print(uuid)
    #     print(groupname)

    #     print(str(struct.unpack('!I', bytes(f.read(4)))[0])) #timestamp
    #     print(str(struct.unpack('!I', bytes(f.read(4)))[0])) #timestamp

    f.seek(dataoffset)
    xmlformat = f.read()

    hf = h5py.File(args['LAGER'].split(".")[0] + "_converted.hdf5", 'w')
    hdf5 = {}

    root = et.fromstring(xmlformat)
    column_size = 0
    items = []
    keys = []

    #add key to a list of keys
    #add item to list of items with the correct uuid
    #add those name | key combo as a string to a dictionary of lists
    for m in root.iter('format'):
        uuid = m.get('uuid')
        version = m.get('version')
        key = m.get('key')
        form = Format(uuid, version, key)
        keys.append(form)
        for n in m:
            name = n.get('name')
            offset = n.get('offset')
            size = n.get('size')
            dtype = n.get('type')
            i = Item(name, offset, size, dtype, uuid, key)
            items.append(i)
            column_size = column_size + i.size

            hdf5.setdefault(name + "|" + key, [])

    #header
    minwidth = 18
    for i in items:
        if len(i.name) < minwidth:
            l = minwidth
        else:
            len(i.name)

    #go back to start of data
    f.seek(10, 0)

    #iterate through all formats/items
    while True:
        if f.tell() + column_size > dataoffset:
            break
        uuid = binascii.hexlify(f.read(16))
        timestamp = struct.unpack('!Q', bytes(f.read(8)))[0]

        for i in items:
            if i.uuid.replace("-","") == uuid:
                b = bytes(f.read(i.size))
                val = struct.unpack('!'+type_map[i.dtype], b)[0]
                if len(i.name) < minwidth:
                    l = minwidth
                else:
                    len(i.name)

                d1 = '{val:<{LEN}{suffix}}'.format(val=val, LEN=l, suffix=fmt_suffix_map[i.dtype])
                hdf5[i.name + "|" + i.key].append(d1)

    #create hdf5 groups
    for g in keys:
        hf.create_group(g.key)

    #create hdf5 datasets and add them to the group
    for x, y in hdf5.items():
        z = list(y)
        for i in items:
            if i.name+"|"+i.key == x:
                array = np.asarray(z, dtype=np.float32)
                hf.create_dataset("/" + i.key + "/" + x.split("|")[0],data=array)

    #close .hdf5 input
    hf.close()