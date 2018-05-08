#ifndef DEMOSCENE_H
#define DEMOSCENE_H

#include "Scene.h"
#include "MaterialPBR.h"
#include "MaterialPhong.h"
#include "ShaderLib.h"


class DemoScene : public Scene
{
  Q_OBJECT
public:
  //-----------------------------------------------------------------------------------------------------
  /// @brief Constructor for DemoScene.
  /// @param [io] io_camera the camera used to view the scene.
  /// @param [io] io_shaderLib the shader library to store and retrieve our shaders.
  /// @param [io] io_parent the parent window to create the GL context in.
  //-----------------------------------------------------------------------------------------------------
  DemoScene(
      const std::shared_ptr<Camera> &io_camera,
      const std::shared_ptr<ShaderLib> &io_shaderLib,
      QWidget *_parent
      ) :
    Scene(io_camera, _parent),
    m_shaderLib(io_shaderLib)
  {}
  //-----------------------------------------------------------------------------------------------------
  /// @brief Default copy constructor.
  //-----------------------------------------------------------------------------------------------------
  DemoScene(const DemoScene&) = default;
  //-----------------------------------------------------------------------------------------------------
  /// @brief Default copy assignment operator.
  //-----------------------------------------------------------------------------------------------------
  DemoScene& operator=(const DemoScene&) = default;
  //-----------------------------------------------------------------------------------------------------
  /// @brief Default move constructor.
  //-----------------------------------------------------------------------------------------------------
  DemoScene(DemoScene&&) = default;
  //-----------------------------------------------------------------------------------------------------
  /// @brief Default move assignment operator.
  //-----------------------------------------------------------------------------------------------------
  DemoScene& operator=(DemoScene&&) = default;
  //-----------------------------------------------------------------------------------------------------
  /// @brief Default destructor.
  //-----------------------------------------------------------------------------------------------------
  ~DemoScene() override = default;
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to intialise the scene, must call the base class init.
  //-----------------------------------------------------------------------------------------------------
  virtual void init() override;
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to intialise the models, vbo and vao.
  //-----------------------------------------------------------------------------------------------------
  void initGeo();
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to create our shader programs, or use exisiting ones if they have been loaded.
  //-----------------------------------------------------------------------------------------------------
  void initMaterials();
  //-----------------------------------------------------------------------------------------------------
  /// @brief Receives and acts on a key event.
  /// @param [io] io_event is the key event that was received.
  //-----------------------------------------------------------------------------------------------------
  virtual void keyPress(QKeyEvent* io_event) override;

public slots:
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to link a Qt button to the scene, to change the metallic value of the shader.
  /// @param [in] _metallic is the new value for the shader.
  //-----------------------------------------------------------------------------------------------------
  void metallicUpdate(const double _metallic);
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to link a Qt button to the scene, to change the roughness value of the shader.
  /// @param [in] _roughness is the new value for the shader.
  //-----------------------------------------------------------------------------------------------------
  void roughnessUpdate(const double _roughness);
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to link a Qt button to the scene, to change the base specular value of the shader.
  /// @param [in] _baseSpec is the new value for the shader.
  //-----------------------------------------------------------------------------------------------------
  void baseSpecUpdate(const double _baseSpec);
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to link a Qt button to the scene, to change the normal strength value of the shader.
  /// @param [in] _normalStrength is the new value for the shader.
  //-----------------------------------------------------------------------------------------------------
  void normalStrengthUpdate(const double _normalStrength);
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to link a Qt button to the scene, to pause and unpause the morph targets.
  /// @param [in] _paused is the new value for the shader.
  //-----------------------------------------------------------------------------------------------------
  void setPaused(const bool _paused);
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to link a Qt button to the scene, to allow switching between meshes in the scene, this
  /// calls loadMesh.
  //-----------------------------------------------------------------------------------------------------
  void generateNewGeometry();

private:
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to write our mesh data into the vbo.
  //-----------------------------------------------------------------------------------------------------
  void writeMeshAttributes();
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to pass attribute pointers to the current shader program.
  //-----------------------------------------------------------------------------------------------------
  void setAttributeBuffers();
  //-----------------------------------------------------------------------------------------------------
  /// @brief Must call the base class function, it then applies our shader and draws the current mesh.
  //-----------------------------------------------------------------------------------------------------
  virtual void renderScene() override;


private:
  //-----------------------------------------------------------------------------------------------------
  /// @brief Holds our test meshes.
  //-----------------------------------------------------------------------------------------------------
  TriMesh m_owlMesh;
  //-----------------------------------------------------------------------------------------------------
  /// @brief Wraps up our OpenGL buffers and VAO.
  //-----------------------------------------------------------------------------------------------------
  MeshVBO m_meshVBO;
  //-----------------------------------------------------------------------------------------------------
  /// @brief Vertex array object, default constructed with a pointer to this OpenGL widget,
  /// a dynamic_cast is used due to Scene's multiple inheritence.
  //-----------------------------------------------------------------------------------------------------
  std::unique_ptr<QOpenGLVertexArrayObject> m_vao {
    new QOpenGLVertexArrayObject(dynamic_cast<QObject*>(this))
  };
  //-----------------------------------------------------------------------------------------------------
  /// @brief A pointer to the shader library used by this scene.
  //-----------------------------------------------------------------------------------------------------
  std::shared_ptr<ShaderLib> m_shaderLib;
  //-----------------------------------------------------------------------------------------------------
  /// @brief The materials used in this scene.
  //-----------------------------------------------------------------------------------------------------
  std::unique_ptr<MaterialPBR> m_material;
  //-----------------------------------------------------------------------------------------------------
  /// @brief Is the mesh rotating.
  //-----------------------------------------------------------------------------------------------------
  bool m_rotating = false;




};

#endif // DEMOSCENE_H
