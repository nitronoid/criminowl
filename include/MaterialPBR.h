#ifndef MATERIALPBR_H
#define MATERIALPBR_H

#include "Material.h"
#include "vec3.hpp"
#include <QOpenGLTexture>
#include "TriMesh.h"
#include "MeshVBO.h"

class MaterialPBR : public Material
{
public:
  MaterialPBR(
      const std::shared_ptr<Camera> &io_camera,
      const std::shared_ptr<ShaderLib> &io_shaderLib,
      std::array<glm::mat4, 3>* io_matrices,
      QOpenGLContext* io_context,
      const float _ao,
      const float _roughness,
      const float _metallic,
      const float _baseSpec,
      const float _normalStrength
      ) :
    Material(io_camera, io_shaderLib, io_matrices),
    m_context(io_context),
    m_ao(_ao),
    m_roughness(_roughness),
    m_metallic(_metallic),
    m_baseSpec(_baseSpec),
    m_normalStrength(_normalStrength)
  {}
  MaterialPBR(const MaterialPBR&) = default;
  MaterialPBR& operator=(const MaterialPBR&) = default;
  MaterialPBR(MaterialPBR&&) = default;
  MaterialPBR& operator=(MaterialPBR&&) = default;
  ~MaterialPBR() override = default;

  virtual void init() override;

  virtual void update() override;

  virtual const char* shaderFileName() const override;

  void setMetallic(const float _metallic) noexcept;
  float getMetallic() const noexcept;

  void setRoughness(const float _roughness) noexcept;
  float getRoughness() const noexcept;

  void setBaseSpec(const float _baseSpec) noexcept;
  float getBaseSpec() const noexcept;

  void setNormalStrength(const float _normalStrength) noexcept;
  float getNormalStrength() const noexcept;

  void setPaused(const bool _paused) noexcept;
  bool getPaused() const noexcept;

private:
  void initTargets();
  void initCaptureMatrices();
  void initSphereMap();
  void initCubeMap(const TriMesh &_cube, const MeshVBO &_vbo);
  void initIrradianceMap(const TriMesh &_cube, const MeshVBO &_vbo);
  void initPrefilteredMap(const TriMesh &_cube, const MeshVBO &_vbo);
  void initBrdfLUTMap(const TriMesh &_plane, const MeshVBO &_vbo);

  void generateCubeMap(const TriMesh &_cube,
      const MeshVBO &_vbo,
      std::unique_ptr<QOpenGLTexture> &_texture,
      const int _dim,
      const std::string &_matPath,
      const std::function<void (QOpenGLShaderProgram* io_prog)> &_prerender = [](QOpenGLShaderProgram*){}
  );

  void generate3DTexture(const TriMesh &_plane,
      const MeshVBO &_vbo,
      std::unique_ptr<QOpenGLTexture> &_texture, const int _dim,
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

  QOpenGLContext* m_context;
  float m_ao;
  float m_roughness;
  float m_metallic;
  float m_baseSpec;
  float m_normalStrength;

  QOpenGLBuffer m_morphTargetBuffer;

  GLuint m_morphTargetSSBO = 0;
  std::chrono::high_resolution_clock::time_point m_last;
  float m_time = 0.0f;
  bool m_paused = true;

};

#endif // MATERIALPBR_H
