#ifndef MATERIALBUMP_H
#define MATERIALBUMP_H

#include "Material.h"
#include <QOpenGLTexture>
#include <QImage>

class MaterialBump : public Material
{
public:
  MaterialBump(const std::shared_ptr<Camera> &io_camera, const std::shared_ptr<ShaderLib> &io_shaderLib, std::array<glm::mat4, 3>* io_matrices) :
    Material(io_camera, io_shaderLib, io_matrices)
  {}
  MaterialBump(const MaterialBump&) = default;
  MaterialBump& operator=(const MaterialBump&) = default;
  MaterialBump(MaterialBump&&) = default;
  MaterialBump& operator=(MaterialBump&&) = default;
  ~MaterialBump() override = default;

  virtual void init() override;

  virtual void update() override;

  virtual const char* shaderFileName() const override;

private:
  void initColor();
  void initNormal();
  std::unique_ptr<QOpenGLTexture> m_colorMap;
  std::unique_ptr<QOpenGLTexture> m_normalMap;
};

#endif // MATERIALBUMP_H
