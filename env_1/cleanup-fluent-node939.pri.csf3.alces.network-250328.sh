/opt/apps/apps/binapps/fluent/19.2/v192/fluent/bin/fluent-cleanup.pl node939.pri.csf3.alces.network 46519 CLEANUP_EXITING

LOCALHOST=`hostname -s`
if [[ node939.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 250968; else ssh node939.pri.csf3.alces.network kill -9 250968; fi
if [[ node939.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 250967; else ssh node939.pri.csf3.alces.network kill -9 250967; fi
if [[ node939.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 250328; else ssh node939.pri.csf3.alces.network kill -9 250328; fi
if [[ node939.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 250001; else ssh node939.pri.csf3.alces.network kill -9 250001; fi

rm -f /net/scratch2/m83358ym/Workingfolder/env_1/cleanup-fluent-node939.pri.csf3.alces.network-250328.sh
