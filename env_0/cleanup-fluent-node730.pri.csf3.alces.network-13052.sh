/opt/apps/apps/binapps/fluent/19.2/v192/fluent/bin/fluent-cleanup.pl node730.pri.csf3.alces.network 41206 CLEANUP_EXITING

LOCALHOST=`hostname -s`
if [[ node730.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 13763; else ssh node730.pri.csf3.alces.network kill -9 13763; fi
if [[ node730.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 13762; else ssh node730.pri.csf3.alces.network kill -9 13762; fi
if [[ node730.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 13052; else ssh node730.pri.csf3.alces.network kill -9 13052; fi
if [[ node730.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 12725; else ssh node730.pri.csf3.alces.network kill -9 12725; fi

rm -f /net/scratch2/m83358ym/Workingfolder/env_0/cleanup-fluent-node730.pri.csf3.alces.network-13052.sh
