#!/bin/bash

LIBDIR=$(cd $(dirname "$0"); echo $PWD)
export LD_PRELOAD="$LIBDIR/bg3_linux_ae.so${LD_PRELOAD:+:$LD_PRELOAD}"
exec "$@"
