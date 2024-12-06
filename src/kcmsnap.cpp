/**
 * SPDX-FileCopyrightText: 2024 Soumyadeep Ghosh <soumyadghosh@ubuntu.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kcmsnap.h"
#include "snapbackend.h"
#include <QBuffer>
#include <QImageReader>
#include <QSettings>
#include <QVariant>
#include <Snapd/Client>

using namespace Qt::Literals::StringLiterals;

KCMSnap::KCMSnap(QSnapdSnap *snap, const QList<QSnapdPlug *> plugs, const QList<QSnapdSlot *> slots)
    : m_snap(snap)
    , m_plugs(plugs)
    , m_slots(slots)
{
}

QSnapdSnap *KCMSnap::snap() const
{
    return m_snap;
}

const QList<QSnapdPlug *> KCMSnap::plugs() const
{
    return m_plugs;
}

const QList<QSnapdSlot *> KCMSnap::slots() const
{
    return m_slots;
}

QVariant KCMSnap::icon() const
{
    if (SnapBackend::invokAble(m_snap)) {
        for (int i = 0; i < m_snap->appCount(); ++i) {
            const auto app = m_snap->app(i);

            if (app->name() == m_snap->name()) {
                if (app->desktopFile().isEmpty()) {
                    for (int m = 0; m < m_snap->mediaCount(); ++m) {
                        if (m_snap->media(m)->type() == u"icon"_s) {
                            return QUrl(m_snap->media(m)->url());
                        }
                    }
                } else {
                    QSettings desktopFile(app->desktopFile(), QSettings::IniFormat);
                    desktopFile.beginGroup("Desktop Entry");
                    const QString iconName = desktopFile.value("Icon").toString();

                    if (!iconName.isEmpty()) {
                        return iconName;
                    }
                }
            }
        }
    }

    if (!m_snap->icon().isEmpty() && !m_snap->icon().startsWith(QLatin1Char('/'))) {
        qDebug() << m_snap->icon();
        return QUrl(m_snap->icon());
    }

    QSnapdClient client;
    auto req = client.getIcon(m_snap->name());
    req->runSync();

    if (req->error() != QSnapdRequest::NoError) {
        return u"package-x-generic"_s;
    }

    QBuffer buffer;
    buffer.setData(req->icon()->data());
    QImageReader reader(&buffer);
    const auto theIcon = QVariant::fromValue<QImage>(reader.read());

    return theIcon.isNull() ? u"package-x-generic"_s : theIcon;
}
