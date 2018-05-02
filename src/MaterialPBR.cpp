#include "MaterialPBR.h"
#include "Scene.h"
#include "ShaderLib.h"
#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLFramebufferObject>
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

void MaterialPBR::init()
{
  auto shaderPtr = m_shaderLib->getShader(m_shaderName);

  QOpenGLVertexArrayObject vao;
  // Create and bind our Vertex Array Object
  vao.create();
  vao.bind();

  Mesh cube;
  cube.load("models/unitCube.obj");
  Mesh plane;
  plane.load("models/unitPlane.obj");

  MeshVBO vbo;
  // Create and bind our Vertex Buffer Object
  vbo.init();
  vbo.reset(sizeof(GLushort), cube.getNIndicesData(), sizeof(GLfloat), cube.getNVertData(), cube.getNUVData(), cube.getNNormData());
  {
    using namespace MeshAttributes;
    vbo.write(cube.getVertexData(), VERTEX);
    vbo.setIndices(cube.getIndicesData());
  }

  initCaptureMatrices();
  initSphereMap();
  initCubeMap(cube, vbo);
  initIrradianceMap(cube, vbo);
  initPrefilteredMap(cube, vbo);

  vbo.reset(sizeof(GLushort), plane.getNIndicesData(), sizeof(GLfloat), plane.getNVertData(), plane.getNUVData(), plane.getNNormData());
  {
    using namespace MeshAttributes;
    vbo.write(plane.getVertexData(), VERTEX);
    vbo.write(plane.getUVsData(), UV);
    vbo.setIndices(plane.getIndicesData());
  }
  initBrdfLUTMap(plane, vbo);
  initNoiseMap(plane, vbo);
  initNormalMap(plane, vbo);

  shaderPtr->bind();
  shaderPtr->setPatchVertexCount(3);

  shaderPtr->setUniformValue("irradianceMap", 0);
  shaderPtr->setUniformValue("prefilterMap", 1);
  shaderPtr->setUniformValue("brdfLUT", 2);
  shaderPtr->setUniformValue("surfaceMap", 3);
  shaderPtr->setUniformValue("normalMap", 4);
  shaderPtr->setUniformValue("albedo", QVector3D{m_albedo.x, m_albedo.y, m_albedo.z});
  shaderPtr->setUniformValue("ao", m_ao);
  shaderPtr->setUniformValue("roughness", m_roughness);
  shaderPtr->setUniformValue("metallic", m_metallic);

//  auto funcs = m_context->versionFunctions<QOpenGLFunctions_4_1_Core>();
//  GLuint rout = funcs->glGetSubroutineIndex(shaderPtr->programId(), GL_FRAGMENT_SHADER, "pnoise_calc");
//  funcs->glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, GL_FRAGMENT_SHADER, &rout);

  // Update our matrices
  update();
}

void MaterialPBR::update()
{

  m_irradianceMap->bind(0);
  m_prefilteredMap->bind(1);
  m_brdfLUTMap->bind(2);
  m_noiseMap->bind(3);
  m_normalMap->bind(4);

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

void MaterialPBR::initCaptureMatrices()
{
  glm::mat4 captureProjectionGLM = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
  m_captureProjection = QMatrix4x4(glm::value_ptr(captureProjectionGLM)).transposed();
  glm::mat4 captureViews[] =
  {
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
  };
  for (unsigned int i = 0; i < 6; ++i)
  {
    // Convert from glm to Qt
    QMatrix4x4 view(glm::value_ptr(captureViews[i]));
    m_captureViews[i] = view.transposed();
  }
}

void MaterialPBR::initSphereMap()
{
  stbi_set_flip_vertically_on_load(true);
  int width, height, nrComponents;
  float *data = stbi_loadf("images/Alexs_Apt_2k.hdr", &width, &height, &nrComponents, 0);
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

void MaterialPBR::initCubeMap(const Mesh &_cube, const MeshVBO &_vbo)
{
  using tex = QOpenGLTexture;
  auto defaultFBO = m_context->defaultFramebufferObject();
  auto cubeMapShaderName = m_shaderLib->loadShaderProg("shaderPrograms/hdr_cubemap.json");
  auto cubeMapShader = m_shaderLib->getShader(cubeMapShaderName);
  auto funcs = m_context->versionFunctions<QOpenGLFunctions_4_1_Core>();

  auto fbo = std::make_unique<QOpenGLFramebufferObject>(512, 512, QOpenGLFramebufferObject::Depth);

  m_cubeMap.reset(new QOpenGLTexture(QOpenGLTexture::TargetCubeMap));
  m_cubeMap->create();
  m_cubeMap->bind(0);
  m_cubeMap->setSize(512, 512);
  m_cubeMap->setFormat(tex::RGB16F);
  m_cubeMap->allocateStorage();
  m_cubeMap->setMinMagFilters(tex::Linear, tex::Linear);
  m_cubeMap->setWrapMode(tex::ClampToEdge);

  // convert HDR equirectangular environment map to cubemap equivalent
  cubeMapShader->bind();
  cubeMapShader->setUniformValue("sphereMap", 0);
  // Need to transpose the matrix as they both use different majors
  cubeMapShader->setUniformValue("projection", m_captureProjection);
  m_sphereMap->bind(0);

  // don't forget to configure the viewport to the capture dimensions.
  funcs->glViewport(0, 0, 512, 512);
  fbo->bind();
  {
    using namespace MeshAttributes;
    cubeMapShader->enableAttributeArray(VERTEX);
    cubeMapShader->setAttributeBuffer(VERTEX, GL_FLOAT, _vbo.offset(VERTEX), 3);
  }

  for (unsigned int i = 0; i < 6; ++i)
  {
    // Need to transpose the matrix as they both use different majors
    cubeMapShader->setUniformValue("view", m_captureViews[i]);
    funcs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_cubeMap->textureId(), 0);
    funcs->glClearColor(0.f, 0.f, 0.f, 1.f);
    funcs->glClear(GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT);

    funcs->glDrawElements(GL_TRIANGLES, _cube.getNIndicesData(), GL_UNSIGNED_SHORT, nullptr);
  }
  funcs->glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
  fbo->release();
}

void MaterialPBR::initIrradianceMap(const Mesh &_cube, const MeshVBO &_vbo)
{
  using tex = QOpenGLTexture;
  auto defaultFBO = m_context->defaultFramebufferObject();
  auto funcs = m_context->versionFunctions<QOpenGLFunctions_4_1_Core>();

  m_irradianceMap.reset(new QOpenGLTexture(QOpenGLTexture::TargetCubeMap));
  m_irradianceMap->create();
  m_irradianceMap->bind(0);
  m_irradianceMap->setSize(32, 32);
  m_irradianceMap->setFormat(tex::RGB16F);
  m_irradianceMap->allocateStorage();
  m_irradianceMap->setMinMagFilters(tex::Linear, tex::Linear);
  m_irradianceMap->setWrapMode(tex::ClampToEdge);

  auto fbo = std::make_unique<QOpenGLFramebufferObject>(32, 32, QOpenGLFramebufferObject::Depth);

  // pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
  // -----------------------------------------------------------------------------
  auto irradianceShaderName = m_shaderLib->loadShaderProg("shaderPrograms/hdr_cubemap_irradiance.json");
  auto irradianceShader = m_shaderLib->getShader(irradianceShaderName);
  irradianceShader->bind();
  irradianceShader->setUniformValue("envMap", 0);
  irradianceShader->setUniformValue("projection", m_captureProjection);
  m_cubeMap->bind(0);
  // don't forget to configure the viewport to the capture dimensions.
  funcs->glViewport(0, 0, 32, 32);
  fbo->bind();
  {
    using namespace MeshAttributes;
    irradianceShader->enableAttributeArray(VERTEX);
    irradianceShader->setAttributeBuffer(VERTEX, GL_FLOAT, _vbo.offset(VERTEX), 3);
  }
  for (unsigned int i = 0; i < 6; ++i)
  {
    irradianceShader->setUniformValue("view", m_captureViews[i]);
    funcs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_irradianceMap->textureId(), 0);
    funcs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    funcs->glDrawElements(GL_TRIANGLES, _cube.getNIndicesData(), GL_UNSIGNED_SHORT, nullptr);
  }

  funcs->glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
  fbo->release();
}

void MaterialPBR::initPrefilteredMap(const Mesh &_cube, const MeshVBO &_vbo)
{
  using tex = QOpenGLTexture;
  auto defaultFBO = m_context->defaultFramebufferObject();
  auto funcs = m_context->versionFunctions<QOpenGLFunctions_4_1_Core>();

  m_prefilteredMap.reset(new QOpenGLTexture(QOpenGLTexture::TargetCubeMap));
  m_prefilteredMap->create();
  m_prefilteredMap->bind(0);
  m_prefilteredMap->setSize(128, 128);
  m_prefilteredMap->setFormat(tex::RGB16F);
  m_prefilteredMap->setMinMagFilters(tex::LinearMipMapLinear, tex::Linear);
  m_prefilteredMap->setWrapMode(tex::ClampToEdge);
  m_prefilteredMap->setMipLevels(4);
  m_prefilteredMap->setMipMaxLevel(4);
  m_prefilteredMap->allocateStorage();
  m_prefilteredMap->generateMipMaps();
  //  m_prefilteredMap->setAutoMipMapGenerationEnabled(true);

  auto prefilterShaderName = m_shaderLib->loadShaderProg("shaderPrograms/hdr_cubemap_prefilter.json");
  auto prefilterShader = m_shaderLib->getShader(prefilterShaderName);
  prefilterShader->bind();
  prefilterShader->setUniformValue("envMap", 0);
  prefilterShader->setUniformValue("projection", m_captureProjection);

  m_cubeMap->bind(0);
  {
    using namespace MeshAttributes;
    prefilterShader->enableAttributeArray(VERTEX);
    prefilterShader->setAttributeBuffer(VERTEX, GL_FLOAT, _vbo.offset(VERTEX), 3);
  }
  int maxMipLevels = 5;
  for (int mip = 0; mip < maxMipLevels; ++mip)
  {
    // reisze framebuffer according to mip-level size.
    auto mipWidth  = static_cast<int>(128 * std::pow(0.5f, mip));
    auto mipHeight = static_cast<int>(128 * std::pow(0.5f, mip));
    auto fbo = std::make_unique<QOpenGLFramebufferObject>(mipWidth, mipHeight, QOpenGLFramebufferObject::Depth);
    fbo->bind();
    //    funcs->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
    funcs->glViewport(0, 0, mipWidth, mipHeight);

    float roughness = static_cast<float>(mip) / static_cast<float>(maxMipLevels - 1);
    prefilterShader->setUniformValue("roughness", roughness);
    for (unsigned int i = 0; i < 6; ++i)
    {
      prefilterShader->setUniformValue("view", m_captureViews[i]);
      funcs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_prefilteredMap->textureId(), mip);
      funcs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      funcs->glDrawElements(GL_TRIANGLES, _cube.getNIndicesData(), GL_UNSIGNED_SHORT, nullptr);
    }
    funcs->glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
    fbo->release();
  }
}

void MaterialPBR::initBrdfLUTMap(const Mesh &_plane, const MeshVBO &_vbo)
{
  using tex = QOpenGLTexture;
  auto defaultFBO = m_context->defaultFramebufferObject();
  auto funcs = m_context->versionFunctions<QOpenGLFunctions_4_1_Core>();

  m_brdfLUTMap.reset(new QOpenGLTexture(QOpenGLTexture::Target2D));
  m_brdfLUTMap->create();
  m_brdfLUTMap->bind();
  m_brdfLUTMap->setSize(512, 512);
  m_brdfLUTMap->setFormat(tex::RGB16F);
  m_brdfLUTMap->setMinMagFilters(tex::Linear, tex::Linear);
  m_brdfLUTMap->setWrapMode(tex::ClampToEdge);
  m_brdfLUTMap->allocateStorage();

  auto brdfLUTShaderName = m_shaderLib->loadShaderProg("shaderPrograms/hdr_cubemap_brdf.json");
  auto brdfLUTShader = m_shaderLib->getShader(brdfLUTShaderName);

  auto fbo = std::make_unique<QOpenGLFramebufferObject>(512, 512, QOpenGLFramebufferObject::Depth);
  funcs->glViewport(0, 0, 512, 512);
  brdfLUTShader->bind();
  {
    using namespace MeshAttributes;
    brdfLUTShader->enableAttributeArray(VERTEX);
    brdfLUTShader->setAttributeBuffer(VERTEX, GL_FLOAT, _vbo.offset(VERTEX), 3);
    brdfLUTShader->enableAttributeArray(UV);
    brdfLUTShader->setAttributeBuffer(UV, GL_FLOAT, _vbo.offset(UV), 2);
  }

  funcs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_brdfLUTMap->textureId(), 0);
  funcs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  funcs->glDrawElements(GL_TRIANGLES, _plane.getNIndicesData(), GL_UNSIGNED_SHORT, nullptr);

  funcs->glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
  fbo->release();
}


void MaterialPBR::initNoiseMap(const Mesh &_plane, const MeshVBO &_vbo)
{
  static constexpr auto RES = 1024;
  using tex = QOpenGLTexture;
  auto defaultFBO = m_context->defaultFramebufferObject();
  auto funcs = m_context->versionFunctions<QOpenGLFunctions_4_1_Core>();

  m_noiseMap.reset(new QOpenGLTexture(QOpenGLTexture::Target3D));
  m_noiseMap->create();
  m_noiseMap->bind();
  m_noiseMap->setSize(RES, RES, RES);
  m_noiseMap->setFormat(tex::RGBA16F);
  m_noiseMap->setMinMagFilters(tex::Linear, tex::Linear);
  m_noiseMap->setWrapMode(tex::Repeat);
  m_noiseMap->allocateStorage();

  auto noiseShaderName = m_shaderLib->loadShaderProg("shaderPrograms/owl_noise.json");
  auto noiseShader = m_shaderLib->getShader(noiseShaderName);

  auto fbo = std::make_unique<QOpenGLFramebufferObject>(RES, RES, QOpenGLFramebufferObject::Depth, GL_TEXTURE_3D);
  funcs->glViewport(0, 0, RES, RES);
  noiseShader->bind();
  {
    using namespace MeshAttributes;
    noiseShader->enableAttributeArray(VERTEX);
    noiseShader->setAttributeBuffer(VERTEX, GL_FLOAT, _vbo.offset(VERTEX), 3);
    noiseShader->enableAttributeArray(UV);
    noiseShader->setAttributeBuffer(UV, GL_FLOAT, _vbo.offset(UV), 2);
  }

  static constexpr auto denom = 1.f / static_cast<float>(RES);
  for (int i = 0; i < RES; ++i)
  {
    noiseShader->setUniformValue("Zdepth", i * denom);
    funcs->glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, m_noiseMap->textureId(), 0, i);
    funcs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    funcs->glDrawElements(GL_TRIANGLES, _plane.getNIndicesData(), GL_UNSIGNED_SHORT, nullptr);
  }

  funcs->glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
  fbo->release();
}


void MaterialPBR::initNormalMap(const Mesh &_plane, const MeshVBO &_vbo)
{
  static constexpr auto RES = 1024;
  using tex = QOpenGLTexture;
  auto defaultFBO = m_context->defaultFramebufferObject();
  auto funcs = m_context->versionFunctions<QOpenGLFunctions_4_1_Core>();

  m_normalMap.reset(new QOpenGLTexture(QOpenGLTexture::Target3D));
  m_normalMap->create();
  m_normalMap->bind();
  m_normalMap->setSize(RES, RES, RES);
  m_normalMap->setFormat(tex::RGB16F);
  m_normalMap->setMinMagFilters(tex::Linear, tex::Linear);
  m_normalMap->setWrapMode(tex::Repeat);
  m_normalMap->allocateStorage();

  auto normalShaderName = m_shaderLib->loadShaderProg("shaderPrograms/owl_normal.json");
  auto normalShader = m_shaderLib->getShader(normalShaderName);

  auto fbo = std::make_unique<QOpenGLFramebufferObject>(RES, RES, QOpenGLFramebufferObject::Depth, GL_TEXTURE_3D);
  funcs->glViewport(0, 0, RES, RES);
  normalShader->bind();
  {
    using namespace MeshAttributes;
    normalShader->enableAttributeArray(VERTEX);
    normalShader->setAttributeBuffer(VERTEX, GL_FLOAT, _vbo.offset(VERTEX), 3);
    normalShader->enableAttributeArray(UV);
    normalShader->setAttributeBuffer(UV, GL_FLOAT, _vbo.offset(UV), 2);
  }
  normalShader->setUniformValue("bump", 0);
  m_noiseMap->bind(0);

  static constexpr auto denom = 1.f / static_cast<float>(RES);
  for (int i = 0; i < RES; ++i)
  {
    normalShader->setUniformValue("Zdepth", i * denom);
    funcs->glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, m_normalMap->textureId(), 0, i);
    funcs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    funcs->glDrawElements(GL_TRIANGLES, _plane.getNIndicesData(), GL_UNSIGNED_SHORT, nullptr);
  }

  funcs->glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
  fbo->release();
}
