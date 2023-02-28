#!/usr/bin/env python

import sys

# Import the CORBA module
from omniORB import CORBA

# Import the stubs for the Example module
import Example

# Initialise the ORB
orb = CORBA.ORB_init(sys.argv, CORBA.ORB_ID)

# Get the IOR of an Echo object from the command line (without
# checking that the arguments are sensible!)
ior = sys.argv[1]

# Convert the IOR to an object reference
obj = orb.string_to_object(ior)

# Narrow reference to an Example::Echo object
eo = obj._narrow(Example.Echo)

if eo is None:
    print "Object reference is not an Example::Echo"
    sys.exit(1)

# Invoke the echoString operation using polling AMI
print "Send..."
poller = eo.sendp_echoString("Hello with a poller")

# Poll with a timeout of 100 ms
while not poller.is_ready(100):
    print "Not ready..."

result = poller.echoString(0)
print "The result was:", result

# Invoke again
print "Send 2..."
poller = eo.sendp_echoString("Hello again")

# Poll with a timeout of 1 second
try:
    result = poller.echoString(1000)

except CORBA.TIMEOUT:
    print "Timed out"

else:
    print "The second result was:", result

orb.destroy()
