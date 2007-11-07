/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License either version 2, or
 *   (at your option) any later version as published by the Free Software
 *   Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "filebrowserengine.h"

#include "plasma/datacontainer.h"

#include <QDir>
#include <KDirWatch>
#include <KDebug>
#include <KFileMetaInfo>

#define InvalidIfEmpty(A) ((A.isEmpty())?(QVariant()):(QVariant(A)))
#define forMatchingSources for (DataEngine::SourceDict::iterator it = sources.begin(); it != sources.end(); it++) \
  if (dir == QDir(it.key()))

FileBrowserEngine::FileBrowserEngine(QObject* parent, const QVariantList& args) :
    Plasma::DataEngine(parent), m_dirWatch(NULL)
{
    Q_UNUSED(args)

    m_dirWatch = new KDirWatch(this);
    connect(m_dirWatch, SIGNAL(created(
        const QString &)), this, SLOT(dirCreated(const QString &)));
    connect(m_dirWatch, SIGNAL(deleted(
        const QString &)), this, SLOT(dirDeleted(const QString &)));
    connect(m_dirWatch, SIGNAL(dirty(
        const QString &)), this, SLOT(dirDirty(const QString &)));
}

FileBrowserEngine::~FileBrowserEngine()
{
    delete m_dirWatch;
}

void FileBrowserEngine::init()
{
    kDebug() << "init() called";
}

bool FileBrowserEngine::sourceRequested(const QString &path)
{
    kDebug() << "source requested() called: "<< path;
    m_dirWatch->addDir(path);
    setData(path, "type", QVariant("unknown"));
    updateData (path, INIT);
    return true;
}

void FileBrowserEngine::dirDirty(const QString &path)
{
    updateData(path, DIRTY);
}

void FileBrowserEngine::dirCreated(const QString &path)
{
    updateData(path, CREATED);
}

void FileBrowserEngine::dirDeleted(const QString &path)
{
    updateData(path, DELETED);
}

void FileBrowserEngine::updateData(const QString &path, EventType event)
{
    Q_UNUSED(event)

    ObjectType type = NOTHING;
    if (QDir(path).exists()) {
        type = DIRECTORY;
    } else if (QFile::exists(path)) {
        type = FILE;
    }

    DataEngine::SourceDict sources = sourceDict();

    QDir dir(path);
    clearData(path);

    if (type == DIRECTORY) {
        kDebug() << "directory info processing: "<< path;
        if (dir.isReadable()) {
            QStringList visibleFiles = dir.entryList(QDir::Files, QDir::Name);
            QStringList allFiles = dir.entryList(QDir::Files | QDir::Hidden,
                    QDir::Name);

            QStringList visibleDirectories = dir.entryList(QDir::Dirs
                    | QDir::NoDotAndDotDot, QDir::Name);
            QStringList allDirectories = dir.entryList(QDir::Dirs
                    | QDir::NoDotAndDotDot | QDir::Hidden, QDir::Name);

            forMatchingSources {
                kDebug() << "MATCH";
                it.value()->setData("item.type", QVariant("directory"));

                it.value()->setData("directories.visible", InvalidIfEmpty(visibleDirectories));
                it.value()->setData("directories.all", InvalidIfEmpty(allDirectories));
                it.value()->setData("files.visible", InvalidIfEmpty(visibleFiles));
                it.value()->setData("files.all", InvalidIfEmpty(allFiles));
            }
        }
    } else if (type == FILE) {
        kDebug() << "file info processing: "<< path;
        KFileMetaInfo kfmi(path, QString::null, KFileMetaInfo::Everything);
        if (kfmi.isValid()) {
            kDebug() << "METAINFO: " << kfmi.keys();

            forMatchingSources {
                kDebug() << "MATCH";
                it.value()->setData("item.type", QVariant("file"));

                for (QHash< QString, KFileMetaInfoItem >::const_iterator i = kfmi.items().constBegin(); i != kfmi.items().constEnd(); i++) {
                    it.value()->setData(i.key(), i.value().value());
                }
            }
        }
    } else {
        forMatchingSources {
            it.value()->setData("item.type", QVariant("imaginary"));
        }
    };

    checkForUpdates();

}

void FileBrowserEngine::clearData(const QString &path)
{
    QDir dir(path);
    DataEngine::SourceDict sources = sourceDict();
    for (DataEngine::SourceDict::iterator it = sources.begin(); it
            != sources.end(); it++) {
        if (dir == QDir(it.key())) {
            kDebug() << "matched: "<< path << " "<< it.key();
            it.value()->clearData();

        } else {
            kDebug() << "didn't match: "<< path << " "<< it.key();
        }
    }
}

#include "filebrowserengine.moc"
