/opt/apps/apps/binapps/fluent/19.2/v192/fluent/bin/fluent-cleanup.pl node517.pri.csf3.alces.network 36111 CLEANUP_EXITING

LOCALHOST=`hostname -s`
if [[ node517.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 8647; else ssh node517.pri.csf3.alces.network kill -9 8647; fi
if [[ node517.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 8646; else ssh node517.pri.csf3.alces.network kill -9 8646; fi
if [[ node517.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 7936; else ssh node517.pri.csf3.alces.network kill -9 7936; fi
if [[ node517.pri.csf3.alces.network == "$LOCALHOST"* ]]; then kill -9 7606; else ssh node517.pri.csf3.alces.network kill -9 7606; fi

rm -f /net/scratch2/m83358ym/Workingfolder/cleanup-fluent-node517.pri.csf3.alces.network-7936.sh
