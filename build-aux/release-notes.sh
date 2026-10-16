#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Print the GitHub release description for VERSION: installation instructions and
# the CHANGELOG.md section of that version.
#
#   build-aux/release-notes.sh 0.5.0 > notes.md
set -eu

version=${1:?usage: $0 VERSION}
repo=https://github.com/${GITHUB_REPOSITORY:-s11va9/network-manager-vless}
src=$(cd "$(dirname "$0")/.." && pwd)
package_version=$(dpkg-parsechangelog --file "$src/debian/changelog" --show-field Version)

changes=$(awk -v heading="## [$version]" '
    index($0, heading) == 1 { found = 1; next }
    found && /^## \[/ { exit }
    found { print }
' "$src/CHANGELOG.md")
if [ -z "$(printf '%s' "$changes" | tr -d '[:space:]')" ]; then
    echo "$0: CHANGELOG.md has no section for $version" >&2
    exit 1
fi

cat <<NOTES
## Installation

Download the core package and the UI package for your desktop below (\`amd64\` for
most PCs, \`arm64\` for ARM devices), then install both from a terminal. For GNOME:

\`\`\`sh
sudo apt install ./network-manager-vless_${package_version}_amd64.deb \\
                 ./network-manager-vless-gnome_${package_version}_amd64.deb
\`\`\`

For KDE Plasma 5, replace \`-gnome\` with \`-kde\` in the second filename. The release
packages target Ubuntu 24.04; the CI artifact for Ubuntu 26.04 contains the Qt 6/KF6 Plasma
editor. You can also install just the core package if you do not need a desktop editor.

Then add a connection in **Settings → Network → VPN → + → VLESS (Xray)**. Ubuntu 24.04
or newer and other distributions with NetworkManager 1.24+ are supported; see the
[README]($repo/blob/v${version}/README.md) ([на русском]($repo/blob/v${version}/README.ru.md)).
If GNOME Settings was open during an update, restart it.

Verify a download with \`sha256sum -c SHA256SUMS --ignore-missing\`.

## Changes
$changes
NOTES
