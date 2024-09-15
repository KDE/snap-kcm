/**
 * SPDX-FileCopyrightText: 2024 Soumyadeep Ghosh <soumyadghosh@ubuntu.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once
#include "snapbackend.h"
#include <KQuickConfigModule>
#include <QList>
#include <QObject>
#include <Snapd/Client>

class SnapPermissions : public KQuickConfigModule
{
    Q_OBJECT

public:
    SnapPermissions(QObject *parent, const KPluginMetaData &data);

private:
    SnapBackend *const snap_model;
};
