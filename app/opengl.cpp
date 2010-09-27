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

#include "opengl.hpp"

#include <QMap>
#include <iostream>

void _check_gl(const char* str, const char* file, int line) {
  static QMap<QString, int> s_errors;
  const int limit = 3;
  int e = glGetError();
  if (e != GL_NO_ERROR) {
    QString out = "%1:%2\t%3 (%4)";
    out = out.arg(file).arg(line).arg((const char*)(gluErrorString(e))).arg(str);
    int num = ++s_errors[out];
    if (num == limit) out += " (starting to ignore)";
    if (num <= limit) std::cerr << out.toStdString() << std::endl;
    //abort();
  }
}