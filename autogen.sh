#!/bin/sh

find $(pwd) -name configure.ac | xargs touch

# Regenerate configuration files
mkdir -vp m4 wxorp/m4
autoreconf -i --force -I m4 || exit 1

