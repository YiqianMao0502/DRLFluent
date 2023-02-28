#!/usr/bin/env python

import sys, time

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


class SimpleHandler(object):
    def __init__(self):
        print "SimpleHandler init"
        self.done = False

    def __del__(self):
        print "SimpleHandler del"

    def echoString(self, result):
        print "echoString result:", result
        self.done = True

    def echoString_excep(self, holder):
        try:
            holder.raise_exception()

        except CORBA.Exception, ex:
            print "echoString exception:", ex

        else:
            print "raise_exception didn't raise an exception!"

        self.done = True


handler = SimpleHandler()

# Invoke the echoString operation using callback AMI
print "Send..."
eo.sendc_echoString(handler, "Hello with a callback")

while not handler.done:
    print "Not done yet..."
    time.sleep(1)

orb.destroy()
