#include "openglwidget.h"


OpenGLWidget::OpenGLWidget(QWidget *parent): QOpenGLWidget(parent), QOpenGLExtraFunctions(context())
{
}

OpenGLWidget::~OpenGLWidget()
{
}

void OpenGLWidget::initializeGL()
{
	initializeOpenGLFunctions();
}

void OpenGLWidget::paintGL()
{
}

void OpenGLWidget::resizeGL(int w, int h)
{

}
