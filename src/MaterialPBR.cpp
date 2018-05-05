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
  // Generate cube map from sphere map
  generateCubeMap(cube, vbo, m_cubeMap, 512, "shaderPrograms/hdr_cubemap.json", [&sphereMap = m_sphereMap, &projection = m_captureProjection](auto shader)
  {
    // convert HDR equirectangular environment map to cubemap equivalent
    shader->bind();
    shader->setUniformValue("u_sphereMap", 0);
    // Need to transpose the matrix as they both use different majors
    shader->setUniformValue("u_P", projection);
    sphereMap->bind(0);
  });
  // Generate irradiance map
  generateCubeMap(cube, vbo, m_irradianceMap, 32, "shaderPrograms/hdr_cubemap_irradiance.json", [&cubeMap = m_cubeMap, &projection = m_captureProjection](auto shader)
  {
    // convert HDR equirectangular environment map to cubemap equivalent
    shader->bind();
    shader->setUniformValue("u_envMap", 0);
    // Need to transpose the matrix as they both use different majors
    shader->setUniformValue("u_P", projection);
    cubeMap->bind(0);
  });
  initPrefilteredMap(cube, vbo);

  vbo.reset(sizeof(GLushort), plane.getNIndicesData(), sizeof(GLfloat), plane.getNVertData(), plane.getNUVData(), plane.getNNormData());
  {
    using namespace MeshAttributes;
    vbo.write(plane.getVertexData(), VERTEX);
    vbo.write(plane.getUVsData(), UV);
    vbo.setIndices(plane.getIndicesData());
  }
  initBrdfLUTMap(plane, vbo);
  // Generate the albedo map
  generate3DTexture(plane, vbo, m_albedoMap, "shaderPrograms/owl_noise.json");
  // Generate the normal map
  generate3DTexture(plane, vbo, m_normalMap, "shaderPrograms/owl_normal.json",
                    [&bumpMap = m_albedoMap](auto shader)
  {
    shader->setUniformValue("u_bumpMap", 0);
    bumpMap->bind(0);
  });

  shaderPtr->bind();
  shaderPtr->setPatchVertexCount(3);

  shaderPtr->setUniformValue("u_irradianceMap", 0);
  shaderPtr->setUniformValue("u_prefilterMap", 1);
  shaderPtr->setUniformValue("u_brdfMap", 2);
  shaderPtr->setUniformValue("u_albedoMap", 3);
  shaderPtr->setUniformValue("u_normalMap", 4);
  shaderPtr->setUniformValue("u_ao", m_ao);
  shaderPtr->setUniformValue("u_roughness", m_roughness);
  shaderPtr->setUniformValue("u_metallic", m_metallic);

  // Update our matrices
  update();
}

void MaterialPBR::update()
{

  m_irradianceMap->bind(0);
  m_prefilteredMap->bind(1);
  m_brdfMap->bind(2);
  m_albedoMap->bind(3);
  m_normalMap->bind(4);

  auto shaderPtr = m_shaderLib->getShader(m_shaderName);
  auto eye = m_cam->getCameraEye();
  shaderPtr->setUniformValue("u_camPos", QVector3D{eye.x, eye.y, eye.z});

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

void MaterialPBR::generateCubeMap(
    const Mesh &_cube,
    const MeshVBO &_vbo,
    std::unique_ptr<QOpenGLTexture> &_texture,
    const int _dim,
    const std::string &_matPath,
    const std::function<void (QOpenGLShaderProgram* io_prog)> &_prerender
    )
{
  using tex = QOpenGLTexture;
  auto defaultFBO = m_context->defaultFramebufferObject();
  auto shaderName = m_shaderLib->loadShaderProg(_matPath.c_str());
  auto shader = m_shaderLib->getShader(shaderName);
  auto funcs = m_context->versionFunctions<QOpenGLFunctions_4_1_Core>();

  auto fbo = std::make_unique<QOpenGLFramebufferObject>(_dim, _dim, QOpenGLFramebufferObject::Depth);

  _texture.reset(new QOpenGLTexture(QOpenGLTexture::TargetCubeMap));
  _texture->create();
  _texture->bind(0);
  _texture->setSize(_dim, _dim);
  _texture->setFormat(tex::RGB16F);
  _texture->allocateStorage();
  _texture->setMinMagFilters(tex::Linear, tex::Linear);
  _texture->setWrapMode(tex::ClampToEdge);

  _prerender(shader);

  // don't forget to configure the viewport to the capture dimensions.
  funcs->glViewport(0, 0, _dim, _dim);
  fbo->bind();
  {
    using namespace MeshAttributes;
    shader->enableAttributeArray(VERTEX);
    shader->setAttributeBuffer(VERTEX, GL_FLOAT, _vbo.offset(VERTEX), 3);
  }

  for (unsigned int i = 0; i < 6; ++i)
  {
    // Need to transpose the matrix as they both use different majors
    shader->setUniformValue("u_MV", m_captureViews[i]);
    funcs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, _texture->textureId(), 0);
    funcs->glClearColor(0.f, 0.f, 0.f, 1.f);
    funcs->glClear(GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT);

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
  prefilterShader->setUniformValue("u_envMap", 0);
  prefilterShader->setUniformValue("u_P", m_captureProjection);

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
    prefilterShader->setUniformValue("u_roughness", roughness);
    for (unsigned int i = 0; i < 6; ++i)
    {
      prefilterShader->setUniformValue("u_MV", m_captureViews[i]);
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

  m_brdfMap.reset(new QOpenGLTexture(QOpenGLTexture::Target2D));
  m_brdfMap->create();
  m_brdfMap->bind();
  m_brdfMap->setSize(512, 512);
  m_brdfMap->setFormat(tex::RGB16F);
  m_brdfMap->setMinMagFilters(tex::Linear, tex::Linear);
  m_brdfMap->setWrapMode(tex::ClampToEdge);
  m_brdfMap->allocateStorage();

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

  funcs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_brdfMap->textureId(), 0);
  funcs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  funcs->glDrawElements(GL_TRIANGLES, _plane.getNIndicesData(), GL_UNSIGNED_SHORT, nullptr);

  funcs->glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
  fbo->release();
}

void MaterialPBR::generate3DTexture(
    const Mesh &_plane,
    const MeshVBO &_vbo,
    std::unique_ptr<QOpenGLTexture> &_texture,
    const std::string &_matPath,
    const std::function<void (QOpenGLShaderProgram* io_prog)> &_prerender
    )
{
  static constexpr auto RES = 512;
  using tex = QOpenGLTexture;
  auto defaultFBO = m_context->defaultFramebufferObject();
  auto funcs = m_context->versionFunctions<QOpenGLFunctions_4_1_Core>();

  _texture.reset(new QOpenGLTexture(QOpenGLTexture::Target3D));
  _texture->create();
  _texture->bind();
  _texture->setSize(RES, RES, RES);
  _texture->setFormat(tex::RGBA16F);
  _texture->setMinMagFilters(tex::Linear, tex::Linear);
  _texture->setWrapMode(tex::Repeat);
  _texture->allocateStorage();

  auto shaderName = m_shaderLib->loadShaderProg(_matPath.c_str());
  auto shader = m_shaderLib->getShader(shaderName);

  auto fbo = std::make_unique<QOpenGLFramebufferObject>(RES, RES, QOpenGLFramebufferObject::Depth, GL_TEXTURE_3D);
  funcs->glViewport(0, 0, RES, RES);
  shader->bind();
  {
    using namespace MeshAttributes;
    shader->enableAttributeArray(VERTEX);
    shader->setAttributeBuffer(VERTEX, GL_FLOAT, _vbo.offset(VERTEX), 3);
    shader->enableAttributeArray(UV);
    shader->setAttributeBuffer(UV, GL_FLOAT, _vbo.offset(UV), 2);
  }

  _prerender(shader);

  static constexpr auto denom = 1.f / static_cast<float>(RES);
  for (int i = 0; i < RES; ++i)
  {
    shader->setUniformValue("u_zDepth", i * denom);
    funcs->glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, _texture->textureId(), 0, i);
    funcs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    funcs->glDrawElements(GL_TRIANGLES, _plane.getNIndicesData(), GL_UNSIGNED_SHORT, nullptr);
  }

  funcs->glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
  fbo->release();
}

