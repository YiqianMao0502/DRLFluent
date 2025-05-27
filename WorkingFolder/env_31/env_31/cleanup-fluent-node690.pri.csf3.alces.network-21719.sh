/opt/apps/apps/binapps/fluent/19.5/ansys_inc/v195/fluent/bin/fluent-cleanup.pl node690.pri.csf3.alces.network 46616 CLEANUP_EXITING

LOCALHOST=`hostname -s`
if [[ node690.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 22171; else ssh node690.pri.csf3.alces.network kill -9 22171; fi
if [[ node690.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 21719; else ssh node690.pri.csf3.alces.network kill -9 21719; fi
if [[ node690.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 21285; else ssh node690.pri.csf3.alces.network kill -9 21285; fi

rm -f /net/scratch2/m83358ym/Workingfolder/env_31/cleanup-fluent-node690.pri.csf3.alces.network-21719.sh
