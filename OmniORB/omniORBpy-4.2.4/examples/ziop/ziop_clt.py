#!/usr/bin/env python

import sys
from omniORB import CORBA, omniZIOP
import Example

orb = CORBA.ORB_init(sys.argv)

# Set empty global ZIOP policies. This enables ZIOP with the default
# settings.
omniZIOP.setGlobalPolicies([])

if len(sys.argv) != 2:
    print 'usage: ziop_clt.py -ORBclientTransportRule "* unix,ssl,tcp,ziop" <ior>'
    sys.exit(1)

obj = orb.string_to_object(sys.argv[1])
obj = obj._narrow(Example.Echo)

# Send a string containing 5 copies of this source
message = open(__file__).read() * 5
result  = obj.echoString(message)

print "I sent", len(message), "characters, and received", len(result)

# Destroy the ORB to clean up
orb.destroy()
