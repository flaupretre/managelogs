#
# Test rotate command
#
# Checks the variables received from managelogs
#
#==============================================================================

exec >>$RCMD_LOG 2>&1

check_vars()
{
set -x
[ -z "$LOGMANAGER_FILE_PATH" ] && return 1
[ -f "$LOGMANAGER_FILE_PATH" ] || return 1

[ -z "$LOGMANAGER_BASE_PATH" ] && return 1

[ -z "$LOGMANAGER_ROOT_DIR" ] && return 1
[ -d "$LOGMANAGER_ROOT_DIR" ] || return 1

[ -z "$LOGMANAGER_VERSION" ] && return 1

[ -z "$LOGMANAGER_TIME" ] && return 1

return 0
}

#---------------------------

check_vars
set +x
if [ $? = 0 ] ; then
	echo ROT_VAR_OK
else
	echo ROT_VAR_KO
fi

#==============================================================================
