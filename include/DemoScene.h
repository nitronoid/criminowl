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
  /// @brief Used to link a Qt button to the scene, to change the ao value of the shader.
  /// @param [in] _ao is the new value for the shader.
  //-----------------------------------------------------------------------------------------------------
  void aoUpdate(const double _ao);
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to link a Qt button to the scene, to change the x value of the eye translate.
  /// @param [in] _x is the new value for the shader.
  //-----------------------------------------------------------------------------------------------------
  void eyeTranslateXUpdate(const double _x);
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to link a Qt button to the scene, to change the y value of the eye translate.
  /// @param [in] _y is the new value for the shader.
  //-----------------------------------------------------------------------------------------------------
  void eyeTranslateYUpdate(const double _y);
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to link a Qt button to the scene, to change the z value of the eye translate.
  /// @param [in] _z is the new value for the shader.
  //-----------------------------------------------------------------------------------------------------
  void eyeTranslateZUpdate(const double _z);
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
  /// @brief Used to link a Qt button to the scene, to set the tessellation algorithm for the eyes.
  /// @param [in] _tessType is the new value for the shader.
  //-----------------------------------------------------------------------------------------------------
  void tessUpdate(const int _tessType);
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to link a Qt button to the scene, to set the phong strength for the eye tessellation.
  /// @param [in] _strengthPercent is the new value for the shader.
  //-----------------------------------------------------------------------------------------------------
  void phongStrengthUpdate(const int _strengthPercent);
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to link a Qt button to the scene, to set the tessellation amount for the eyes.
  /// @param [in] _tessLevel is the new value for the shader.
  //-----------------------------------------------------------------------------------------------------
  void tessLevelInnerUpdate(const int _tessLevel);
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to link a Qt button to the scene, to set the tessellation amount for the eyes.
  /// @param [in] _tessLevel is the new value for the shader.
  //-----------------------------------------------------------------------------------------------------
  void tessLevelOuterUpdate(const int _tessLevel);
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to link a Qt button to the scene, to change the eye displacement amount of the shader.
  /// @param [in] _eyeDisp is the new value for the shader.
  //-----------------------------------------------------------------------------------------------------
  void eyeDispUpdate(const double _eyeDisp);
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to link a Qt button to the scene, to change the eye scale of the shader.
  /// @param [in] _eyeScale is the new value for the shader.
  //-----------------------------------------------------------------------------------------------------
  void eyeScaleUpdate(const double _eyeScale);
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to link a Qt button to the scene, to change the eye rotation amount of the shader.
  /// @param [in] _eyeRotation is the new value for the shader.
  //-----------------------------------------------------------------------------------------------------
  void eyeRotationUpdate(const double _eyeRotation);
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to link a Qt button to the scene, to change the eye warp amount of the shader.
  /// @param [in] _eyeWarp is the new value for the shader.
  //-----------------------------------------------------------------------------------------------------
  void eyeWarpUpdate(const double _eyeWarp);
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to link a Qt button to the scene, to change the eye exponent of the shader.
  /// @param [in] _eyeExponent is the new value for the shader.
  //-----------------------------------------------------------------------------------------------------
  void eyeExponentUpdate(const double _eyeExponent);
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to link a Qt button to the scene, to change the eye thickness of the shader.
  /// @param [in] _eyeThickness is the new value for the shader.
  //-----------------------------------------------------------------------------------------------------
  void eyeThicknessUpdate(const double _eyeThickness);
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to link a Qt button to the scene, to change the eye gap size of the shader.
  /// @param [in] _eyeGap is the new value for the shader.
  //-----------------------------------------------------------------------------------------------------
  void eyeGapUpdate(const double _eyeGap);
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to link a Qt button to the scene, to change the eye fuzz amount of the shader.
  /// @param [in] _eyeFuzz is the new value for the shader.
  //-----------------------------------------------------------------------------------------------------
  void eyeFuzzUpdate(const double _eyeFuzz);
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to link a Qt button to the scene, to change the eye mask cap size of the shader.
  /// @param [in] _cap is the new value for the shader.
  //-----------------------------------------------------------------------------------------------------
  void eyeMaskCapUpdate(const double _cap);
  //-----------------------------------------------------------------------------------------------------
  /// @brief Used to link a Qt button to the scene, to change the tess mask cap size of the shader.
  /// @param [in] _cap is the new value for the shader.
  //-----------------------------------------------------------------------------------------------------
  void tessMaskCapUpdate(const double _cap);
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


};

#endif // DEMOSCENE_H
