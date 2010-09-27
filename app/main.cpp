/**
 * Copyright 2010 Riku Palomäki.
 * This file is part of GLSL Lab.
 *
 * GLSL Lab is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GLSL Lab is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GLSL Lab.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainwindow.hpp"
#include "welcome.hpp"
#include "project.hpp"
#include "shaderdb/shaderdb.hpp"

#include <QApplication>
#include <QDir>

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  ShaderDB db;
  {
    db.addPath(QDir::currentPath());
    db.addPath(QDir::currentPath() + "/examples");
    db.addPath(QDir::currentPath() + "/..");
    db.addPath(QDir::currentPath() + "/../examples");
    QDir dir(argv[0]);
    if (dir.cdUp()) {
      db.addPath(dir.path());
      db.addPath(dir.path() + "/examples");
      db.addPath(dir.path() + "/..");
      db.addPath(dir.path() + "/../examples");
      db.addPath(dir.path() + "../share/glsl-lab/examples");
    }
    db.addPath(QDir::homePath() + "/.glsl-lab/shaderdb");
  }

  MainWindow window;
  Welcome welcome(window);
  welcome.show();

  return app.exec();
}