#include "MaterialPBR.h"
#include "Scene.h"
#include "ShaderLib.h"
#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLFramebufferObject>
//#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

void MaterialPBR::init()
{
  auto shaderPtr = m_shaderLib->getShader(m_shaderName);
  initSphereMap();
  initEnvMap();
  shaderPtr->bind();

  shaderPtr->setUniformValue("irradianceMap", 0);
  shaderPtr->setPatchVertexCount(3);
  shaderPtr->setUniformValue("albedo", QVector3D{m_albedo.x, m_albedo.y, m_albedo.z});
  shaderPtr->setUniformValue("ao", m_ao);
  shaderPtr->setUniformValue("exposure", m_exposure);
  shaderPtr->setUniformValue("roughness", m_roughness);
  shaderPtr->setUniformValue("metallic", m_metallic);

  // Update our matrices
  update();
}

void MaterialPBR::update()
{

  m_irradianceMap->bind(0);
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


void MaterialPBR::initSphereMap()
{
  stbi_set_flip_vertically_on_load(true);
  int width, height, nrComponents;
  float *data = stbi_loadf("images/LA_Downtown_Afternoon_Fishing_3k.hdr", &width, &height, &nrComponents, 0);
  const float* cdata = data;
  using tex = QOpenGLTexture;
  m_sphereMap.reset(new QOpenGLTexture(QOpenGLTexture::Target2D));
  m_sphereMap->create();
  m_sphereMap->bind();
  m_sphereMap->setSize(width, height);
  m_sphereMap->setFormat(tex::RGB16F);
  m_sphereMap->allocateStorage();
  m_sphereMap->setData(tex::RGB, tex::Float32, cdata);
  m_sphereMap->setWrapMode(tex::ClampToEdge);
  m_sphereMap->setMinMagFilters(tex::Linear, tex::Linear);

  stbi_image_free(data);
}

void MaterialPBR::initEnvMap()
{
  auto defaultFBO = m_context->defaultFramebufferObject();
  auto cubeMapShaderName = m_shaderLib->loadShaderProg("shaderPrograms/hdr_cubemap.json");
  auto cubeMapShader = m_shaderLib->getShader(cubeMapShaderName);
  auto funcs = m_context->versionFunctions<QOpenGLFunctions_4_1_Core>();

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

  auto fbo = std::make_unique<QOpenGLFramebufferObject>(512, 512, QOpenGLFramebufferObject::Depth);

  using tex = QOpenGLTexture;
  m_cubeMap.reset(new QOpenGLTexture(QOpenGLTexture::TargetCubeMap));
  m_cubeMap->create();
  m_cubeMap->bind(0);
  m_cubeMap->setSize(512, 512);
  m_cubeMap->setFormat(tex::RGB16F);
  m_cubeMap->allocateStorage();
  m_cubeMap->setMinMagFilters(tex::Linear, tex::Linear);
  m_cubeMap->setWrapMode(tex::ClampToEdge);
  m_cubeMap->generateMipMaps();
  m_cubeMap->setAutoMipMapGenerationEnabled(true);

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
  m_sphereMap->bind(0);

  // don't forget to configure the viewport to the capture dimensions.
  funcs->glViewport(0, 0, 512, 512);
  fbo->bind();
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
    funcs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_cubeMap->textureId(), 0);
    funcs->glClearColor(0.f, 0.f, 0.f, 1.f);
    funcs->glClear(GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT);

    funcs->glDrawElements(GL_TRIANGLES, cube.getNIndicesData(), GL_UNSIGNED_SHORT, nullptr);
  }
  funcs->glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
  //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  m_irradianceMap.reset(new QOpenGLTexture(QOpenGLTexture::TargetCubeMap));
  m_irradianceMap->create();
  m_irradianceMap->bind(0);
  m_irradianceMap->setSize(32, 32);
  m_irradianceMap->setFormat(tex::RGB16F);
  m_irradianceMap->allocateStorage();
  m_irradianceMap->setMinMagFilters(tex::Linear, tex::Linear);
  m_irradianceMap->setWrapMode(tex::ClampToEdge);
  m_irradianceMap->generateMipMaps();
  m_irradianceMap->setAutoMipMapGenerationEnabled(true);

  fbo->release();
  fbo = std::make_unique<QOpenGLFramebufferObject>(32, 32, QOpenGLFramebufferObject::Depth);

  // pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
  // -----------------------------------------------------------------------------
  auto irradianceShaderName = m_shaderLib->loadShaderProg("shaderPrograms/hdr_cubemap_irradiance.json");
  auto irradianceShader = m_shaderLib->getShader(irradianceShaderName);
  irradianceShader->bind();
  irradianceShader->setUniformValue("envMap", 0);
  irradianceShader->setUniformValue("projection", captureProjection);
  m_cubeMap->bind(0);

  funcs->glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
//  funcs->glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  fbo->bind();
  for (unsigned int i = 0; i < 6; ++i)
  {
    // Convert from glm to Qt
    QMatrix4x4 view(glm::value_ptr(captureViews[i]));
    // Need to transpose the matrix as they both use different majors
    irradianceShader->setUniformValue("view", view.transposed());
    funcs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_irradianceMap->textureId(), 0);
    funcs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    funcs->glDrawElements(GL_TRIANGLES, cube.getNIndicesData(), GL_UNSIGNED_SHORT, nullptr);
  }

  funcs->glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
  fbo->release();
}


