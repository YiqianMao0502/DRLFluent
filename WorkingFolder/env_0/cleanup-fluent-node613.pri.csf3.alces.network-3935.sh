/opt/apps/apps/binapps/fluent/19.5/ansys_inc/v195/fluent/bin/fluent-cleanup.pl node613.pri.csf3.alces.network 41031 CLEANUP_EXITING

LOCALHOST=`hostname -s`
if [[ node613.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 4383; else ssh node613.pri.csf3.alces.network kill -9 4383; fi
if [[ node613.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 3935; else ssh node613.pri.csf3.alces.network kill -9 3935; fi
if [[ node613.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 3507; else ssh node613.pri.csf3.alces.network kill -9 3507; fi

rm -f /net/scratch2/m83358ym/Workingfolder/env_0/cleanup-fluent-node613.pri.csf3.alces.network-3935.sh
