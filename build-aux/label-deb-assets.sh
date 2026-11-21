#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Add the target Ubuntu release to generated binary package filenames.
set -eu

ubuntu_version=${1:?usage: $0 UBUNTU_VERSION UBUNTU_CODENAME}
ubuntu_codename=${2:?usage: $0 UBUNTU_VERSION UBUNTU_CODENAME}
count=0

for package in dist/*.deb; do
    [ -f "$package" ] || continue
    mv "$package" "${package%.deb}_ubuntu-${ubuntu_version}-${ubuntu_codename}.deb"
    count=$((count + 1))
done

if [ "$count" -eq 0 ]; then
    echo "$0: no .deb packages found in dist/" >&2
    exit 1
fi

ls -1 dist/*.deb
