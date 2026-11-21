#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Print the GitHub release description for VERSION: installation instructions and
# the CHANGELOG.md section of that version.
#
#   build-aux/release-notes.sh 0.5.0 > notes.md
set -eu

version=${1:?usage: $0 VERSION}
repo=https://github.com/${GITHUB_REPOSITORY:-iEvgeny/network-manager-vless}
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

The release contains packages for every released Ubuntu LTS series currently supported:
Jammy 22.04, Noble 24.04 and Resolute 26.04. Each package filename includes its target
Ubuntu series before the architecture (for example, \`_ubuntu-22.04-jammy_amd64.deb\`).
Builds are available for both
\`amd64\` and \`arm64\` on all three series. Jammy and Noble use the Plasma 5/Qt 5 editor;
Resolute uses the Plasma 6/Qt 6/KF6 editor.

Choose the appropriate \`UBUNTU\` and \`ARCH\` values, then install the core package and
one desktop editor. For example, GNOME on Jammy:

\`\`\`sh
VERSION=${package_version}
ARCH=amd64
UBUNTU=22.04-jammy
sudo apt install "./network-manager-vless_\${VERSION}_ubuntu-\${UBUNTU}_\${ARCH}.deb" \\
                 "./network-manager-vless-gnome_\${VERSION}_ubuntu-\${UBUNTU}_\${ARCH}.deb"
\`\`\`

For Noble set \`UBUNTU=24.04-noble\`; for Resolute set \`UBUNTU=26.04-resolute\`. Replace
\`-gnome\` with \`-kde\` in the UI package name for KDE Plasma. Use \`ARCH=arm64\` for ARM
systems.

Then add a connection in **Settings → Network → VPN → + → VLESS (Xray)**. Ubuntu 22.04
or newer and other distributions with NetworkManager 1.24+ and Python 3.10+ are supported;
see the [README]($repo/blob/v${version}/README.md)
([на русском]($repo/blob/v${version}/README.ru.md)). If the desktop settings editor was
open during an update, restart it.

Verify a download with \`sha256sum -c SHA256SUMS --ignore-missing\`.

## Changes
$changes
NOTES
