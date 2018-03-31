#ifndef MATERIALHDRCUBE_H
#define MATERIALHDRCUBE_H

#include "Material.h"
#include <QOpenGLTexture>
#include <QImage>

class HDR_cube
{
public:
  HDR_cube() = default;
  HDR_cube(const HDR_cube&) = default;
  HDR_cube& operator=(const HDR_cube&) = default;
  HDR_cube(HDR_cube&&) = default;
  HDR_cube& operator=(HDR_cube&&) = default;
  ~HDR_cube() = default;

  void init(const std::shared_ptr<ShaderLib> &io_shaderLib, QOpenGLContext* io_context);

  unsigned int irradianceMapID() const noexcept { return m_irradianceMap; }

private:
  unsigned int m_irradianceMap;
  unsigned int initSphereMap(QOpenGLContext *io_context);

};

#endif // MATERIALHDRCUBE_H
