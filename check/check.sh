
CHECK_DIR=$PWD
RUN_DIR=$CHECK_DIR/tmp

LOG=$CHECK_DIR/check.log
RESFILE=$CHECK_DIR/check.res
PIDFILE=$CHECK_DIR/pidfile
BASE_PATH=$RUN_DIR/mylog
M_PIDFILE=$BASE_PATH.pid
PIPE=$CHECK_DIR/pipe
M_STATUSFILE=$BASE_PATH.status
TMPFILE=$RUN_DIR/tmp1

M=$CHECK_DIR/../src/managelogs
G=$CHECK_DIR/genlog

export LOG RESFILE PIDFILE BASE_PATH M_PIDFILE PIPE M_STATUSFILE TMPFILE CHECK_DIR RUN_DIR M G
#-------------------------------

msg()
{
echo "$*"
echo "$*" >&3
}

#------------

msgn()
{
echon "$*"
echon "$*" >&3
}

#------------

info()
{
msg "  -- $* --"
}

#------------

echon()
{
if [ -n "$BASH" ] ; then
	echo -n "$*"
else
	echo "$*\c"
fi
}

#------------

new_test()
{
msg
msg "== Test : $*"
msg
cleanup
}

#------------

checking()
{
printf "%-60s" "> Checking $* "
printf "%-60s" "> Checking $* " >&3
}

#------------

test_ok()
{
msg "OK"
}

#------------

test_ko()
{
msg "****KO****"
echo 1 >$RESFILE
}

#------------

cleanup()
{
/bin/rm -rf $RUN_DIR/*
}

#------------

full_cleanup()
{
cd $CHECK_DIR
/bin/rm -rf $RUN_DIR $PIPE $PIDFILE $RESFILE
}

#------------

mk_fifo()
{
/bin/rm -rf $PIPE
mkfifo $PIPE
}

#------------

kill_m()
{
info Stopping process
kill `pid`
sleep 1
}

#------------

bg_run()
{
info Starting process
$M -i $PIPE $* $BASE_PATH &
echo $! >$PIDFILE
sleep 1
}

#------------

pid()
{
cat $PIDFILE
}

#------------

test_rc()
{
if [ "X$1" = 'X0' ] ; then test_ok
else test_ko
fi
}

#------------

inv_rc()
{
if [ $1 = 0 ] ; then echo 1
else echo 0
fi
}

#------------
# $1 = run / stop

check_process()
{
if [ "$1" = run ] ; then
	str='is running'
	inv=0
else
	str='is stopped'
	inv=1
fi

checking if process $str
ps -ef | awk '{ print $2 }' | grep "^`pid`\$" >/dev/null
rc=$?
[ $inv = 1 ] && rc=`inv_rc $rc`
test_rc $rc
}

#------------

check_pid_file()
{
checking PID file
pid=`pid`
pid_from_file=`cat $M_PIDFILE`

test "X$pid" = "X$pid_from_file"
test_rc $?
}

#------------

check_status_file()
{
checking status file
test -f $M_STATUSFILE
test_rc $?
}

#------------

nb_log_files()
{
echo `ls -1 $BASE_PATH._* | wc -l`
}

#------------

global_log_size()
{
cat $BASE_PATH._* | wc -c
}

#-------------------------------

exec 3>&1
exec >$LOG 2>&1
set -x

full_cleanup
/bin/rm -rf $RUN_DIR
mkdir -p $RUN_DIR
cd $RUN_DIR
mk_fifo

#-------------------------------

new_test Basic run

bg_run

check_process run
check_pid_file

checking if log file exists
test `nb_log_files` -gt 0
test_rc $?


$G 1000 >$TMPFILE
count=`cat $TMPFILE | wc -c`
cat $TMPFILE >$PIPE

kill_m

check_status_file
check_process stop

checking log size
test `global_log_size` = $count
test_rc $?

#-------------------------------

new_test Options

checking -h option
$M -h
test_rc $?

checking -V option
$M -V
test_rc $?

checking '-I option (return code)'
$M -I </dev/null $BASE_PATH 2>&1 | tee $TMPFILE
test_rc $?
checking '-I option (statistics)'
grep statistics $TMPFILE
test_rc $?



checking wrong short option
$M -W
test_rc `inv_rc $?`

checking wrong long option
$M --wrong
test_rc `inv_rc $?`

#-------------------------------
# End

rc=0
[ -f $RESFILE ] && rc=`cat $RESFILE`

cd $CHECK_DIR
full_cleanup

exit $rc
