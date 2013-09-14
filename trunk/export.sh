#!/bin/sh
V=`svnversion`
rm -rf export_tmp
mkdir export_tmp
svn export . export_tmp/ccons
cd export_tmp
DST="ccons-r$V.tar.gz"
tar cvzf $DST ccons
ls -al $DST
mv $DST ..
