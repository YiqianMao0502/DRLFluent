/opt/apps/apps/binapps/fluent/19.2/v192/fluent/bin/fluent-cleanup.pl node717.pri.csf3.alces.network 36745 CLEANUP_EXITING

LOCALHOST=`hostname -s`
if [[ node717.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 15255; else ssh node717.pri.csf3.alces.network kill -9 15255; fi
if [[ node717.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 15254; else ssh node717.pri.csf3.alces.network kill -9 15254; fi
if [[ node717.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 13826; else ssh node717.pri.csf3.alces.network kill -9 13826; fi
if [[ node717.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 13169; else ssh node717.pri.csf3.alces.network kill -9 13169; fi

rm -f /net/scratch2/m83358ym/Workingfolder/env_3/cleanup-fluent-node717.pri.csf3.alces.network-13826.sh
