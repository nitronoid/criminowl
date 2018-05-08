#include "MainWindow.h"


void MainWindow::init(const std::shared_ptr<Scene> &io_scene)
{
  m_scene = io_scene;
  m_ui.setupUi(this);
  m_ui.s_mainWindowGridLayout->addWidget(m_scene.get(),0,0,3,4);
  connect(m_ui.metallicSpinBox, SIGNAL(valueChanged(double)), m_scene.get(), SLOT(metallicUpdate(double)));
  connect(m_ui.roughnessSpinBox, SIGNAL(valueChanged(double)), m_scene.get(), SLOT(roughnessUpdate(double)));
  connect(m_ui.baseSpecSpinBox, SIGNAL(valueChanged(double)), m_scene.get(), SLOT(baseSpecUpdate(double)));
  connect(m_ui.normalStrengthSpinBox, SIGNAL(valueChanged(double)), m_scene.get(), SLOT(normalStrengthUpdate(double)));
  connect(m_ui.pausedCheckBox, SIGNAL(clicked(bool)), m_scene.get(), SLOT(setPaused(bool)));

  m_ui.tessButtonGroup->setId(m_ui.flatTessRadioButton, 0);
  m_ui.tessButtonGroup->setId(m_ui.phongTessRadioButton, 1);
  connect(m_ui.tessButtonGroup, SIGNAL(buttonClicked(int)), m_scene.get(), SLOT(tessUpdate(int)));
  connect(m_ui.tessLevelInnerSpinBox, SIGNAL(valueChanged(int)), m_scene.get(), SLOT(tessLevelInnerUpdate(int)));
  connect(m_ui.tessLevelOuterSpinBox, SIGNAL(valueChanged(int)), m_scene.get(), SLOT(tessLevelOuterUpdate(int)));
  connect(m_ui.tessMaskCapSpinBox, SIGNAL(valueChanged(double)), m_scene.get(), SLOT(tessMaskCapUpdate(double)));


  connect(m_ui.eyeDisplacementSpinBox, SIGNAL(valueChanged(double)), m_scene.get(), SLOT(eyeDispUpdate(double)));
  connect(m_ui.eyeRotateSpinBox, SIGNAL(valueChanged(double)), m_scene.get(), SLOT(eyeRotationUpdate(double)));
  connect(m_ui.eyeScaleSpinBox, SIGNAL(valueChanged(double)), m_scene.get(), SLOT(eyeScaleUpdate(double)));
  connect(m_ui.eyeWarpSpinBox, SIGNAL(valueChanged(double)), m_scene.get(), SLOT(eyeWarpUpdate(double)));
  connect(m_ui.eyeExponentSpinBox, SIGNAL(valueChanged(double)), m_scene.get(), SLOT(eyeExponentUpdate(double)));
  connect(m_ui.eyeThicknessSpinBox, SIGNAL(valueChanged(double)), m_scene.get(), SLOT(eyeThicknessUpdate(double)));
  connect(m_ui.eyeGapSpinBox, SIGNAL(valueChanged(double)), m_scene.get(), SLOT(eyeGapUpdate(double)));
  connect(m_ui.eyeFuzzSpinBox, SIGNAL(valueChanged(double)), m_scene.get(), SLOT(eyeFuzzUpdate(double)));
  connect(m_ui.eyeFuzzSpinBox, SIGNAL(valueChanged(double)), m_scene.get(), SLOT(eyeFuzzUpdate(double)));
  connect(m_ui.eyeMaskCapSpinBox, SIGNAL(valueChanged(double)), m_scene.get(), SLOT(eyeMaskCapUpdate(double)));
}

//----------------------------------------------------------------------------------------------------------------------

void MainWindow::keyPressEvent(QKeyEvent *io_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch ( io_event->key() )
  {
    case Qt::Key_Escape : QApplication::exit(EXIT_SUCCESS); break;
    default : break;
  }
  m_scene->keyPress(io_event);
}

//----------------------------------------------------------------------------------------------------------------------

void MainWindow::mouseMoveEvent(QMouseEvent * io_event)
{
  m_scene->mouseMove(io_event);
}

//----------------------------------------------------------------------------------------------------------------------

void MainWindow::mousePressEvent(QMouseEvent * io_event)
{
  m_scene->mouseClick(io_event);
}

//----------------------------------------------------------------------------------------------------------------------

void MainWindow::mouseReleaseEvent(QMouseEvent * io_event)
{
  m_scene->mouseClick(io_event);
}

//----------------------------------------------------------------------------------------------------------------------
