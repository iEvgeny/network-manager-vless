#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Install the build dependencies for a supported Ubuntu LTS target.
set -eu

ubuntu_version=${1:?usage: $0 UBUNTU_VERSION}

sudo apt-get update
case "$ubuntu_version" in
    22.04)
        # Jammy's archive Meson is older than this project's Meson minimum.
        # Install the other declared build dependencies from Ubuntu, then use a
        # pinned Meson in an isolated venv; build-deb.sh is called with -d there
        # to skip dpkg's check for the newer Meson version.
        sudo apt-get install --no-install-recommends -y \
            build-essential debhelper-compat cmake ninja-build pkgconf \
            libnm-dev libgtk-4-dev qtbase5-dev \
            libkf5coreaddons-dev libkf5i18n-dev libkf5networkmanagerqt-dev \
            libkf5widgetsaddons-dev plasma-nm libglib2.0-dev \
            gettext curl unzip ca-certificates python3 python3-gi \
            gir1.2-nm-1.0 python3-pytest python3-venv
        meson_venv="${RUNNER_TEMP:-${TMPDIR:-/tmp}}/meson-build-tools"
        python3 -m venv --system-site-packages "$meson_venv"
        "$meson_venv/bin/pip" install --disable-pip-version-check meson==1.12.1 pytest==9.1.1
        if [ -n "${GITHUB_PATH:-}" ]; then
            printf '%s\n' "$meson_venv/bin" >> "$GITHUB_PATH"
        fi
        ;;
    24.04)
        sudo apt-get build-dep -y .
        ;;
    26.04)
        sudo apt-get build-dep -y .
        sudo apt-get install --no-install-recommends -y \
            qt6-base-dev \
            libkf6coreaddons-dev \
            libkf6i18n-dev \
            libkf6networkmanagerqt-dev \
            libkf6widgetsaddons-dev
        ;;
    *)
        echo "$0: unsupported Ubuntu LTS version: $ubuntu_version" >&2
        exit 2
        ;;
esac
