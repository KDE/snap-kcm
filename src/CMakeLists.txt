# SPDX-FileCopyrightText: 2025 Soumyadeep Ghosh <soumyadghosh@ubuntu.com>
#
# SPDX-License-Identifier: BSD-2-Clause

add_definitions(-DTRANSLATION_DOMAIN="kcm_snap")
kcmutils_add_qml_kcm(kcm_snap)
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
target_sources(kcm_snap PRIVATE
   snappermissions.cpp
   snappermissions.h
   kcmsnap.cpp
   kcmsnap.h
   kcmplug.cpp
   kcmplug.h
   snapbackend.cpp
   snapbackend.h
)
target_link_libraries(kcm_snap PRIVATE
    Qt::DBus
    Qt::Quick
    KF6::CoreAddons
    KF6::KCMUtils
    KF6::I18n
    Snapd::Core
)
