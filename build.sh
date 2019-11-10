#!/bin/sh
./clean.sh
cmake -G "Unix Makefiles" ./;make ${MAKE_J}
