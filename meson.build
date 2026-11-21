# SPDX-License-Identifier: GPL-2.0-or-later
project('NetworkManager-vless',
  version: '0.5.0',
  license: 'GPL-2.0-or-later',
  meson_version: '>= 1.1.0',
  default_options: ['c_std=gnu11', 'warning_level=2', 'werror=false'],
)

gnome = import('gnome')
i18n = import('i18n')
python = import('python').find_installation(get_option('python'), modules: ['gi'])

gettext_package = 'NetworkManager-vless'
prefix = get_option('prefix')
bindir = prefix / get_option('bindir')
libexecdir = prefix / get_option('libexecdir')
pkglibexecdir = libexecdir / 'nm-vless'
pkgdatadir = prefix / get_option('datadir') / 'nm-vless'
docdir = get_option('docdir') != '' ? get_option('docdir') : prefix / get_option('datadir') / 'doc' / 'NetworkManager-vless'

# Integration directories are absolute: NetworkManager, D-Bus and systemd only
# look in fixed locations, independently of the installation prefix.
nm_vpn_dir = get_option('nm_vpn_dir')
if nm_vpn_dir == ''
  nm_vpn_dir = '/usr/lib/NetworkManager/VPN'
endif
systemd = dependency('systemd', required: false)
sysusers_dir = get_option('sysusers_dir')
if sysusers_dir == ''
  sysusers_dir = systemd.get_variable(pkgconfig: 'sysusersdir', default_value: '/usr/lib/sysusers.d')
endif
user_unit_dir = get_option('systemd_user_unit_dir')
if user_unit_dir == ''
  user_unit_dir = systemd.get_variable(pkgconfig: 'systemduserunitdir',
    default_value: '/usr/lib/systemd/user')
endif

# ---- Desktop connection editors -----------------------------------------------------------------

# GNOME Settings uses libnm's GTK 4 VPN editor API.
editor_opt = get_option('editor')
libnm_dep = dependency('libnm', version: '>= 1.24', required: editor_opt)
gtk4_dep = dependency('gtk4', version: '>= 4.6', required: editor_opt)
build_editor = libnm_dep.found() and gtk4_dep.found() and add_languages('c', required: editor_opt)

nm_plugin_dir = prefix / get_option('libdir') / 'NetworkManager'
libnm_section = ''
if build_editor
  gmodule_dep = dependency('gmodule-2.0', version: '>= 2.70')
  dl_dep = meson.get_compiler('c').find_library('dl', required: false)
  add_project_arguments(
    meson.get_compiler('c').get_supported_arguments(
      '-Wno-unused-parameter', '-Wno-missing-field-initializers', '-Wshadow',
      '-Wstrict-prototypes', '-Wformat=2', '-Werror=implicit-function-declaration'),
    language: 'c')
  libnm_section = '\n[libnm]\nplugin=' + nm_plugin_dir / 'libnm-vpn-plugin-vless.so\n'
  subdir('src/editor')
endif

# KDE Plasma's NetworkManager editor has its own Qt/KPlugin API; it does not load
# NMVpnEditorPlugin. Probe both supported Plasma generations so the same source
# builds as a Qt 5 plugin on Plasma 5 and a Qt 6 plugin on Plasma 6.
plasma_editor_opt = get_option('plasma_editor')
have_plasma_cpp = false
have_plasma_c = build_editor
if not plasma_editor_opt.disabled()
  have_plasma_cpp = add_languages('cpp', required: plasma_editor_opt, native: false)
  if not have_plasma_c
    have_plasma_c = add_languages('c', required: plasma_editor_opt, native: false)
  endif
endif

build_plasma5_editor = false
build_plasma6_editor = false
plasma_qt_version = get_option('plasma_qt_version')
if have_plasma_cpp and have_plasma_c
  libnm_plasma_dep = dependency('libnm', version: '>= 1.24', required: false)
  # The VPN editor API (VpnUiPlugin and SettingWidget) is exported by plasma-nm
  # and is not supplied by Qt/KF. Link the module to its provider explicitly;
  # otherwise Plasma's plugin loader rejects it for unresolved symbols.
  plasma_nm_editor_dep = meson.get_compiler('cpp').find_library('plasmanm_editor', required: false)
  # CMAKE_PROJECT_INCLUDE is blacklisted by Meson, so use its project-specific
  # equivalent for Meson's generated CMake project (MesonTemp). This also keeps
  # KF5/KF6 generated QCH exports compatible with CMake's CMP0160 policy.
  cmake_policy_include = meson.project_source_root() / 'build-aux' / 'meson-cmake-policy.cmake'
  kf_cmake_args = ['-DCMAKE_PROJECT_MesonTemp_INCLUDE_BEFORE=' + cmake_policy_include]

  if plasma_qt_version != '6'
    qt5_widgets_dep = dependency('Qt5Widgets', version: '>= 5.15', required: false)
    kf5_coreaddons_dep = dependency(
      'KF5CoreAddons', method: 'cmake', modules: 'KF5::CoreAddons',
      cmake_args: kf_cmake_args, required: false)
    kf5_i18n_dep = dependency(
      'KF5I18n', method: 'cmake', modules: 'KF5::I18n',
      cmake_args: kf_cmake_args, required: false)
    kf5_networkmanagerqt_dep = dependency(
      'KF5NetworkManagerQt', method: 'cmake', modules: 'KF5::NetworkManagerQt',
      cmake_args: kf_cmake_args, required: false)
    kf5_widgetsaddons_dep = dependency(
      'KF5WidgetsAddons', method: 'cmake', modules: 'KF5::WidgetsAddons',
      cmake_args: kf_cmake_args, required: false)
    build_plasma5_editor = (libnm_plasma_dep.found() and plasma_nm_editor_dep.found()
      and qt5_widgets_dep.found() and kf5_coreaddons_dep.found()
      and kf5_i18n_dep.found() and kf5_networkmanagerqt_dep.found()
      and kf5_widgetsaddons_dep.found())
  endif

  if plasma_qt_version != '5'
    qt6_widgets_dep = dependency('Qt6Widgets', version: '>= 6.4', required: false)
    kf6_coreaddons_dep = dependency(
      'KF6CoreAddons', method: 'cmake', modules: 'KF6::CoreAddons',
      cmake_args: kf_cmake_args, required: false)
    kf6_i18n_dep = dependency(
      'KF6I18n', method: 'cmake', modules: 'KF6::I18n',
      cmake_args: kf_cmake_args, required: false)
    kf6_networkmanagerqt_dep = dependency(
      'KF6NetworkManagerQt', method: 'cmake', modules: 'KF6::NetworkManagerQt',
      cmake_args: kf_cmake_args, required: false)
    kf6_widgetsaddons_dep = dependency(
      'KF6WidgetsAddons', method: 'cmake', modules: 'KF6::WidgetsAddons',
      cmake_args: kf_cmake_args, required: false)
    build_plasma6_editor = (libnm_plasma_dep.found() and plasma_nm_editor_dep.found()
      and qt6_widgets_dep.found() and kf6_coreaddons_dep.found()
      and kf6_i18n_dep.found() and kf6_networkmanagerqt_dep.found()
      and kf6_widgetsaddons_dep.found())
  endif
endif

build_plasma_editor = build_plasma5_editor or build_plasma6_editor
if plasma_editor_opt.enabled() and not build_plasma_editor
  error('KDE Plasma editor requested but the selected Qt version, libnm, plasma-nm editor library and matching KDE Frameworks development files were not found')
endif
if build_plasma_editor
  subdir('src/kde')
endif

# Both editors use the same gettext catalog; compile it if either one is built.
if build_editor or build_plasma_editor
  subdir('po')
endif

# ---- bundled Xray-core ----------------------------------------------------------------------------

bundled_xray = get_option('bundled_xray')
bundled_xray_path = ''
if bundled_xray != ''
  bundled_xray_path = pkglibexecdir / 'xray'
  install_data(bundled_xray, rename: 'xray', install_dir: pkglibexecdir,
    install_mode: 'rwxr-xr-x')
endif

# ---- Python package, service and tool ------------------------------------------------------------

conf = configuration_data()
conf.set('PYTHON', python.full_path())
conf.set('PKGDATADIR', pkgdatadir)
conf.set('LIBEXECDIR', libexecdir)
conf.set('BINDIR', bindir)
conf.set('DOCDIR', docdir)
conf.set('BUNDLED_XRAY', bundled_xray_path)
conf.set('LOCALEDIR', prefix / get_option('localedir'))
conf.set('LIBNM_SECTION', libnm_section)

install_subdir('src/nm_vless',
  install_dir: pkgdatadir,
  exclude_directories: ['__pycache__'],
  exclude_files: ['_build_paths.py', '_build_paths.py.in'],
)
configure_file(
  input: 'src/nm_vless/_build_paths.py.in',
  output: '_build_paths.py',
  configuration: conf,
  install_dir: pkgdatadir / 'nm_vless',
)

configure_file(
  input: 'src/nm-vless-service.in',
  output: 'nm-vless-service',
  configuration: conf,
  install_dir: libexecdir,
  install_mode: 'rwxr-xr-x',
)
configure_file(
  input: 'src/nm-vless-auth-dialog.in',
  output: 'nm-vless-auth-dialog',
  configuration: conf,
  install_dir: libexecdir,
  install_mode: 'rwxr-xr-x',
)
configure_file(
  input: 'src/nm-vless.in',
  output: 'nm-vless',
  configuration: conf,
  install_dir: bindir,
  install_mode: 'rwxr-xr-x',
)
configure_file(
  input: 'data/nm-vless-service.name.in',
  output: 'nm-vless-service.name',
  configuration: conf,
  install_dir: nm_vpn_dir,
)
configure_file(
  input: 'data/nm-vless-subscriptions.service.in',
  output: 'nm-vless-subscriptions.service',
  configuration: conf,
  install_dir: user_unit_dir,
)
configure_file(
  input: 'data/nm-vless-proxy-guard.service.in',
  output: 'nm-vless-proxy-guard.service',
  configuration: conf,
  install_dir: user_unit_dir,
)
install_data('data/nm-vless-subscriptions.timer', install_dir: user_unit_dir)
install_data('data/nm-vless-service.conf', install_dir: get_option('dbus_policy_dir'))
install_data('data/nm-vless.sysusers.conf', rename: 'nm-vless.conf', install_dir: sysusers_dir)
install_data('README.md', 'README.ru.md', 'CHANGELOG.md', install_dir: docdir)

meson.add_install_script('build-aux/meson-postinstall.sh', sysusers_dir / 'nm-vless.conf')

pytest = find_program('pytest', required: false)
if pytest.found()
  test('pytest', pytest,
    args: ['-q', meson.project_source_root() / 'tests'],
    workdir: meson.project_source_root(),
    env: {'NM_VLESS_BUILD_DIR': meson.project_build_root()},
    timeout: 300,
  )
endif

summary({
  'prefix': prefix,
  'python': python.full_path(),
  'NetworkManager VPN dir': nm_vpn_dir,
  'GNOME Settings editor': build_editor,
  'libnm plugin dir': build_editor ? nm_plugin_dir : '-',
  'KDE Plasma 5 editor': build_plasma5_editor,
  'KDE Plasma 6 editor': build_plasma6_editor,
  'bundled xray': bundled_xray != '' ? bundled_xray_path : 'no (uses /usr/local/bin/xray)',
  'D-Bus policy dir': get_option('dbus_policy_dir'),
  'sysusers.d': sysusers_dir,
  'systemd user units': user_unit_dir,
})
