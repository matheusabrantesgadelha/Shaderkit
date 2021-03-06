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

#include "gl/fbo.hpp"
#include "gl/opengl.hpp"

#include <cassert>

namespace Shaderkit
{

  FBOImage::FBOImage(const QString& name)
    : SceneObject(name),
      m_role(*this),
      m_render_buffer_id(0),
      m_attachment(*this, 0),
      m_active_attachment(0),
      m_width(0),
      m_height(0),
      m_fbo_num(0)
  {}

  FBOImage::FBOImage(const FBOImage& f)
    : QObject(),
      SceneObject(f),
      std::enable_shared_from_this<FBOImage>(),
      m_role(*this, f.m_role),
      m_render_buffer_id(0),
      m_attachment(*this, f.m_attachment),
      m_active_attachment(0),
      m_width(f.m_width),
      m_height(f.m_height),
      m_fbo_num(0)
  {
  }

  RenderBuffer::RenderBuffer(const QString& name) : FBOImage(name) {}

  RenderBuffer::~RenderBuffer()
  {
    if (m_render_buffer_id)
      glDeleteRenderbuffers(1, &m_render_buffer_id);
  }

  void RenderBuffer::bind()
  {
    glRun(glBindRenderbuffer(GL_RENDERBUFFER, m_render_buffer_id));
  }

  void RenderBuffer::unbind()
  {
    glRun(glBindRenderbuffer(GL_RENDERBUFFER, 0));
  }

  void RenderBuffer::setup(unsigned int fbo, int width, int height)
  {
    if (m_render_buffer_id == 0) glRun(glGenRenderbuffers(1, &m_render_buffer_id));

    bool type_changed = m_attachment != m_active_attachment,
         size_changed = m_width != width || m_height != height,
         fbo_changed = !m_fbo_num != fbo;

    if (type_changed || size_changed) {
      /// @todo needs m_internalformat
      int format = m_attachment == GL_DEPTH_ATTACHMENT ? GL_DEPTH_COMPONENT24 :
                   m_attachment == GL_STENCIL_ATTACHMENT ? GL_DEPTH24_STENCIL8 : /** @todo is this right? */
                   GL_RGBA;
      bind();
      glRun(glRenderbufferStorageMultisample(GL_RENDERBUFFER, 0 /* samples */,
                                             format, width, height));
      unbind();
    }

    /// @todo Should we run this if only size was changed?
    if (type_changed || size_changed || fbo_changed)
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, m_attachment, GL_RENDERBUFFER, m_render_buffer_id);

    m_width = width;
    m_height = height;
    m_active_attachment = m_attachment;
    if (fbo_changed) m_fbo_num = fbo;
  }

  QVariantMap FBOImage::toMap() const
  {
    QVariantMap map = SceneObject::toMap();
    if (!m_role->isEmpty()) map["role"] = m_role.value();
    return map;
  }

  void FBOImage::load(QVariantMap map)
  {
    SceneObject::load(map);
    if (map.contains("role"))
      m_role = map["role"].toString();
  }

  void FBOImage::dataUpdated() {}

  void FBOImage::setFBO(FBOPtr fbo)
  {
    FBOPtr old = m_fbo.lock();
    m_fbo.reset();
    if (old && old != fbo) {
      old->remove(shared_from_this());
    }
    m_fbo = fbo;
  }

  void FBOImage::attributeChanged()
  {
    emit changed(shared_from_this());
  }

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

  FrameBufferObject::FrameBufferObject() : m_id(0), m_width(0), m_height(0)
  {
  }

  FrameBufferObject::~FrameBufferObject()
  {
    if (m_id)
      glRun(glDeleteFramebuffers(1, &m_id));

    foreach (FBOImagePtr img, m_buffers)
      img->setFBO(FBOPtr());
  }

  void FrameBufferObject::resize(int width, int heigth)
  {
    m_width = width;
    m_height = heigth;
  }

  void FrameBufferObject::set(int type, FBOImagePtr buffer)
  {
    remove(buffer);
    buffer->setAttachment(type);
    buffer->setFBO(shared_from_this());
    m_buffers[type] = buffer;
  }

  void FrameBufferObject::clearBuffers()
  {
    foreach (FBOImagePtr img, m_buffers)
      img->setFBO(FBOPtr());
    foreach (int i, m_buffers.keys())
      m_lazyRemove << i;
    m_buffers.clear();
  }

  void FrameBufferObject::remove(FBOImagePtr buffer)
  {
    foreach (int i, m_buffers.keys(buffer)) {
      m_buffers.remove(i);
      m_lazyRemove << i;
    }
    if (buffer) buffer->setFBO(FBOPtr());
  }

  void FrameBufferObject::bind()
  {
    if (m_id == 0)
      glRun(glGenFramebuffers(1, &m_id));

    glRun(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_id));

    int max_attachments = colorAttachments();
    GLenum* buffers = new GLenum[max_attachments];
    for (int i = 0; i < max_attachments; ++i)
      buffers[i] = m_buffers.contains(GL_COLOR_ATTACHMENT0 + i)
                   ? GL_COLOR_ATTACHMENT0 + i : GL_NONE;
    glRun(glDrawBuffers(max_attachments, buffers));
    delete[] buffers;

    for (auto it = m_buffers.begin(); it != m_buffers.end(); ++it) {
      (*it)->setup(m_id, m_width, m_height);
      m_lazyRemove.remove(it.key());
    }

    foreach (int i, m_lazyRemove) {
      glRun(glFramebufferTexture2D(GL_FRAMEBUFFER, i, GL_TEXTURE_2D, 0, 0));
      glRun(glFramebufferRenderbuffer(GL_FRAMEBUFFER, i, GL_RENDERBUFFER, 0));
    }
    m_lazyRemove.clear();

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
    if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
      Log::error("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: Not all framebuffer attachment points are framebuffer attachment complete");

      printDebug();

      /*
        * <image> is a component of an existing object with the name
          specified by FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, and of the
          type specified by FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE.

        * The width and height of <image> must be non-zero.

        * If FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is TEXTURE and
          FRAMEBUFFER_ATTACHMENT_OBJECT_NAME names a 3-dimensional
          texture, then FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER must be
          smaller than the depth of the texture.

        * If <attachment> is COLOR_ATTACHMENTi, then <image> must have
          a color-renderable internal format.

        * If <attachment> is DEPTH_ATTACHMENT, then <image> must have
          a depth-renderable internal format.

        * If <attachment> is STENCIL_ATTACHMENT, then <image> must
          have a stencil-renderable internal format.

        */
    } else if (status == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT) {
      Log::error("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT: Not all attached images have the same width and height");
    } else if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
      Log::error("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: No images are attached to the framebuffer");
    } else if (status == GL_FRAMEBUFFER_UNSUPPORTED) {
      Log::error("GL_FRAMEBUFFER_UNSUPPORTED: The combination of internal formats of the attached "
                 "images violates an implementation-dependent set of restrictions");
    } else if (status == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE) {
      Log::error("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: The value of RENDERBUFFER_SAMPLES is not the same for all attached images");
    } else if (status != GL_FRAMEBUFFER_COMPLETE) {
      Log::error("FBO status is not complete!");
    }
    glCheck("FrameBufferObject::bind()");
  }

  void FrameBufferObject::unbind()
  {
    glRun(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
  }

  void FrameBufferObject::printDebug()
  {
    for (int i = -1; i < 16; ++i) {
      int attachment = i == -1 ? GL_DEPTH_ATTACHMENT : GL_COLOR_ATTACHMENT0 + i;
      QString bufname = i == -1 ? "depth" : QString("color %1").arg(i);
      int type = GL_NONE;
      glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment,
                                            GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
      if (type == GL_NONE) {
        Log::info(bufname + ": NONE");
        continue;
      } else if (type == GL_FRAMEBUFFER_DEFAULT) {
        Log::info(bufname + ": FRAMEBUFFER_DEFAULT");
        continue;
      } else if (type != GL_TEXTURE && type != GL_RENDERBUFFER) {
        Log::error(bufname + ": ERROR");
        continue;
      }

      int name = 0;
      glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment,
                                            GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &name);

      Log::info(bufname + ": %s %d", type == GL_TEXTURE ? "TEXTURE" : "RENDERBUFFER", name);
    }
  }

  int FrameBufferObject::colorAttachments()
  {
    static GLint m_value = -1;
    if (m_value <= 0) {
      glRun(glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &m_value));
    }
    return m_value;
  }

} // namespace Shaderkit
