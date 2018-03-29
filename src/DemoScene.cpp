#include "DemoScene.h"
#include "MaterialWireframe.h"
#include "MaterialPBR.h"
#include "MaterialPhong.h"
#include "MaterialFractal.h"
#include "MaterialEnvMap.h"
#include "MaterialBump.h"
#include <QOpenGLContext>
#include <QOpenGLFunctions_4_1_Core>

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

  context()->versionFunctions<QOpenGLFunctions_4_1_Core>()->glPatchParameteri(GL_PATCH_VERTICES, 3);
}
//-----------------------------------------------------------------------------------------------------
void DemoScene::keyPress(QKeyEvent* io_event)
{
  makeCurrent();
  Scene::keyPress(io_event);
  m_materials[m_currentMaterial]->handleKey(io_event, context());
}
//-----------------------------------------------------------------------------------------------------
void DemoScene::initMaterials()
{
  m_materials.reserve(7);

  m_materials.emplace_back(new MaterialPBR(m_camera, m_shaderLib, &m_matrices, {0.5f, 0.0f, 0.0f}, 1.0f, 1.0f, 0.5f, 1.0f));
  m_materials.emplace_back(new MaterialPBR(m_camera, m_shaderLib, &m_matrices, {0.1f, 0.2f, 0.5f}, 0.5f, 1.0f, 0.4f, 0.2f));
  m_materials.emplace_back(new MaterialWireframe(m_camera, m_shaderLib, &m_matrices));

  for (auto& mat : m_materials)
  {
    auto name = m_shaderLib->loadShaderProg(mat->shaderFileName());
    mat->setShaderName(name);
    mat->apply();
  }

  m_materials[m_currentMaterial]->apply();
}
//-----------------------------------------------------------------------------------------------------
void DemoScene::rotating( const bool _rotating )
{
  m_rotating = _rotating;
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
void DemoScene::nextMaterial()
{
  makeCurrent();
  m_currentMaterial = (m_currentMaterial + 1) % m_materials.size();

  m_materials[m_currentMaterial]->apply();
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

  m_materials[m_currentMaterial]->update();

  m_meshVBO.use();
  glDrawElements(GL_PATCHES, m_owlMesh.getNIndicesData(), GL_UNSIGNED_SHORT, nullptr);
}
//-----------------------------------------------------------------------------------------------------
