#ifndef MATERIALPBR_H
#define MATERIALPBR_H

#include "Material.h"
#include "vec3.hpp"
#include <QOpenGLTexture>
#include "Mesh.h"
#include "MeshVBO.h"

class MaterialPBR : public Material
{
public:
  MaterialPBR(
      const std::shared_ptr<Camera> &io_camera,
      const std::shared_ptr<ShaderLib> &io_shaderLib,
      std::array<glm::mat4, 3>* io_matrices,
      QOpenGLContext* io_context,
      const glm::vec3 &_albedo,
      const float _ao,
      const float _exposure,
      const float _roughness,
      const float _metallic
      ) :
    Material(io_camera, io_shaderLib, io_matrices),
    m_albedo(_albedo),
    m_context(io_context),
    m_ao(_ao),
    m_exposure(_exposure),
    m_roughness(_roughness),
    m_metallic(_metallic)
  {}
  MaterialPBR(const MaterialPBR&) = default;
  MaterialPBR& operator=(const MaterialPBR&) = default;
  MaterialPBR(MaterialPBR&&) = default;
  MaterialPBR& operator=(MaterialPBR&&) = default;
  ~MaterialPBR() override = default;

  virtual void init() override;

  virtual void update() override;

  virtual const char* shaderFileName() const override;


private:
  void initCaptureMatrices();
  void initSphereMap();
  void initCubeMap(const Mesh &_cube, const MeshVBO &_vbo);
  void initIrradianceMap(const Mesh &_cube, const MeshVBO &_vbo);
  void initPrefilteredMap(const Mesh &_cube, const MeshVBO &_vbo);
  std::array<QMatrix4x4, 6>  m_captureViews;
  QMatrix4x4 m_captureProjection;

  std::unique_ptr<QOpenGLTexture> m_sphereMap;
  std::unique_ptr<QOpenGLTexture> m_cubeMap;
  std::unique_ptr<QOpenGLTexture> m_irradianceMap;
  std::unique_ptr<QOpenGLTexture> m_prefilteredMap;

  glm::vec3 m_albedo;
  QOpenGLContext* m_context;
  float m_ao;
  float m_exposure;
  float m_roughness;
  float m_metallic;

};

#endif // MATERIALPBR_H
