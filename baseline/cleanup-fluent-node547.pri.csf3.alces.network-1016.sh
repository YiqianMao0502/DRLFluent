/opt/apps/apps/binapps/fluent/19.2/v192/fluent/bin/fluent-cleanup.pl node547.pri.csf3.alces.network 45910 CLEANUP_EXITING

LOCALHOST=`hostname -s`
if [[ node547.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 1835; else ssh node547.pri.csf3.alces.network kill -9 1835; fi
if [[ node547.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 1834; else ssh node547.pri.csf3.alces.network kill -9 1834; fi
if [[ node547.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 1016; else ssh node547.pri.csf3.alces.network kill -9 1016; fi
if [[ node547.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 670; else ssh node547.pri.csf3.alces.network kill -9 670; fi

rm -f /net/scratch2/m83358ym/Workingfolder/simulation_base/cleanup-fluent-node547.pri.csf3.alces.network-1016.sh
