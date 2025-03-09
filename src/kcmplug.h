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
    Q_PROPERTY(QString plugIcon READ plugIcon CONSTANT)
    Q_PROPERTY(QString plugLabel READ plugLabel CONSTANT)
    Q_PROPERTY(QString connectedSlotSnap READ connectedSlotSnap CONSTANT)
    Q_PROPERTY(QString title READ title CONSTANT)

public:
    KCMPlug(QSnapdPlug *plug, QString m_plugLabel, QString m_plugIcon);
    QString name() const;
    int connectedSlotCount() const;
    QString plugSnap() const;
    QString plugIcon() const;
    QString plugInterface() const;
    QString plugLabel() const;
    QString connectedSlotSnap() const;
    QString title() const;

private:
    QSnapdPlug *m_plug;
    QString m_plugLabel;
    QString m_plugIcon;
};