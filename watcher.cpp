#include "watcher.hpp"

#include <QDir>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

Watcher::WatchableObj::WatchableObj(WatchablePtr obj_, ino_t inode_)
  : obj(obj_), inode(inode_) {}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Watcher* Watcher::s_instance = 0;

Watcher& Watcher::instance() {
  if (!s_instance) s_instance = new Watcher;
  return *s_instance;
}

Watcher::Watcher() {
  connect(&m_watcher, SIGNAL(fileChanged(QString)),
          this, SLOT(update(QString)));
  connect(&m_watcher, SIGNAL(directoryChanged(QString)),
          this, SLOT(updateDir(QString)));
}

void Watcher::add(WatchablePtr obj, const QString& filename) {
  // Since vim/gedit/"almost any decent text editor" save files using temporary files
  // (first renaming the old one to backup file and then renaming temp file to the
  // target file), normal inotify watching fails. We make extra check for inode changes.
  struct stat buf;
  if (stat(file.toStdString().c_str(), &buf)) {
    qDebug() << "Could not watch" << file;
    return;
  }

  m_objects[filename] = WatchableObj(obj, buf.st_ino);
  m_watcher.addPath(filename);
}

void Watcher::update(const QString& filename) {
  Objects::iterator it = m_objects.find(filename);
  if (it != m_objects.end()) {
    WatchableObj& w = *it;

    struct stat buf;
    if (stat(filename.toStdString().c_str(), &buf)) {
      qDebug() << filename << "missing, starting to watch the directory";
      watchDir(filename);
      return;
    }

    w.obj->fileUpdated(filename);

    // If the inode is different than on last time, then we need to readd this file to listeners,
    // update the inode and maybe even stop watching the directory
    if (w.inode != buf.st_ino) {
      m_watcher.removePath(filename);
      m_watcher.addPath(filename);
      w.inode = buf.st_ino;
      forgetDir(filename);
    }
  }
}

void Watcher::watchDir(const QString& filename) {
  if (m_waiting.contains(filename)) return;
  m_waiting.insert(filename);
  QFileInfo info(filename);
  QString dir = info.absolutePath();

  // Start watching the directory, if not yet watched
  if (!m_directories.contains(dir)) m_watcher.addPath(dir);

  m_directories[dir].insert(filename);
}

void Watcher::forgetDir(const QString& filename) {
  if (!m_waiting.contains(filename)) return;
  m_waiting.remove(filename);
  QFileInfo info(filename);
  QString dir = info.absolutePath();

  m_directories[dir].remove(file);

  // Forget the directory watch, if there aren't any interesting files left
  if (m_directories[dir].empty()) {
    m_watcher.removePath(dir);
    m_directories.remove(dir);
  }
}

void Watcher::updateDir(const QString& dir) {
  if (!m_directories.contains(dir)) return;

  // tmp is a set of missing files in the directory. We try to update() every
  // one of these. If any of those succeed, the update will forgetDir it.
  QSet<QString> tmp = m_directories[path];
  for (QSet<QString>::iterator it = tmp.begin(); it != tmp.end(); it++)
    update(*it);
}
