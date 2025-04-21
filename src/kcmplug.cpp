/**
 * SPDX-FileCopyrightText: 2025 Soumyadeep Ghosh <soumyadghosh@ubuntu.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kcmplug.h"
#include "snapbackend.h"

KCMPlug::KCMPlug(QSnapdPlug *plug, const QString &plugLabel, const QString &plugIcon, const QStringList &slotSnaps)
    : m_plug(plug)
    , m_plugLabel(plugLabel)
    , m_plugIcon(plugIcon)
    , m_slotSnaps(slotSnaps)
{
    if (plug->connectedSlotCount() > 0) {
        m_connectedSlotSnap = plug->connectedSlot(0)->snap();
    }
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
    return m_connectedSlotSnap;
}

void KCMPlug::setconnectedSlotSnap(const QString &slotSnap)
{
    m_connectedSlotSnap = slotSnap;
    Q_EMIT connectedSlotSnapChanged(slotSnap);
}

QStringList KCMPlug::slotSnaps() const
{
    QStringList slotSnaps = m_slotSnaps;
    if (!connectedSlotSnap().isEmpty() && m_slotSnaps.contains(connectedSlotSnap())) {
        slotSnaps.removeOne(connectedSlotSnap());
        slotSnaps.insert(0, connectedSlotSnap());
    }
    return slotSnaps;
}

QString KCMPlug::title() const
{
    return SnapBackend::capitalize(m_plug->name());
}

void KCMPlug::changePermission(bool connect, QString slotSnap)
{
    QString errorString;
    if (connect) {
        errorString = SnapBackend::connectPlug(plugSnap(), name(), slotSnap, plugInterface());
    } else {
        errorString = SnapBackend::disconnectPlug(plugSnap(), name(), slotSnap, plugInterface());
    }
    if (errorString.isEmpty()) {
        setconnectedSlotSnap(connect ? slotSnap : QString());
    } else {
        Q_EMIT errorLogChanged(errorString);
    }
}