#!/usr/bin/env python

# Script to extract the distribution date from update.log

import sys

if len(sys.argv) > 1:
    package = sys.argv[1]
else:
    package = "OMNIORB"

line = sys.stdin.readline()

while line == "\n":
    line = sys.stdin.readline()

line = line.strip()

output = """\
// distdate.hh -- Automatically generated file

#define %s_DIST_DATE "%s"
"""

sys.stdout.write(output % (package, line))
