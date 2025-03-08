/**
 * SPDX-FileCopyrightText: 2025 Soumyadeep Ghosh <soumyadghosh@ubuntu.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kcmplug.h"

KCMPlug::KCMPlug(QSnapdPlug *plug)
    : m_plug(plug)
{
}

QString KCMPlug::name() const
{
    return m_plug->name();
}
int KCMPlug::connectedSlotCount() const
{
    return m_plug->connectedSlotCount();
}

QString KCMPlug::plugInterface() const
{
    return m_plug->interface();
}

QString KCMPlug::plugSnap() const
{
    return m_plug->snap();
}

QString KCMPlug::connectedSlotSnap() const
{
    if (m_plug->connectedSlotCount() > 0) {
        return m_plug->connectedSlot(0)->snap();
    }
    return QString();
}