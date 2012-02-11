/**
 * Copyright 2010-2012 Riku Palomäki.
 * This file is part of Shaderkit, http://www.shaderkit.org/.
 *
 * Shaderkit is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3 as
 * published by the Free Software Foundation.
 *
 * Shaderkit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Shaderkit.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SCENE_SAVER_HPP
#define SCENE_SAVER_HPP

#include "forward.hpp"

#include <QDateTime>

class QTimer;

/// Saving happens after a timeout (m_timeout), but is also restricted by
/// another timeout (m_min_interval). However, if the user keeps using the
/// ui all the time and the first timeout seems never to be passed by,
/// there is a third timeout (m_max_wait).

///                     (not saving before that)        (must save before this)
///                                     v                                 v
/// saved   changed     changed         v           (saving?)             v
/// --x--------x-----------x------------x--------------x------------------x--->
///   ^        ^           ^            ^              ^                  ^
///   ^        ^           |...............m_timeout...|                  ^
///   ^        ^                        ^                                 ^
///   |...........m_min_interval........|                                 ^
///            ^                                                          ^
///            |......................m_max_wait..........................|

class SceneSaver : public QObject {
  Q_OBJECT

public:
  SceneSaver(Scene &scene);
  ~SceneSaver();

  bool enabled() const;

public slots:
  void sceneChanged();
  void stateChanged();

private slots:
  void trigger();

private:
  void updateTimer();

  Scene& m_scene;

  /// "how long should be idle before triggering the saving process" (secs)
  float m_timeout;
  /// "how long minimum interval should be between two consecutive saves" (secs)
  float m_min_interval;
  /// "How long can we wait before forcing saving" (secs)
  float m_max_wait;

  QTimer* m_timer;

  QDateTime m_last_changed;
  QDateTime m_last_saved;
  QDateTime m_first_changed;
};

#endif // SCENE_SAVER_HPP