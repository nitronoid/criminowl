#include "HDR_cube.h"
#include "Scene.h"
#include "ShaderLib.h"
#include "Mesh.h"
#include "MeshVBO.h"
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_1_Core>
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"


void HDR_cube::init(const std::shared_ptr<ShaderLib> &io_shaderLib, QOpenGLContext *io_context)
{
  auto defaultFBO = io_context->defaultFramebufferObject();
  auto hdrTex = initSphereMap(io_context);
  auto cubeMapShaderName = io_shaderLib->loadShaderProg("shaderPrograms/hdr_cubemap.json");
  auto cubeMapShader = io_shaderLib->getShader(cubeMapShaderName);
  auto funcs = io_context->versionFunctions<QOpenGLFunctions_4_1_Core>();

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

  unsigned int envCubeMap;
  funcs->glGenTextures(1, &envCubeMap);
  funcs->glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);
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
  cubeMapShader->setUniformValue("P", captureProjection);
  funcs->glActiveTexture(GL_TEXTURE0);
  funcs->glBindTexture(GL_TEXTURE_2D, hdrTex);

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
    cubeMapShader->setUniformValue("M", view.transposed());
    funcs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubeMap, 0);
    funcs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    funcs->glDrawElements(GL_TRIANGLES, cube.getNIndicesData(), GL_UNSIGNED_SHORT, nullptr);
  }
  funcs->glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
  //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  funcs->glGenTextures(1, &m_irradianceMap);
  funcs->glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceMap);
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
  auto irradianceShaderName = io_shaderLib->loadShaderProg("shaderPrograms/hdr_cubemap_irradiance.json");
  auto irradianceShader = io_shaderLib->getShader(irradianceShaderName);
  irradianceShader->bind();
  irradianceShader->setUniformValue("envMap", 0);
  irradianceShader->setUniformValue("P", captureProjection);
  funcs->glActiveTexture(GL_TEXTURE0);
  funcs->glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);

  funcs->glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
  funcs->glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  for (unsigned int i = 0; i < 6; ++i)
  {
    // Convert from glm to Qt
    QMatrix4x4 view(glm::value_ptr(captureViews[i]));
    // Need to transpose the matrix as they both use different majors
    irradianceShader->setUniformValue("M", view.transposed());
    funcs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_irradianceMap, 0);
    funcs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    funcs->glDrawElements(GL_TRIANGLES, cube.getNIndicesData(), GL_UNSIGNED_SHORT, nullptr);
  }

  funcs->glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
}

unsigned int HDR_cube::initSphereMap(QOpenGLContext *io_context)
{
  stbi_set_flip_vertically_on_load(true);
  int width, height, nrComponents;
  float *data = stbi_loadf("images/LA_Downtown_Afternoon_Fishing_3k.hdr", &width, &height, &nrComponents, 0);
  unsigned int hdrTexture;

  auto funcs = io_context->versionFunctions<QOpenGLFunctions_4_1_Core>();
  funcs->glGenTextures(1, &hdrTexture);
  funcs->glBindTexture(GL_TEXTURE_2D, hdrTexture);
  funcs->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
  funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  stbi_image_free(data);
  return hdrTexture;
}


