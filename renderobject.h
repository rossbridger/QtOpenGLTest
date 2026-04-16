#ifndef RENDEROBJECT_H
#define RENDEROBJECT_H

#include <QOpenGLShaderProgram>
#include <qopenglextrafunctions.h>

class RenderObject: public QObject, protected QOpenGLExtraFunctions
{
	Q_OBJECT
public:
	RenderObject(QOpenGLContext* context = nullptr);
	~RenderObject();
	void setShader(const char* vertex, const char* fragment);
	virtual void onDraw() = 0;
protected:
	QOpenGLShaderProgram *shader;
	QOpenGLContext* context;
};

#endif // RENDEROBJECT_H
