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

The release includes Ubuntu-specific packages. Select files ending in
\`_ubuntu-24.04.deb\` for Ubuntu 24.04 or \`_ubuntu-26.04.deb\` for Ubuntu 26.04.
The 24.04 build is available for \`amd64\` and \`arm64\`; the 26.04 build is currently
available for \`amd64\` only. The Plasma editor is built with Qt 5 on Ubuntu 24.04 and
Qt 6 on Ubuntu 26.04.

For GNOME on Ubuntu 24.04 (replace \`amd64\` with \`arm64\` on ARM):

\`\`\`sh
sudo apt install ./network-manager-vless_${package_version}_amd64_ubuntu-24.04.deb \\
                 ./network-manager-vless-gnome_${package_version}_amd64_ubuntu-24.04.deb
\`\`\`

For KDE Plasma 5 on Ubuntu 24.04, replace \`-gnome\` with \`-kde\` in the second filename.
For GNOME on Ubuntu 26.04:

\`\`\`sh
sudo apt install ./network-manager-vless_${package_version}_amd64_ubuntu-26.04.deb \\
                 ./network-manager-vless-gnome_${package_version}_amd64_ubuntu-26.04.deb
\`\`\`

For KDE Plasma 6 on Ubuntu 26.04, replace \`-gnome\` with \`-kde\` in the second filename.
Install the core package together with the editor for your desktop.

Then add a connection in **Settings → Network → VPN → + → VLESS (Xray)**. Ubuntu 24.04
or newer and other distributions with NetworkManager 1.24+ are supported; see the
[README]($repo/blob/v${version}/README.md) ([на русском]($repo/blob/v${version}/README.ru.md)).
If GNOME Settings was open during an update, restart it.

Verify a download with \`sha256sum -c SHA256SUMS --ignore-missing\`.

## Changes
$changes
NOTES
