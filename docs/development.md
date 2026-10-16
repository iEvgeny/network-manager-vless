# Development

## Setup

```sh
sudo apt build-dep ./                 # build dependencies from debian/control
sudo apt install python3-venv libgtk-4-bin
make venv                             # .venv with --system-site-packages, plus dev tools
make check                            # ruff, ruff format --check, mypy --strict, pytest
```

Build and try the plugin:

```sh
.venv/bin/meson setup build --prefix=/usr
.venv/bin/meson compile -C build
build-aux/build-deb.sh                # dist contains core, -gnome and -kde .debs
# Install the core plus one UI package; choose -gnome or -kde.
sudo apt install ./dist/network-manager-vless_*.deb ./dist/network-manager-vless-kde_*.deb
```

Installing the packages is the recommended way to test on a desktop: it puts every file
where NetworkManager, D-Bus, GNOME Settings and Plasma look for it. Runtime packages are
split into core, `-gnome` and `-kde`; core only suggests the UIs, so unrelated desktop
libraries are not installed by default. The KDE plugin is auto-detected for Qt 5/KF5 and
Qt 6/KF6; use `-Dplasma_qt_version=5` or `=6` to build only one ABI, or leave it at
`auto` to build every detected ABI. Use `-Dplasma_editor=enabled` to require a plugin or
`-Dplasma_editor=disabled` to skip it. Debian's default build includes the Qt 5/KF5 plugin
on Ubuntu 24.04; the Ubuntu 26.04 CI package sets `PLASMA_QT_VERSION=6` for Plasma 6.
`build-deb.sh` downloads the Xray-core release pinned in
`build-aux/xray.lock` and refuses it if the checksum does not match; set
`XRAY_ZIP=/path/to/Xray-linux-64.zip` to build offline.

## Tests

The tests need no running NetworkManager and no root. Optional parts are enabled by
the environment:

```sh
build-aux/fetch-xray.sh amd64 /tmp/xray         # pinned, verified Xray-core
gtk4-broadwayd :5 &                             # headless display for GTK 4
GDK_BACKEND=broadway BROADWAY_DISPLAY=:5 NM_VLESS_TEST_XRAY=/tmp/xray/xray \
    NM_VLESS_BUILD_DIR=build .venv/bin/pytest
```

- `tests/test_editor.py` loads the built editor plugins through libnm, as GNOME
  Settings does, and checks that loading and saving a connection keeps every setting,
  that validation works and that files with links can be imported. The GTK part needs
  a display; with Broadway you can also open <http://127.0.0.1:8085> in a browser to
  see the windows.
- `tests/test_service.py` uses an unprivileged user and network namespace
  (`unshare --user --map-root-user --net`), which allows creating TUN devices without
  root, and a private D-Bus daemon standing in for the system bus. It runs a real
  `Session` with xray, checks error reporting and drives `nm-vless-service` over D-Bus
  exactly like NetworkManager: `NeedSecrets`, rejected `Connect` calls, a successful
  `Connect` with `Config`/`Ip4Config`/`Ip6Config` signals, and `Disconnect`.

Tests are skipped where the build, a display, user namespaces or `dbus-run-session`
are unavailable.

## Layout

```text
src/editor/         GNOME Settings plugins (C, GTK 4)
  nm-vless-editor-plugin.c   NMVpnEditorPlugin: name, import, loads the editor
  nm-vless-editor.c          NMVpnEditor: the settings page
  nm-vless-dialog.ui         its layout (GtkBuilder, compiled into GResource)
  nm-vless-common.[ch]       keys and "nm-vless parse-link" helper
src/kde/             KDE Plasma NetworkManager editor (Qt 5/KF5 and Qt 6/KF6)
  vlessplugin.*       plugin factory and VPN link/file import
  vlesssettingwidget.*   connection editor and authentication prompt
  plasma-nm/          vendored Plasma NM editor ABI declarations
po/                   translations of the GNOME and KDE editors
debian/               Debian/Ubuntu packaging
build-aux/          fetch-xray.sh, xray.lock, build-deb.sh, release-notes.sh
src/nm_vless/
  link.py          vless:// parsing and formatting, VlessServer validation
  settings.py      VlessServer <-> vpn.data / vpn.secrets, tunnel options
  xray.py          locating and checking xray, building its JSON configuration
  tun.py           creating TUN devices
  vpnconfig.py     Config / Ip4Config / Ip6Config dictionaries for NetworkManager
  service.py       nm-vless-service: the NetworkManager VPN plugin (root)
  nmclient.py      libnm helpers for creating and updating profiles
  subscription.py  downloading, decoding and storing subscriptions
  sync.py          reconciling subscription servers with profiles
  cli.py           the nm-vless command
data/              NetworkManager, D-Bus, sysusers and systemd files
tests/             pytest suite
```

## Conventions

- Python 3.11+, fully typed; `mypy --strict` and `ruff` must pass.
- C: GLib/GObject style, no warnings with `warning_level=2`, only the factory
  functions exported (`gnu_symbol_visibility: hidden`).
- Link parsing lives only in Python; the editor calls `nm-vless parse-link`.
- New user-visible strings in the editor need a translation in `po/ru.po`
  (`xgettext` + `msgmerge`, see `po/POTFILES.in`).
- Every file starts with `SPDX-License-Identifier: GPL-2.0-or-later` (checked by tests).
- Pure logic lives in modules without GObject imports and is unit-tested.
- Never log, print or put in exception messages: user ids, links, subscription URLs.
- Keys in `vpn.data` are a public interface: add new keys, do not rename existing ones,
  and document them in [connection-settings.md](connection-settings.md).
- User-visible changes go to `CHANGELOG.md` under *Unreleased*.

## Testing the service end to end

The service must run as root and is started by NetworkManager, so end-to-end tests are
manual. Install into the system as described in the README, then:

```sh
nm-vless check
nm-vless import -                          # a test server
nmcli connection up "Test"
ip -brief address show | grep vless        # vless0 with 198.18.0.1/30
ip route get 1.1.1.1                       # dev vless0
ip route get <server IP>                   # dev wlan0/eth0, not vless0
resolvectl status                          # DNS on vless0
ps -o user,cmd -C xray                     # user nm-vless
curl -s https://ifconfig.me                # the server's address
nmcli connection down "Test"
journalctl -b -t nm-vless-service
```

Also check that killing Xray (`sudo pkill -x xray`) restarts it on the same `vless0`
device, and that killing it four times within five minutes makes NetworkManager report
the connection as failed and removes the device.

To try changes to the service, reinstall with `sudo meson install -C build`; the next
connection attempt starts the new code. Do not point the installed `.name` file at a
user-writable source tree: that code runs as root.

## GitHub Actions and downloadable packages

The `CI` workflow runs automatically on pushes and pull requests. On a push (or a manual
run) it uploads two 14-day artifacts: `network-manager-vless-deb-ubuntu-24.04-amd64`
and `network-manager-vless-deb-ubuntu-26.04-amd64`. Each ZIP contains the core `.deb`,
`network-manager-vless-gnome`, and `network-manager-vless-kde`; the 26.04 build includes
the Qt 6/KF6 Plasma editor. Pull-request runs validate the build but do not publish
artifacts.

To start a fresh build in GitHub: **Actions → CI → Run workflow**, then choose a branch.
The workflow must first be merged into the repository's default branch for the manual
button to be available. To repeat an existing run instead, open it and choose **Re-run all
jobs**, or use `gh run rerun <RUN_ID>` from this checkout. Download the ZIP from the
run's **Artifacts** section. The *Release* workflow is different: a `vX.Y.Z` tag starts a
public GitHub release, so do not use it just to build a test artifact.

## Releasing

1. Update the version in `meson.build`, `src/nm_vless/__init__.py` and
   `debian/changelog` (tests check they match) and move *Unreleased* entries in
   `CHANGELOG.md` under the new version.
2. `make check` and the manual end-to-end test above.
3. Push a tag `vX.Y.Z`. The *Release* workflow builds the `.deb` for amd64 and arm64
   and publishes them with `SHA256SUMS` as a GitHub release. The description is made by
   `build-aux/release-notes.sh X.Y.Z` (installation instructions and the
   `CHANGELOG.md` section of the version); run it before tagging to check it.

To update the bundled Xray-core, change `version` and both checksums in
`build-aux/xray.lock` (take them from the release's `.dgst` files) and run the
integration tests with the new binary.
