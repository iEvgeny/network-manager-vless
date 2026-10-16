# SPDX-License-Identifier: GPL-2.0-or-later
"""Consistency checks between the Python package and the build files."""

from __future__ import annotations

import configparser
import json
import re
from pathlib import Path

import nm_vless
from nm_vless.cli import build_parser

ROOT = Path(__file__).resolve().parent.parent


def test_version_matches_meson() -> None:
    meson = (ROOT / "meson.build").read_text()
    match = re.search(r"version:\s*'([^']+)'", meson)
    assert match is not None
    assert match.group(1) == nm_vless.__version__


def test_changelog_mentions_version() -> None:
    assert f"## [{nm_vless.__version__}]" in (ROOT / "CHANGELOG.md").read_text()


def test_plugin_name_file() -> None:
    template = (ROOT / "data" / "nm-vless-service.name.in").read_text()
    parser = configparser.ConfigParser()
    parser.read_string(template.replace("@LIBNM_SECTION@", ""))
    section = parser["VPN Connection"]
    assert section["service"] == nm_vless.SERVICE_TYPE
    assert section["program"].endswith("/nm-vless-service")
    # GNOME Shell's secret agent needs an auth dialog for every VPN type.
    assert parser["GNOME"]["auth-dialog"].endswith("/nm-vless-auth-dialog")
    assert parser["GNOME"]["supports-external-ui-mode"] == "true"


def test_kde_plasma_plugin_metadata() -> None:
    metadata = json.loads((ROOT / "src/kde/plasmanetworkmanagement_vlessui.json").read_text())
    assert metadata["KPlugin"]["Name"] == "VLESS (Xray)"
    assert metadata["KPlugin"]["Version"] == nm_vless.__version__
    assert metadata["X-NetworkManager-Services"] == nm_vless.SERVICE_TYPE
    assert (
        "option('plasma_editor', type: 'feature', value: 'auto'"
        in (ROOT / "meson.options").read_text()
    )
    kde_meson = (ROOT / "src/kde/meson.build").read_text()
    assert "plasmanetworkmanagement_vlessui_qt5" in kde_meson
    assert "plasmanetworkmanagement_vlessui_qt6" in kde_meson


def test_dbus_policy_matches_service() -> None:
    policy = (ROOT / "data/nm-vless-service.conf").read_text()
    assert f'own_prefix="{nm_vless.SERVICE_TYPE}"' in policy


def test_every_source_file_has_spdx_header() -> None:
    files = [
        *ROOT.glob("src/**/*.py"),
        *ROOT.glob("src/**/*.c"),
        *ROOT.glob("src/**/*.h"),
        *ROOT.glob("src/**/*.cpp"),
        *ROOT.glob("tests/*.py"),
        *ROOT.glob("src/*.in"),
    ]
    # These declarations are vendored from plasma-nm and retain their upstream LGPL terms.
    files = [f for f in files if "plasma-nm" not in f.parts]
    missing = [
        f.name for f in files if "SPDX-License-Identifier: GPL-2.0-or-later" not in f.read_text()
    ]
    assert missing == []


def test_cli_help_builds() -> None:
    help_text = build_parser().format_help()
    for command in ("import", "export", "list", "subscription", "check", "parse-link"):
        assert command in help_text


def test_debian_changelog_matches_version() -> None:
    first = (ROOT / "debian" / "changelog").read_text().splitlines()[0]
    match = re.search(r"\(([^)]+)\)", first)
    assert match is not None
    # Debian's package revision can change without changing the app's upstream version.
    assert match.group(1) == nm_vless.__version__ or match.group(1).startswith(
        f"{nm_vless.__version__}-"
    )


def test_translations_are_complete() -> None:
    po = (ROOT / "po" / "ru.po").read_text()
    assert 'msgstr ""\n\n' not in po.split("\n\n", 1)[1]
