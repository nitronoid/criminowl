#include "MaterialBump.h"
#include "Scene.h"
#include "ShaderLib.h"

void MaterialBump::init()
{

  auto shaderPtr = m_shaderLib->getShader(m_shaderName);
  initColor();
  shaderPtr->setUniformValue("ColourTexture", 0);

  initNormal();
  shaderPtr->setUniformValue("NormalTexture", 1);

  update();
}

void MaterialBump::initColor()
{
  using tex = QOpenGLTexture;
  m_colorMap.reset(new QOpenGLTexture(QOpenGLTexture::Target2D));
  auto map = QImage("images/bricktexture.jpg").mirrored().convertToFormat(QImage::Format_RGB888);

  m_colorMap->create();
  m_colorMap->bind(0);
  m_colorMap->setSize(map.width(), map.height(), map.depth());
  m_colorMap->setFormat(tex::RGBFormat);
  m_colorMap->allocateStorage();
  m_colorMap->setData(tex::RGB, tex::UInt8, map.constBits());

  m_colorMap->setWrapMode(tex::Repeat);
  m_colorMap->setMinMagFilters(tex::Linear, tex::Linear);

}

void MaterialBump::initNormal()
{
  using tex = QOpenGLTexture;
  m_normalMap.reset(new QOpenGLTexture(QOpenGLTexture::Target2D));
  auto map = QImage("images/bricknormals.jpg").mirrored().convertToFormat(QImage::Format_RGB888);

  m_normalMap->create();
  m_normalMap->bind(1);
  m_normalMap->setSize(map.width(), map.height(), map.depth());
  m_normalMap->setFormat(tex::RGBFormat);
  m_normalMap->allocateStorage();
  m_normalMap->setData(tex::RGB, tex::UInt8, map.constBits());

  m_normalMap->setWrapMode(tex::Repeat);
  m_normalMap->setMinMagFilters(tex::Linear, tex::Linear);

}

void MaterialBump::update()
{

  m_colorMap->bind(0);
  m_normalMap->bind(1);

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

const char* MaterialBump::shaderFileName() const
{
  return "shaderPrograms/bump.json";
}
