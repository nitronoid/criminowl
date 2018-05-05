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
      const float _roughness,
      const float _metallic
      ) :
    Material(io_camera, io_shaderLib, io_matrices),
    m_albedo(_albedo),
    m_context(io_context),
    m_ao(_ao),
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
  void initBrdfLUTMap(const Mesh &_plane, const MeshVBO &_vbo);

  void generateCubeMap(const Mesh &_cube,
      const MeshVBO &_vbo,
      std::unique_ptr<QOpenGLTexture> &_texture,
      const int _dim,
      const std::string &_matPath,
      const std::function<void (QOpenGLShaderProgram* io_prog)> &_prerender = [](QOpenGLShaderProgram*){}
  );

  void generate3DTexture(
      const Mesh &_plane,
      const MeshVBO &_vbo,
      std::unique_ptr<QOpenGLTexture> &_texture,
      const std::string &_matPath,
      const std::function<void (QOpenGLShaderProgram* io_prog)> &_prerender = [](QOpenGLShaderProgram*){}
  );

  std::array<QMatrix4x4, 6>  m_captureViews;
  QMatrix4x4 m_captureProjection;

  std::unique_ptr<QOpenGLTexture> m_sphereMap;
  std::unique_ptr<QOpenGLTexture> m_cubeMap;
  std::unique_ptr<QOpenGLTexture> m_irradianceMap;
  std::unique_ptr<QOpenGLTexture> m_prefilteredMap;
  std::unique_ptr<QOpenGLTexture> m_brdfMap;
  std::unique_ptr<QOpenGLTexture> m_albedoMap;
  std::unique_ptr<QOpenGLTexture> m_normalMap;

  glm::vec3 m_albedo;
  QOpenGLContext* m_context;
  float m_ao;
  float m_roughness;
  float m_metallic;

};

#endif // MATERIALPBR_H
