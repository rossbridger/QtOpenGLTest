#include "renderobject.h"
#include <QApplication>


RenderObject::RenderObject(QOpenGLContext *context): QObject(context), QOpenGLExtraFunctions(context)
{
	if (context == nullptr)
	{
		qFatal() << "Creating a render object without OpenGL context!";
		qApp->exit(-1);
	}
	this->context = context;
	shader = new QOpenGLShaderProgram(context);
}

RenderObject::~RenderObject()
{
	delete shader;
}

void RenderObject::setShader(const char* vertex, const char* fragment)
{
	assert(shader->addShaderFromSourceFile(QOpenGLShader::Vertex, vertex));
	assert(shader->addShaderFromSourceFile(QOpenGLShader::Fragment, fragment));
	shader->link();
}
