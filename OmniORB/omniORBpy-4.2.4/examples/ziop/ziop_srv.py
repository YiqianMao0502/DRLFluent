#!/usr/bin/env python

import sys
from omniORB import CORBA, PortableServer
from omniORB import ZIOP, Compression

# Import the stubs and skeletons for the Example module
import Example, Example__POA

# Define an implementation of the Echo interface
class Echo_i (Example__POA.Echo):
    def echoString(self, mesg):
        print "echoString() called with", len(mesg), "characters"
        return mesg * 2

# Initialise the ORB
orb = CORBA.ORB_init(sys.argv, CORBA.ORB_ID)

# Find the root POA
root_poa = orb.resolve_initial_references("RootPOA")

# Create ZIOP enabling policies
zlib_id  = Compression.CompressorIdLevel(Compression.COMPRESSORID_ZLIB, 6)
comp_ids = [zlib_id]

ps = [ orb.create_policy(ZIOP.COMPRESSION_ENABLING_POLICY_ID, True),
       orb.create_policy(ZIOP.COMPRESSOR_ID_LEVEL_LIST_POLICY_ID, comp_ids) ]

# Create POA with the ZIOP policies
poa = root_poa.create_POA("test", None, ps)

# Create an instance of Echo_i
ei = Echo_i()

# Activate the object and create an object reference
poa.activate_object(ei)
eo = ei._this()

# Print out the IOR
print orb.object_to_string(eo)

# Activate the POA
poa.the_POAManager.activate()

# Run
orb.run()
