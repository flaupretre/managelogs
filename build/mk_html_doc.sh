
TMP_FILE=/tmp/.d$$

man2html >$TMP_FILE

end=`fgrep -n '<A NAME="index">' $TMP_FILE | awk -F: '{ print $1 }'`
end=`expr $end - 4`

start=`fgrep -n '<H2>NAME</H2>' $TMP_FILE | awk -F: '{ print $1 }'`

head -$end <$TMP_FILE | tail -n +$start

/bin/rm -f $TMP_FILE
