#!/bin/bash

if ! which git 2>&1 > /dev/null ; then
    echo "Git not found. Please add to your path."
    exit 1
fi

MAJOR=0
MINOR=1
REVISION=0
GIT_HASH=`git rev-parse --short HEAD`

PRINT_MINOR=$MINOR
if [ ${PRINT_MINOR} -lt 10 ] ; then
    PRINT_MINOR=0${MINOR}
fi

PRINT_REVISION=$REVISION
if [ ${PRINT_REVISION} -lt 10 ] ; then
    PRINT_REVISION=0${REVISION}
fi
cat > ../../../../src/FWVersionGenerated.h << EOF
#pragma once

// !!! This file was auto-generated.
// DO NOT CHANGE !!!

class FWVersionGenerated {
 public:
  static constexpr unsigned int Major = $MAJOR;
  static constexpr unsigned int Minor = $MINOR;
  static constexpr unsigned int Revision = $REVISION;
  static constexpr char const * const VerStr = "${MAJOR}.${PRINT_MINOR}.${PRINT_REVISION}";
  static constexpr char const * const GitHash = "$GIT_HASH";
  static constexpr char const * const BuildDate = __DATE__;
  static constexpr char const * const BuildTime = __TIME__;
};
EOF
