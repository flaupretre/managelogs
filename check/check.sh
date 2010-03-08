#
# managelogs self tests
#
# Run via 'make check' in the top directory
#
#===========================================================================

CHECK_DIR=$PWD
LOG=$CHECK_DIR/check.log

exec 3>&1
exec >$LOG 2>&1
set -x

RUN_DIR=$CHECK_DIR/tmp

d=`pwd`
SRC_DIR=`dirname $0` # Needed to support build in a separate dir (distcheck)
cd $SRC_DIR
SRC_DIR=`pwd`	# Make it absolute as we will cd to tmp
cd $d

RESFILE=$CHECK_DIR/check.res
PIDFILE=$CHECK_DIR/pidfile
REL_BASE_PATH=mylog
BASE_PATH=$RUN_DIR/$REL_BASE_PATH
M_PIDFILE=$BASE_PATH.pid
PIPE=$CHECK_DIR/pipe
M_STATUSFILE=$BASE_PATH.status
TMPFILE=$RUN_DIR/tmp1
REL_CMD=.././rcmd.sh	# Test relative path in command
RCMD=$RUN_DIR/$REL_CMD
SZ_LIMIT=10000
KEEP_COUNT=5
GEN_SIZE=105000
M=$CHECK_DIR/../src/managelogs
G=$CHECK_DIR/genlog
RCMD_LOG=$RUN_DIR/rcmd.log
DBGFILE=$RUN_DIR/check.dbg
DBG_GLOBAL=$CHECK_DIR/check.dbg
DATAFILE=$RUN_DIR/data

check_cnt=0

export LOG RESFILE PIDFILE BASE_PATH REL_BASE_PATH M_PIDFILE PIPE M_STATUSFILE TMPFILE \
	REL_CMD RCMD SZ_LIMIT KEEP_COUNT GEN_SIZE CHECK_DIR RUN_DIR M G RCMD_LOG SRC_DIR \
	DBGFILE DBG_GLOBAL check_cnt

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
check_cnt=`expr $check_cnt + 1`

printf "%-60s" "$check_cnt> Checking $* "
printf "%-60s" "$check_cnt> Checking $* " >&3
}

#------------

test_ok()
{
msg "OK"
}

#------------

test_skip()
{
msg "OK (skipped)"
}

#------------

test_ko()
{
msg "*** KO ***"
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
kill `pid`
sleep 1
check_process stop

checking PID file deletion
test ! -f $M_PIDFILE
test_rc $?

/bin/rm -f $PIDFILE

cat $DBGFILE >>$DBG_GLOBAL
echo >>$DBG_GLOBAL
cat $M_STATUSFILE >>$DBG_GLOBAL
echo >>$DBG_GLOBAL
echo "===================== Stop process ===================" >>$DBG_GLOBAL
echo >>$DBG_GLOBAL
}

#------------

fg_run()
{
echo >>$DBG_GLOBAL
echo "====== RUN: $G $GEN_SIZE | $M $*" >>$DBG_GLOBAL
echo >>$DBG_GLOBAL

/bin/rm -f $DBGFILE
$G $GEN_SIZE >$DATAFILE
cat $DATAFILE | $M -v -d $DBGFILE $*

cat $DBGFILE >>$DBG_GLOBAL
echo >>$DBG_GLOBAL
cat $M_STATUSFILE >>$DBG_GLOBAL
echo >>$DBG_GLOBAL
echo "====== END RUN =========" >>$DBG_GLOBAL
echo >>$DBG_GLOBAL
}

#------------

bg_run()
{
echo >>$DBG_GLOBAL
echo "====== Starting bg process: $M -i $PIPE $*" >>$DBG_GLOBAL
echo >>$DBG_GLOBAL

/bin/rm -f $DBGFILE
$M -i $PIPE -v -d $DBGFILE $* &
echo $! >$PIDFILE
sleep 1
check_process run
check_pid_file
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
if [ ! -f "$M_PIDFILE" ] ; then
	test_ko
	return
fi

pid_from_file=`cat $M_PIDFILE`

test "X$pid" = "X$pid_from_file"
test_rc $?
}

#------------

base_path()
{
d=$1
[ -z "$d" ] && d=$BASE_PATH
echo "$d/$REL_BASE_PATH"
}

#------------
# Return status file path for a given dir

status_file()
{
echo "`base_path $1`.status"
}

#------------
# Check the status in a given dir

check_status_file()
{
checking $2 status file
sfile=`status_file $1`
test -f $status_file
test_rc $?
}

#------------

log_files()
{
ls -1 $BASE_PATH._*
}

#------------

nb_log_files()
{
log_files | wc -l
}

#------------

backup_links()
{
ls -1 $BASE_PATH.B.*
}

#------------

nb_backup_links()
{
backup_links | wc -l
}

#------------

fsize()
{
wc -c <$1
}

#------------

global_log_size()
{
cat $BASE_PATH._* | wc -c
}

#------------

option_check()
{
checking $1 option
$M $1 </dev/null $BASE_PATH
test_rc $?
}

#============================= main ===================================

full_cleanup
/bin/rm -rf $RUN_DIR $DBG_GLOBAL
mkdir -p $RUN_DIR
cd $RUN_DIR
mk_fifo

#-------------------------------

new_test Options

for opt in -h --help -V --version -R --refresh-only -v --verbose
do
	option_check $opt
done

checking -i option
$M -i /dev/null $BASE_PATH
test_rc $?

checking '-d option (return code)'
/bin/rm -rf $TMPFILE
$M -v -d $TMPFILE </dev/null $BASE_PATH
test_rc $?
checking '-d option (debug data)'
cat $TMPFILE	# log
c=`cat $TMPFILE | wc -c`
test "$c" -gt 0
test_rc $?

checking '-I option (return code)'
$M -I </dev/null $BASE_PATH 2>&1 | tee $TMPFILE
test_rc $?
checking '-I option (data)'
grep statistics $TMPFILE
test_rc $?

checking 'invalid option (short)'
$M -W
test_rc `inv_rc $?`

checking 'invalid option (long)'
$M --wrong
test_rc `inv_rc $?`

#-------------------------------

new_test 'Basic run (background)'

bg_run $BASE_PATH

checking if log file exists
test `nb_log_files` -gt 0
test_rc $?

kill_m

check_status_file

#-------------------------------

new_test 'Basic run (foreground)'

fg_run $BASE_PATH

checking log size
count=`cat $DATAFILE | wc -c`
test `global_log_size` = $count
test_rc $?

#-------------------------------

new_test 'Relative path support'

fg_run $REL_BASE_PATH

checking file path
count=`cat $DATAFILE | wc -c`
test `global_log_size` = $count
test_rc $?
cleanup

#---

fg_run ./$REL_BASE_PATH

checking "'./' relative path"
count=`cat $DATAFILE | wc -c`
test `global_log_size` = $count
test_rc $?
cleanup

#---

mkdir toto
cd toto
fg_run ../$REL_BASE_PATH

checking "'../' relative path"
count=`cat $DATAFILE | wc -c`
test `global_log_size` = $count
test_rc $?
cd ..
cleanup

#---

mkdir -p t1/t2/t3
cd t1/t2/t3
fg_run ../t3/../../t2/../../t1/../$REL_BASE_PATH

checking "complex back path"
count=`cat $DATAFILE | wc -c`
test `global_log_size` = $count
test_rc $?
cd ../../..
cleanup

#---

#-------------------------------

new_test 'Rotation / Purge / Status file'

# The shell script created by configure is not executable. So, we need to
# make the file executable before using it as a command.

# Running the command with a relative path tests more

chmod +x $RCMD

fg_run -s $SZ_LIMIT -k $KEEP_COUNT -C $REL_CMD -I $BASE_PATH

check_status_file

checking log file count
test `nb_log_files` = $KEEP_COUNT
test_rc $?

checking log "file size ($KEEP_COUNT files)"
for f in `log_files` ''
	do
	if [ -z "$f" ] ; then
		test_ok
		break
	fi
	if [ `fsize $f` -gt $SZ_LIMIT ] ; then
		test_ko
		break
	fi
done

checking rotate command variables
grep ROT_VAR_OK $RCMD_LOG
test_rc $?

checking rotate command count
n=`grep ROT_VAR_OK $RCMD_LOG | wc -l`
rcount=`grep 'rotate count' $DBGFILE | sed 's/^.*://'`
test $n = $rcount
test_rc $?

checking purge count
pcount=`grep 'remove_oldest count' $DBGFILE | sed 's/^.*://'`
test $pcount = `expr $rcount + 1 - $KEEP_COUNT`
test_rc $?

# Run again to check that the status file is retrieved correctly from disk

fg_run -s $SZ_LIMIT -k $KEEP_COUNT $BASE_PATH

checking log file count after restart
test `nb_log_files` = $KEEP_COUNT
test_rc $?

#-------------------------------

test_compression()
{
# $1 = name (suffix)
# $2 = CMD

name=$1
CMD=$2

if [ -n "$CMD" ] ; then

new_test "Compression ($name)"

# Run several times because we must have at least one rotation

count=0
for i in 1 2 3 4 5
	do
	fg_run -c $name -s $SZ_LIMIT -k $KEEP_COUNT -C $REL_CMD -I $BASE_PATH
	c=`cat $DATAFILE | wc -c`
	count=`expr $count + $c`
done

check_status_file

sz=0;
checking log file format
for f in `log_files` ''
	do
	if [ -z "$f" ] ; then
		test_ok
		break
	fi
	$CMD -t $f
	fsize=`$CMD -d <$f | wc -c`
	sz=`expr $sz + $fsize`
	if [ $? != 0 ] ; then
		test_ko
		break
	fi
done

checking uncompressed data
test $sz = $count
test_rc $?

checking rotate command variable
grep LOGMANAGER_COMPRESSION_is_$name $RCMD_LOG
test_rc $?

fi
}

test_compression gz "$GZIP_CMD"
test_compression bz2 "$BZIP2_CMD"

#-------------------------------

new_test Links

fg_run -s $SZ_LIMIT -k $KEEP_COUNT -l -L -C $RCMD $BASE_PATH

check_status_file

checking active link
ls -l $RUN_DIR
test -f $BASE_PATH
test_rc $?

c=`expr $KEEP_COUNT - 1`
checking backup link count
test `nb_backup_links` = $c
test_rc $?

checking "backup link existence ($c links)"
for f in `backup_links` ''
	do
	if [ -z "$f" ] ; then
		test_ok
		break
	fi
	if [ ! -f $f ] ; then
		test_ko
		break
	fi
done

#-------------------------------
#new_test Compression

# TODO

#-------------------------------
# End

rc=0
[ -f $RESFILE ] && rc=`cat $RESFILE`

cd $CHECK_DIR
full_cleanup

msg
if [ $rc != 0 ] ; then
	msg "************************** ERROR **********************************"
	msg "* One or more tests failed                                        *"
	msg "* Look in the check.log and check.dbg files for more information  *"
	msg "*******************************************************************"
else
	msg "============================ TESTS OK ============================"
fi
msg

exit $rc
