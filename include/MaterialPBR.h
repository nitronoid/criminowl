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
      const float _normalStrength,
      const unsigned _morphTargetCount = 0,
      const unsigned _morphTargetFPS = 0
      ) :
    Material(io_camera, io_shaderLib, io_matrices),
    m_context(io_context),
    m_ao(_ao),
    m_roughness(_roughness),
    m_metallic(_metallic),
    m_baseSpec(_baseSpec),
    m_normalStrength(_normalStrength),
    m_morphTargetCount(_morphTargetCount),
    m_morphTargetFPS(_morphTargetFPS)
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

  void setAO(const float _ao) noexcept;
  float getAO() const noexcept;

  void setRoughness(const float _roughness) noexcept;
  float getRoughness() const noexcept;

  void setBaseSpec(const float _baseSpec) noexcept;
  float getBaseSpec() const noexcept;

  void setNormalStrength(const float _normalStrength) noexcept;
  float getNormalStrength() const noexcept;

  void setPaused(const bool _paused) noexcept;
  bool getPaused() const noexcept;

  void setTessType(const int _tessType) noexcept;
  int getTessType() const noexcept;

  void setTessLevelInner(const int _tessLevel) noexcept;
  int getTessLevelInner() const noexcept;

  void setTessLevelOuter(const int _tessLevel) noexcept;
  int getTessLevelOuter() const noexcept;

  void  setEyeDisp(const float _eyeDisp) noexcept;
  float getEyeDisp() const noexcept;

  void  setEyeScale(const float _eyeScale) noexcept;
  float getEyeScale() const noexcept;

  void  setEyeTranslate(const glm::vec3 _eyeTranslate) noexcept;
  glm::vec3 getEyeTranslate() const noexcept;

  void  setEyeRotation(const float _eyeRotation) noexcept;
  float getEyeRotation() const noexcept;

  void  setEyeWarp(const float _eyeWarp) noexcept;
  float getEyeWarp() const noexcept;

  void  setEyeExponent(const float _eyeExp) noexcept;
  float getEyeExponent() const noexcept;

  void  setEyeThickness(const float _eyeThickness) noexcept;
  float getEyeThickness() const noexcept;

  void  setEyeGap(const float _eyeGap) noexcept;
  float getEyeGap() const noexcept;

  void  setEyeFuzz(const float _eyeFuzz) noexcept;
  float getEyeFuzz() const noexcept;

  void  setEyeMaskCap(const float _eyeMaskCap) noexcept;
  float getEyeMaskCap() const noexcept;

  void  setTessMaskCap(const float _tessMaskCap) noexcept;
  float getTessMaskCap() const noexcept;

  void  setPhongStrength(const float _strength) noexcept;
  float getPhongStrength() const noexcept;

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

  void generate3DTexture(
      const TriMesh &_plane,
      const MeshVBO &_vbo,
      std::unique_ptr<QOpenGLTexture> &_texture, const int _dim,
      const std::string &_matPath,
      const QOpenGLTexture::TextureFormat _format,
      const std::function<void (QOpenGLShaderProgram* io_prog)> &_prerender = [](QOpenGLShaderProgram*){}
  );

  std::array<QMatrix4x4, 6>  m_captureViews;
  std::array<QVector4D, 9> m_colours = {
    {
      {0.093f,  0.02f, 0.003f, 0.0f},
      {0.036f, 0.008f, 0.001f, 0.0f},
      { 0.03f, 0.009f,   0.0f, 0.0f},
      { 0.08f, 0.002f,   0.0f, 0.0f},
      {0.703f, 0.188f, 0.108f, 0.0f},
      {0.707f, 0.090f, 0.021f, 0.0f},
      {0.960f, 0.436f, 0.149f, 0.0f},
      {0.843f, 0.326f, 0.176f, 0.0f},
      {  1.0f,  0.31f, 0.171f, 0.0f}
    }
  };
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
  GLuint m_tessType = 1;
  int m_tessLevelInner  = 15;
  int m_tessLevelOuter  = 15;
  float m_tessMaskCap   = 1.f;
  float m_phongStrength = 0.55f;

  glm::vec3 m_eyeTranslate {0.21, 0.3, 0.0};
  float m_eyeDisp      = -0.2f;
  float m_eyeScale     = 1.55f;
  float m_eyeRotation  =  7.0f;
  float m_eyeWarp      =  1.0f;
  float m_eyeExponent  =  3.0f;
  float m_eyeThickness = 0.08f;
  float m_eyeGap       = 0.19f;
  float m_eyeFuzz      = 0.02f;
  float m_eyeMaskCap   =  0.7f;

  unsigned m_morphTargetCount = 0;
  unsigned m_morphTargetFPS = 0;

};

#endif // MATERIALPBR_H
