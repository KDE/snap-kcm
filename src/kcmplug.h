/**
 * SPDX-FileCopyrightText: 2025 Soumyadeep Ghosh <soumyadghosh@ubuntu.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once
#include <Snapd/Plug>
#include <Snapd/Slot>
#include <Snapd/plug.h>
#include <qtmetamacros.h>

class KCMPlug : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(int connectedSlotCount READ connectedSlotCount CONSTANT)
    Q_PROPERTY(QString plugSnap READ plugSnap CONSTANT)
    Q_PROPERTY(QString plugInterface READ plugInterface CONSTANT)
    Q_PROPERTY(QString connectedSlotSnap READ connectedSlotSnap CONSTANT)

public:
    KCMPlug(QSnapdPlug *plug);
    QString name() const;
    int connectedSlotCount() const;
    QString plugSnap() const;
    QString plugInterface() const;
    QString connectedSlotSnap() const;

private:
    QSnapdPlug *m_plug;
};