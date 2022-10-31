/opt/apps/apps/binapps/fluent/19.2/v192/fluent/bin/fluent-cleanup.pl node717.pri.csf3.alces.network 38797 CLEANUP_EXITING

LOCALHOST=`hostname -s`
if [[ node717.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 15253; else ssh node717.pri.csf3.alces.network kill -9 15253; fi
if [[ node717.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 15252; else ssh node717.pri.csf3.alces.network kill -9 15252; fi
if [[ node717.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 13825; else ssh node717.pri.csf3.alces.network kill -9 13825; fi
if [[ node717.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 13168; else ssh node717.pri.csf3.alces.network kill -9 13168; fi

rm -f /net/scratch2/m83358ym/Workingfolder/env_2/cleanup-fluent-node717.pri.csf3.alces.network-13825.sh
