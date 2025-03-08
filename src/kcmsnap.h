/**
 * SPDX-FileCopyrightText: 2025 Soumyadeep Ghosh <soumyadghosh@ubuntu.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once
#include "kcmplug.h"
#include <Snapd/Plug>
#include <Snapd/Slot>
#include <Snapd/Snap>

class KCMSnap : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QSnapdSnap *snap READ snap CONSTANT)
    Q_PROPERTY(QList<KCMPlug *> plugs READ plugs CONSTANT)
    Q_PROPERTY(QVariant icon READ icon CONSTANT)

public:
    KCMSnap(QSnapdSnap *snap, const QList<KCMPlug *> plugs);
    QSnapdSnap *snap() const;
    const QList<KCMPlug *> plugs() const;
    QVariant icon() const;

private:
    QSnapdSnap *m_snap;
    QList<KCMPlug *> m_plugs;
};
