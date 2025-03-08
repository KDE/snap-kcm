/**
 * SPDX-FileCopyrightText: 2025 Soumyadeep Ghosh <soumyadghosh@ubuntu.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kcmsnap.h"
#include "kcmplug.h"
#include "snapbackend.h"
#include <KDesktopFile>
#include <QBuffer>
#include <QImageReader>
#include <QVariant>
#include <Snapd/Client>

using namespace Qt::Literals::StringLiterals;

KCMSnap::KCMSnap(QSnapdSnap *snap, const QList<KCMPlug *> plugs)
    : m_snap(snap)
    , m_plugs(plugs)
{
}

QSnapdSnap *KCMSnap::snap() const
{
    return m_snap;
}

const QList<KCMPlug *> KCMSnap::plugs() const
{
    return m_plugs;
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
                    KDesktopFile desktopFile(app->desktopFile());
                    const QString iconName = desktopFile.readIcon();

                    if (!iconName.isEmpty()) {
                        return iconName;
                    }
                }
            }
        }
    }

    if (!m_snap->icon().isEmpty() && !m_snap->icon().startsWith(QLatin1Char('/'))) {
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
