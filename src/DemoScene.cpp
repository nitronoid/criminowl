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
#include "HDR_cube.h"
//#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

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

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

//  HDR_cube map;
//  map.init(m_shaderLib, context());
  initSphereMap();
  initCubeMap();
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
  m_material.reset(new MaterialPBR(m_camera, m_shaderLib, &m_matrices, context(), {0.5f, 0.5f, 0.5f}, 1.0f, 1.0f, 0.5f, 1.0f));

  auto name = m_shaderLib->loadShaderProg(m_material->shaderFileName());
  m_material->setShaderName(name);
  m_material->apply();

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
void DemoScene::renderScene()
{
  Scene::renderScene();

  // Scope the using declaration
  {
    using namespace SceneMatrices;
    m_matrices[MODEL_VIEW] = glm::rotate(m_matrices[MODEL_VIEW], glm::radians(-1.0f * m_rotating), glm::vec3(0.0f, 1.0f, 0.0f));
  }


  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceTextureID);

  m_material->update();


  m_meshVBO.use();
  glDrawElements(GL_PATCHES, m_owlMesh.getNIndicesData(), GL_UNSIGNED_SHORT, nullptr);
}
//-----------------------------------------------------------------------------------------------------



void DemoScene::initSphereMap()
{
  stbi_set_flip_vertically_on_load(true);
  int width, height, nrComponents;
  float *data = stbi_loadf("images/Alexs_Apt_2k.hdr", &width, &height, &nrComponents, 0);

  auto funcs = context()->versionFunctions<QOpenGLFunctions_4_1_Core>();
  funcs->glGenTextures(1, &m_hdrTextureID);
  funcs->glBindTexture(GL_TEXTURE_2D, m_hdrTextureID);
  funcs->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
  funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  stbi_image_free(data);
}

void DemoScene::initCubeMap()
{
  auto defaultFBO = defaultFramebufferObject();
  auto cubeMapShaderName = m_shaderLib->loadShaderProg("shaderPrograms/hdr_cubemap.json");
  auto cubeMapShader = m_shaderLib->getShader(cubeMapShaderName);
  auto funcs = context()->versionFunctions<QOpenGLFunctions_4_1_Core>();

  QOpenGLVertexArrayObject vao;
  // Create and bind our Vertex Array Object
  vao.create();
  vao.bind();

  Mesh cube;
  cube.load("models/unitCube.obj");

  MeshVBO vbo;
  // Create and bind our Vertex Buffer Object
  vbo.init();
  vbo.reset(
        sizeof(GLushort),
        cube.getNIndicesData(),
        sizeof(GLfloat),
        cube.getNVertData(),
        cube.getNUVData(),
        cube.getNNormData()
        );


  unsigned int captureFBO, captureRBO;

  funcs->glGenFramebuffers(1, &captureFBO);
  funcs->glGenRenderbuffers(1, &captureRBO);

  funcs->glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  funcs->glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  funcs->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
  funcs->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

  funcs->glGenTextures(1, &m_envTextureID);
  funcs->glBindTexture(GL_TEXTURE_CUBE_MAP, m_envTextureID);
  for (unsigned int i = 0; i < 6; ++i)
  {
    // note that we store each face with 16 bit floating point values
    funcs->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
  }
  funcs->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  funcs->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  funcs->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  funcs->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  funcs->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glm::mat4 captureProjectionGLM = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
  QMatrix4x4 captureProjection(glm::value_ptr(captureProjectionGLM));
  captureProjection = captureProjection.transposed();
  glm::mat4 captureViews[] =
  {
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
  };

  // convert HDR equirectangular environment map to cubemap equivalent
  cubeMapShader->bind();
  cubeMapShader->setUniformValue("sphereMap", 0);
  // Need to transpose the matrix as they both use different majors
  cubeMapShader->setUniformValue("projection", captureProjection);
  funcs->glActiveTexture(GL_TEXTURE0);
  funcs->glBindTexture(GL_TEXTURE_2D, m_hdrTextureID);

  funcs->glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
  funcs->glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

  {
    using namespace MeshAttributes;
    vbo.write(cube.getVertexData(), VERTEX);
    vbo.setIndices(cube.getIndicesData());
    cubeMapShader->enableAttributeArray(VERTEX);
    cubeMapShader->setAttributeBuffer(VERTEX, GL_FLOAT, vbo.offset(VERTEX), 3);
  }

  for (unsigned int i = 0; i < 6; ++i)
  {
    // Convert from glm to Qt
    QMatrix4x4 view(glm::value_ptr(captureViews[i]));
    // Need to transpose the matrix as they both use different majors
    cubeMapShader->setUniformValue("view", view.transposed());
    funcs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_envTextureID, 0);
    funcs->glClearColor(0.f, 0.f, 0.f, 1.f);
    funcs->glClear(GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT);

    funcs->glDrawElements(GL_TRIANGLES, cube.getNIndicesData(), GL_UNSIGNED_SHORT, nullptr);
  }
  funcs->glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
  //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  funcs->glGenTextures(1, &m_irradianceTextureID);
  funcs->glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceTextureID);
  for (unsigned int i = 0; i < 6; ++i)
  {
    funcs->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
  }
  funcs->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  funcs->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  funcs->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  funcs->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  funcs->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  funcs->glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  funcs->glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  funcs->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

  // pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
  // -----------------------------------------------------------------------------
  auto irradianceShaderName = m_shaderLib->loadShaderProg("shaderPrograms/hdr_cubemap_irradiance.json");
  auto irradianceShader = m_shaderLib->getShader(irradianceShaderName);
  irradianceShader->bind();
  irradianceShader->setUniformValue("envMap", 0);
  irradianceShader->setUniformValue("projection", captureProjection);
  funcs->glActiveTexture(GL_TEXTURE0);
  funcs->glBindTexture(GL_TEXTURE_CUBE_MAP, m_envTextureID);

  funcs->glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
  funcs->glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  for (unsigned int i = 0; i < 6; ++i)
  {
    // Convert from glm to Qt
    QMatrix4x4 view(glm::value_ptr(captureViews[i]));
    // Need to transpose the matrix as they both use different majors
    irradianceShader->setUniformValue("view", view.transposed());
    funcs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_irradianceTextureID, 0);
    funcs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    funcs->glDrawElements(GL_TRIANGLES, cube.getNIndicesData(), GL_UNSIGNED_SHORT, nullptr);
  }

  funcs->glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
}
