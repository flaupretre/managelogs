#-- Uncomment the line corresponding to your environment
#- 32 bits :
%define libdir /usr/lib
#- 64 bits :
#%%define libdir /usr/lib64
#-----------------------------------
Name: managelogs
Summary: managelogs log management software
Group: Applications/Internet
Version: 1.1.0
Release: 0
ExclusiveOs: Linux
License: Apache license 2.0
Source: none
URL: http://managelogs.tekwire.net/
Packager: F. Laupretre
AutoReqProv: no
Requires: httpd bzip2-libs zlib
provides: managelogs
%description
managelogs is a log management program for Apache, like rotatelogs and cronolog.
It allows to rotate and purge the Apache log files based on different size
limits. It also brings a lot of other features, like running as a given
non-root user, on-the-fly compression, maintaining symbolic links on log files,
ensuring that rotation occurs on line boundaries, and more.
%files
/usr/bin/managelogs
%{libdir}/liblogmanager.la
%{libdir}/liblogmanager.so
%{libdir}/liblogmanager.so.1
%{libdir}/liblogmanager.so.1.0.0
/usr/include/logmanager.h
/usr/share/man/man8/managelogs.8
