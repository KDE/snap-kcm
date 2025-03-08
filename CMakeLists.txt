# SPDX-FileCopyrightText: 2025 Soumyadeep Ghosh <soumyadghosh@ubuntu.com>
#
# SPDX-License-Identifier: GPL-2.0-or-later

cmake_minimum_required(VERSION 3.16)

project(kcm_snap)

set(PROJECT_VERSION "6.3.80")
set(PROJECT_VERSION_MAJOR 6)

set(PROJECT_DEP_VERSION "6.2.0")
set(QT_MIN_VERSION "6.7.0")
set(KF6_MIN_VERSION "6.10.0")

set(SNAPD_MIN_VERSION "1.6.5")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)


find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
find_package(Snapd ${SNAPD_MIN_VERSION} REQUIRED)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED Core Quick Gui Qml)

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
    Config
    CoreAddons
    KCMUtils
    I18n
)

add_subdirectory(src)
