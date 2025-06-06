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
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(QString desktopFile READ desktopFile CONSTANT)
    Q_PROPERTY(bool invokable READ invokable CONSTANT)
    Q_PROPERTY(QVariant icon READ icon CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QList<KCMPlug *> plugs READ plugs CONSTANT)
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)

public:
    KCMSnap(QSnapdSnap *snap, const QList<KCMPlug *> plugs);
    QString description() const;
    QString desktopFile() const;
    bool invokable() const;
    QVariant icon() const;
    QString name() const;
    const QList<KCMPlug *> plugs() const;
    QString title() const;
    QString version() const;

private:
    QSnapdSnap *m_snap;
    QList<KCMPlug *> m_plugs;
};
