#include "MainWindow.h"


void MainWindow::init(const std::shared_ptr<Scene> &io_scene)
{
  m_scene = io_scene;
  m_ui.setupUi(this);
  m_ui.s_mainWindowGridLayout->addWidget(m_scene.get(),0,0,3,5);
  connect(m_ui.metallicSpinBox, SIGNAL(valueChanged(double)), m_scene.get(), SLOT(metallicUpdate(double)));
  connect(m_ui.roughnessSpinBox, SIGNAL(valueChanged(double)), m_scene.get(), SLOT(roughnessUpdate(double)));
  connect(m_ui.baseSpecSpinBox, SIGNAL(valueChanged(double)), m_scene.get(), SLOT(baseSpecUpdate(double)));
  connect(m_ui.normalStrengthSpinBox, SIGNAL(valueChanged(double)), m_scene.get(), SLOT(normalStrengthUpdate(double)));
  connect(m_ui.pausedCheckBox, SIGNAL(clicked(bool)), m_scene.get(), SLOT(setPaused(bool)));
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
