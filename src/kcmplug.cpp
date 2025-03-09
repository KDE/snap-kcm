/**
 * SPDX-FileCopyrightText: 2025 Soumyadeep Ghosh <soumyadghosh@ubuntu.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kcmplug.h"
#include "snapbackend.h"

KCMPlug::KCMPlug(QSnapdPlug *plug, QString plugLabel, QString plugIcon)
    : m_plug(plug)
    , m_plugLabel(plugLabel)
    , m_plugIcon(plugIcon)
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

QString KCMPlug::plugIcon() const
{
    return m_plugIcon;
}
QString KCMPlug::plugInterface() const
{
    return m_plug->interface();
}

QString KCMPlug::plugLabel() const
{
    return m_plugLabel;
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

QString KCMPlug::title() const
{
    return SnapBackend::capitalize(m_plug->name());
}