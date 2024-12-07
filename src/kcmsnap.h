/**
 * SPDX-FileCopyrightText: 2024 Soumyadeep Ghosh <soumyadghosh@ubuntu.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once
#include <Snapd/Plug>
#include <Snapd/Slot>
#include <Snapd/Snap>

class KCMSnap : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QSnapdSnap *snap READ snap CONSTANT)
    Q_PROPERTY(QList<QSnapdPlug *> plugs READ plugs CONSTANT)
    Q_PROPERTY(QList<QSnapdSlot *> slots READ slots CONSTANT)
    Q_PROPERTY(QVariant icon READ icon CONSTANT)

public:
    KCMSnap(QSnapdSnap *snap, const QList<QSnapdPlug *> plugs, const QList<QSnapdSlot *> slots);
    QSnapdSnap *snap() const;
    const QList<QSnapdPlug *> plugs() const;
    const QList<QSnapdSlot *> slots() const;
    QVariant icon() const;

private:
    QSnapdSnap *m_snap;
    QList<QSnapdPlug *> m_plugs;
    QList<QSnapdSlot *> m_slots;
};
