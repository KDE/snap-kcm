/**
 * SPDX-FileCopyrightText: 2024 Soumyadeep Ghosh <soumyadghosh@ubuntu.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once
#include "kcmsnap.h"

#include <Snapd/Client>

class SnapBackend : public QObject
{
    Q_OBJECT

public:
    explicit SnapBackend();
    Q_INVOKABLE const QList<KCMSnap *> snaps(const QString &filter = QString()) const;
    // Q_INVOKABLE const QList<KCMSnap *> slotSnaps() const;
    Q_INVOKABLE QString connectPlug(const QString &plug_snap, const QString &plug_name, const QString &slot_snap, const QString &slot_name) const;
    Q_INVOKABLE QString disconnectPlug(const QString &plug_snap, const QString &plug_name, const QString &slot_snap, const QString &slot_name) const;
    Q_INVOKABLE static const QString plugLabel(const QSnapdPlug *plug);
    Q_INVOKABLE static const QString plugIcon(const QString &plugName);
    Q_INVOKABLE static const QString capitalize(const QString &text);
    Q_INVOKABLE static bool invokAble(QSnapdSnap *snap);
    Q_INVOKABLE void invokeDesktopApp(QSnapdSnap *snap) const;
    static bool comparebyName(QSnapdSnap *, QSnapdSnap *);

Q_SIGNALS:
    void snapsChanged();

private:
    QSnapdClient m_client;
    QList<KCMSnap *> m_snaps;
};
