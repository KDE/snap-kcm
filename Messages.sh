#! /usr/bin/env bash
# SPDX-FileCopyrightText: 2025 Soumyadeep Ghosh <soumyadghosh@ubuntu.com>
# SPDX-License-Identifier: GPL-2.0-or-later

$XGETTEXT $(find . -name "*.cpp" -o -name "*.qml") -o $podir/kcm_snap.pot