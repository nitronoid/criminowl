#include "MaterialPBR.h"
#include "Scene.h"
#include "ShaderLib.h"
#include <QOpenGLFunctions_4_1_Core>

void MaterialPBR::init()
{
  auto shaderPtr = m_shaderLib->getShader(m_shaderName);
  //  m_envMap.init(m_shaderLib, m_context);
//  initEnvMap();
//  initSphereMap();
//  initCubeMap();

  shaderPtr->setUniformValue("irradianceMap", 0);
//  shaderPtr->setUniformValue("sphereMap", 1);
  shaderPtr->setPatchVertexCount(3);
  shaderPtr->setUniformValue("albedo", QVector3D{m_albedo.x, m_albedo.y, m_albedo.z});
  shaderPtr->setUniformValue("ao", m_ao);
  shaderPtr->setUniformValue("exposure", m_exposure);
  shaderPtr->setUniformValue("roughness", m_roughness);
  shaderPtr->setUniformValue("metallic", m_metallic);

  // Update our matrices
  update();
}



void MaterialPBR::initEnvMap()
{
  m_envMap.reset(new QOpenGLTexture(QOpenGLTexture::TargetCubeMap));
  static constexpr std::array<const char*, 6> paths = {{
                                                         "images/sky_xpos.png",
                                                         "images/sky_ypos.png",
                                                         "images/sky_zpos.png",
                                                         "images/sky_xneg.png",
                                                         "images/sky_yneg.png",
                                                         "images/sky_zneg.png"
                                                       }};

  using tex = QOpenGLTexture;
  static constexpr std::array<tex::CubeMapFace,6> dataTypes = {{
                                                                 tex::CubeMapPositiveX,
                                                                 tex::CubeMapPositiveY,
                                                                 tex::CubeMapPositiveZ,
                                                                 tex::CubeMapNegativeX,
                                                                 tex::CubeMapNegativeY,
                                                                 tex::CubeMapNegativeZ
                                                               }};
  std::array<QImage, 6> maps;
  for (size_t i = 0; i < maps.size(); ++i)
    maps[i] = QImage(paths[i]).mirrored().convertToFormat(QImage::Format_RGBA8888);

  m_envMap->create();
  m_envMap->bind(0);
  m_envMap->setSize(maps[0].width(), maps[0].height(), maps[0].depth());
  m_envMap->setFormat(tex::RGBAFormat);
  m_envMap->allocateStorage();

  for (size_t i = 0; i < maps.size(); ++i)
    m_envMap->setData(0, 0, dataTypes[i], tex::RGBA, tex::UInt8, maps[i].constBits());

  m_envMap->setMinMagFilters(tex::LinearMipMapLinear, tex::Linear);
  m_envMap->setWrapMode(tex::ClampToEdge);
  m_envMap->generateMipMaps();
  m_envMap->setAutoMipMapGenerationEnabled(true);
}


void MaterialPBR::update()
{
//  m_sphereMap->bind(1);
//  m_envMap->bind(0);
  auto shaderPtr = m_shaderLib->getShader(m_shaderName);
  auto eye = m_cam->getCameraEye();
  shaderPtr->setUniformValue("camPos", QVector3D{eye.x, eye.y, eye.z});

  // Scope the using declaration
  {
    using namespace SceneMatrices;
    static constexpr std::array<const char*, 3> shaderUniforms = {{"M", "MVP", "N"}};
    // Send all our matrices to the GPU
    for (const auto matrixId : {MODEL_VIEW, PROJECTION, NORMAL})
    {
      // Convert from glm to Qt
      QMatrix4x4 qmat(glm::value_ptr((*m_matrices)[matrixId]));
      // Need to transpose the matrix as they both use different majors
      shaderPtr->setUniformValue(shaderUniforms[matrixId], qmat.transposed());
    }
  }
}

const char* MaterialPBR::shaderFileName() const
{
  return "shaderPrograms/owl_pbr.json";
}


