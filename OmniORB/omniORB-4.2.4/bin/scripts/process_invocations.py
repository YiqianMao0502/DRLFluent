#!/usr/bin/env python

"""
process_invocations.py

This processes omniORB invocation logs obtained when the following
omniORB trace options are enabled:

  traceInvocations 1
  traceInvocationReturns 1
  traceThreadId 1
  traceTime 1
"""
import sys, re, itertools
from time import mktime, strptime
from collections import defaultdict



def output(call_counts, call_times, call_max):
    print

    print ("%-30s %8s  :  %9s  :  %9s  :  %9s" %
           ("Operation", "Count", "Total", "Mean", "Max"))

    print ("%-30s %8s  :  %9s  :  %9s  :  %9s" %
           ("---------", "-----", "-----", "----", "---"))

    for operation, count in sorted(call_counts.items()):
        total = call_times[operation]

        print ("%-30s %8d  :  %9.3f  :  %9.3f  :  %9.3f" %
               (operation, count, total, total / count, call_max[operation]))

    call_counts.clear()
    call_times.clear()
    call_max.clear()



def process(input):
    exp = re.compile(r"omniORB: \(([^)]+)\) (....-..-.. ..:..:..)(\.......): (Dispatching|Return from) (remote|in process) call '([^']+)' to: ([^ ]+) \(([^)]+)\)\n")
    in_progress = {}
    call_counts = defaultdict(int)
    call_times  = defaultdict(float)
    call_max    = defaultdict(float)

    # *** This should be an option...
    midpoint = "2012-05-24 18:05:00"
    midpoint = mktime(strptime(midpoint, "%Y-%m-%d %H:%M:%S"))

    out_time = False

    try:
        for lcount, line in enumerate(input):

            if lcount % 100000 == 0:
                print lcount,
                out_time = True

            match = exp.match(line)
            if match:
                thread    = match.group(1)
                time      = (mktime(strptime(match.group(2),
                                             "%Y-%m-%d %H:%M:%S")) +
                             float(match.group(3)))
                dispatch  = match.group(4) == "Dispatching"
                remote    = match.group(5) == "remote"
                operation = match.group(6)
                #key       = match.group(7)

                if not remote:
                    continue

                if out_time:
                    print match.group(2),
                    out_time = False
                    sys.stdout.flush()

                if midpoint and time > midpoint:
                    output(call_counts, call_times, call_max)
                    print "\n\n********\n\n"
                    midpoint = None

                if dispatch:
                    in_progress[thread] = time, operation
                else:
                    try:
                        ip_time, ip_op = in_progress.pop(thread)

                        if ip_op == operation:
                            duration = time - ip_time

                            call_counts[operation] += 1
                            call_times[operation]  += duration
                            call_max[operation]     = max(call_max[operation],
                                                          duration)

                            #print operation, "\t", duration

                    except KeyError:
                        # Missing dispatch log
                        pass

    except KeyboardInterrupt:
        pass

    output(call_counts, call_times, call_max)



def main():
    args = sys.argv[1:]
    if args:
        process(itertools.chain.from_iterable( open(f) for f in args ))
    else:
        process(sys.stdin)



if __name__ == "__main__":
    main()
