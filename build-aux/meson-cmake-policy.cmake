# SPDX-License-Identifier: GPL-2.0-or-later
# Meson's CMake dependency probe sets the policy version to the installed CMake
# version. KF5 5.115's generated QCH targets set read-only imported properties,
# which fails under CMP0160 NEW (CMake 3.29+). Keep the legacy behavior for these
# package-discovery probes until those exported target files are corrected.
if(POLICY CMP0160)
    cmake_policy(SET CMP0160 OLD)
endif()
