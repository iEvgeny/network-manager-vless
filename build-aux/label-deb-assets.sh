#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Rename binary package assets to include Ubuntu target before architecture.
set -eu

ubuntu_version=${1:?usage: $0 UBUNTU_VERSION UBUNTU_CODENAME}
ubuntu_codename=${2:?usage: $0 UBUNTU_VERSION UBUNTU_CODENAME}

case "$ubuntu_version" in
    *[!0-9.]*|'') echo "$0: invalid Ubuntu version: $ubuntu_version" >&2; exit 2 ;;
esac
case "$ubuntu_codename" in
    *[!a-z]*|'') echo "$0: invalid Ubuntu codename: $ubuntu_codename" >&2; exit 2 ;;
esac

count=0
for package in dist/*.deb; do
    [ -f "$package" ] || continue
    name=$(dpkg-deb -f "$package" Package)
    version=$(dpkg-deb -f "$package" Version)
    arch=$(dpkg-deb -f "$package" Architecture)
    destination="dist/${name}_${version}_ubuntu-${ubuntu_version}-${ubuntu_codename}_${arch}.deb"
    if [ -e "$destination" ]; then
        echo "$0: refusing to overwrite $destination" >&2
        exit 1
    fi
    mv "$package" "$destination"
    count=$((count + 1))
done

if [ "$count" -eq 0 ]; then
    echo "$0: no .deb packages found in dist/" >&2
    exit 1
fi

ls -1 dist/*.deb
