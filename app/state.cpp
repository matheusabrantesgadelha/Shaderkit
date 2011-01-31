/**
 * Copyright 2010,2011 Riku Palomäki.
 * This file is part of GLSL Lab.
 *
 * GLSL Lab is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3 as
 * published by the Free Software Foundation.
 *
 * GLSL Lab is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GLSL Lab.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "forward.hpp"
#include "state.hpp"
#include "material.hpp"

State::State() {
  m_data.push_back(Data());
}

int State::nextFreeLight() const {
  return nextFree(m_data.back().m_lights);
}

void State::setLight(int light_id, bool in_use) {
  if (in_use) {
    m_data.back().m_lights.insert(light_id);
    enable(GL_LIGHTING);
    enable(GL_LIGHT0 + light_id);
  } else {
    m_data.back().m_lights.remove(light_id);
    disable(GL_LIGHT0 + light_id);
  }
}

void State::enable(GLenum cap) {
  glEnable(cap);
}

void State::disable(GLenum cap) {
  glDisable(cap);
}

int State::reserveTexUnit(void* keyptr, QString keyname) {
  typedef QPair<void*, QString> P;
  QMap<P, int>& map = m_data.back().m_texunits;
  P key(keyptr, keyname);

  if (map.contains(key)) return map[key];

  int unit = nextFree(map.values().toSet());
  map[key] = unit;

  return unit;
}

void State::push() {
  m_data.push_back(m_data.back());
}

void State::pop() {
  if (m_data.size() <= 1) {
    Log::error("State push/pop mismatch");
  } else {
    m_data.pop_back();
  }
}

void State::pushMaterial(MaterialPtr m) {
  if (!m_materials.isEmpty() && m_materials.back()) m_materials.back()->unbind();
  m_materials.push_back(m);
  push();
  if (m) m->bind(*this);
}

void State::popMaterial() {
  if (m_materials.isEmpty()) {
    Log::error("State material push/pop mismatch");
  } else {
    if (m_materials.back()) m_materials.back()->unbind();
    m_materials.pop_back();
    pop();
    if (!m_materials.isEmpty() && m_materials.back()) m_materials.back()->bind(*this);
  }
}

int State::nextFree(const QSet<int>& lst, int id) const {
  while (lst.contains(id)) ++id;
  return id;
}
