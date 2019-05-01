#!/bin/sh

# This shell script sets the shared library path and runs CartoTypeMaps: see http://doc.qt.io/qt-5/linux-deployment.html

appname=`basename $0 | sed s,\.sh$,,`

dirname=`dirname $0`
tmp="${dirname#?}"

if [ "${dirname%$tmp}" != "/" ]; then
dirname=$PWD/$dirname
fi
LD_LIBRARY_PATH=$dirname
export LD_LIBRARY_PATH
$dirname/$appname "$@"
