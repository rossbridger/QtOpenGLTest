#include <QOpenGLShader>
#include "openglwidget.h"


OpenGLWidget::OpenGLWidget(QWidget *parent): QOpenGLWidget(parent), QOpenGLExtraFunctions(context())
{
	m_program = new QOpenGLShaderProgram(context());
}

OpenGLWidget::~OpenGLWidget()
{
	delete m_program;
}

void OpenGLWidget::initializeGL()
{
	initializeOpenGLFunctions();
	m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, QString("vertex.vert"));
	m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, QString("fragment.frag"));
	m_program->link();
	m_program->bind();
}

void OpenGLWidget::paintGL()
{
}

void OpenGLWidget::resizeGL(int w, int h)
{

}
