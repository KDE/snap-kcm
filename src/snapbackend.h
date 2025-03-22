/**
 * SPDX-FileCopyrightText: 2025 Soumyadeep Ghosh <soumyadghosh@ubuntu.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once
#include "kcmsnap.h"

#include <Snapd/Client>
#include <Snapd/Interface>

class SnapBackend : public QObject
{
    Q_OBJECT

public:
    explicit SnapBackend();
    Q_INVOKABLE const QList<KCMSnap *> snaps(const QString &filter = QString()) const;
    // Q_INVOKABLE const QList<KCMSnap *> slotSnaps() const;
    static QString connectPlug(const QString &plug_snap, const QString &plug_name, const QString &slot_snap, const QString &slot_name);
    static QString disconnectPlug(const QString &plug_snap, const QString &plug_name, const QString &slot_snap, const QString &slot_name);
    static const QString plugLabel(const QString &plugName);
    static const QString plugIcon(const QString &plugName);
    Q_INVOKABLE static const QString capitalize(const QString &text);
    Q_INVOKABLE void invokeDesktopApp(const QString &desktop) const;
    Q_INVOKABLE const QStringList getSlotSnap(const QString &interface) const;
    Q_INVOKABLE const QString getPlugLabel(const QString &interface);
    static bool comparebyName(QSnapdSnap *, QSnapdSnap *);
    static QString readableError(QSnapdRequest *request);

Q_SIGNALS:
    void snapsChanged();

private:
    QSnapdClient m_client;
    QList<QSnapdInterface *> m_interfaces;
    QList<KCMSnap *> m_snaps;
};
