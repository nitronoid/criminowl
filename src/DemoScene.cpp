#include "DemoScene.h"
#include "MaterialWireframe.h"
#include "MaterialPBR.h"
#include "MaterialPhong.h"
#include "MaterialFractal.h"
#include "MaterialEnvMap.h"
#include "MaterialBump.h"
#include <QOpenGLContext>
#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLFramebufferObject>

//-----------------------------------------------------------------------------------------------------
void DemoScene::writeMeshAttributes()
{
  using namespace MeshAttributes;
  for (const auto buff : {VERTEX, UV, NORMAL})
  {
    m_meshVBO.write(m_owlMesh.getAttribData(buff), buff);
  }
  m_meshVBO.setIndices(m_owlMesh.getIndicesData());
}
//-----------------------------------------------------------------------------------------------------
void DemoScene::setAttributeBuffers()
{
  static constexpr int tupleSize[] = {3,2,3};
  auto prog = m_shaderLib->getCurrentShader();

  using namespace MeshAttributes;
  for (const auto buff : {VERTEX, UV, NORMAL})
  {
    prog->enableAttributeArray(buff);
    prog->setAttributeBuffer(buff, GL_FLOAT, m_meshVBO.offset(buff), tupleSize[buff]);
  }
}
//-----------------------------------------------------------------------------------------------------
void DemoScene::init()
{
  Scene::init();

  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  initMaterials();

  initGeo();

  // Scope the using declaration
  {
    using namespace SceneMatrices;
    m_matrices[MODEL_VIEW] = glm::translate(m_matrices[MODEL_VIEW], glm::vec3(0.0f, 0.0f, -2.0f));
  }
}
//-----------------------------------------------------------------------------------------------------
void DemoScene::initGeo()
{
  m_owlMesh.load("models/owl.obj");
  // Create and bind our Vertex Array Object
  m_vao->create();
  m_vao->bind();
  // Create and bind our Vertex Buffer Object
  m_meshVBO.init();
  generateNewGeometry();
}
//-----------------------------------------------------------------------------------------------------
void DemoScene::keyPress(QKeyEvent* io_event)
{
  makeCurrent();
  Scene::keyPress(io_event);
  m_material->handleKey(io_event, context());
}
//-----------------------------------------------------------------------------------------------------
void DemoScene::initMaterials()
{
  m_material.reset(new MaterialPBR(m_camera, m_shaderLib, &m_matrices, context(), 1.0f, 0.2f, 0.0, 0.1f, 0.3f));

  auto name = m_shaderLib->loadShaderProg(m_material->shaderFileName());
  m_material->setShaderName(name);
  m_material->apply();
}
//-----------------------------------------------------------------------------------------------------
void DemoScene::metallicUpdate(const double _metallic)
{
  makeCurrent();
  m_material->setMetallic(static_cast<float>(_metallic));
}
//-----------------------------------------------------------------------------------------------------
void DemoScene::roughnessUpdate(const double _roughness)
{
  makeCurrent();
  m_material->setRoughness(static_cast<float>(_roughness));
}
//-----------------------------------------------------------------------------------------------------
void DemoScene::baseSpecUpdate(const double _baseSpec)
{
  makeCurrent();
  m_material->setBaseSpec(static_cast<float>(_baseSpec));
}
//-----------------------------------------------------------------------------------------------------
void DemoScene::normalStrengthUpdate(const double _normalStrength)
{
  makeCurrent();
  m_material->setNormalStrength(static_cast<float>(_normalStrength));
}
//-----------------------------------------------------------------------------------------------------
void DemoScene::setPaused(const bool _paused)
{
  makeCurrent();
  m_material->setPaused(_paused);
}
//-----------------------------------------------------------------------------------------------------
void DemoScene::generateNewGeometry()
{
  makeCurrent();
  m_meshVBO.reset(
        sizeof(GLushort),
        m_owlMesh.getNIndicesData(),
        sizeof(GLfloat),
        m_owlMesh.getNVertData(),
        m_owlMesh.getNUVData(),
        m_owlMesh.getNNormData()
        );
  writeMeshAttributes();
  setAttributeBuffers();
}
//-----------------------------------------------------------------------------------------------------
void DemoScene::renderScene()
{
  Scene::renderScene();

  // Scope the using declaration
  {
    using namespace SceneMatrices;
    m_matrices[MODEL_VIEW] = glm::rotate(m_matrices[MODEL_VIEW], glm::radians(-1.0f * m_rotating), glm::vec3(0.0f, 1.0f, 0.0f));
  }

  m_material->update();

  m_meshVBO.use();
  glDrawElements(GL_PATCHES, m_owlMesh.getNIndicesData(), GL_UNSIGNED_SHORT, nullptr);
}
//-----------------------------------------------------------------------------------------------------



