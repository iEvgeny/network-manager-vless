# SPDX-License-Identifier: GPL-2.0-or-later
# plasma-nm uses its own VPN editor plugin API, distinct from libnm's GTK editor API.

kde_conf = configuration_data()
kde_conf.set_quoted('NM_VLESS_CLI_PATH', bindir / 'nm-vless')
configure_file(output: 'config.h', configuration: kde_conf)

kde_includes = [
  include_directories('.'),
  include_directories('../editor'),
  include_directories('plasma-nm'),
]
kde_sources = files(
  'vlessplugin.cpp',
  'vlesssettingwidget.cpp',
  '../editor/nm-vless-common.c',
)

plasma5_plugin_dir = get_option('plasma5_plugin_dir')
if plasma5_plugin_dir == ''
  plasma5_plugin_dir = prefix / get_option('libdir') / 'qt5' / 'plugins' / 'plasma' / 'network' / 'vpn'
endif

plasma6_plugin_dir = get_option('plasma6_plugin_dir')
if plasma6_plugin_dir == ''
  plasma6_plugin_dir = prefix / get_option('libdir') / 'qt6' / 'plugins' / 'plasma' / 'network' / 'vpn'
endif

if build_plasma5_editor
  qt5 = import('qt5')
  plasma5_deps = [
    libnm_plasma_dep,
    plasma_nm_editor_dep,
    qt5_widgets_dep,
    kf5_coreaddons_dep,
    kf5_i18n_dep,
    kf5_networkmanagerqt_dep,
    kf5_widgetsaddons_dep,
  ]
  plasma5_moc = qt5.preprocess(
    moc_headers: files('vlessplugin.h'),
    moc_sources: files('vlessplugin.cpp'),
    dependencies: plasma5_deps,
  )
  shared_module('plasmanetworkmanagement_vlessui_qt5',
    kde_sources + plasma5_moc,
    name_prefix: 'lib',
    include_directories: kde_includes,
    dependencies: plasma5_deps,
    cpp_args: ['-std=c++17'],
    install: true,
    install_dir: plasma5_plugin_dir,
  )
endif

if build_plasma6_editor
  qt6 = import('qt6')
  plasma6_deps = [
    libnm_plasma_dep,
    plasma_nm_editor_dep,
    qt6_widgets_dep,
    kf6_coreaddons_dep,
    kf6_i18n_dep,
    kf6_networkmanagerqt_dep,
    kf6_widgetsaddons_dep,
  ]
  plasma6_moc = qt6.preprocess(
    moc_headers: files('vlessplugin.h'),
    moc_sources: files('vlessplugin.cpp'),
    dependencies: plasma6_deps,
  )
  shared_module('plasmanetworkmanagement_vlessui_qt6',
    kde_sources + plasma6_moc,
    name_prefix: 'lib',
    include_directories: kde_includes,
    dependencies: plasma6_deps,
    cpp_args: ['-std=c++17'],
    install: true,
    install_dir: plasma6_plugin_dir,
  )
endif
