/**
 * SPDX-FileCopyrightText: 2025 Soumyadeep Ghosh <soumyadghosh@ubuntu.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once
#include "snapbackend.h"
#include <KQuickConfigModule>
class SnapPermissions : public KQuickConfigModule
{
    Q_OBJECT
    QML_ELEMENT

public:
    SnapPermissions(QObject *parent, const KPluginMetaData &data);

private:
    QSharedPointer<SnapBackend> const snap_model;
};
