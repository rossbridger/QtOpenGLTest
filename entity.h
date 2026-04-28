#ifndef ENTITY_H
#define ENTITY_H

#include <QObject>
#include <qopenglextrafunctions.h>
#include <qopenglshaderprogram.h>

class Entity : public QObject, public QOpenGLExtraFunctions
{
	Q_OBJECT
public:
	explicit Entity(QOpenGLContext* context);
	virtual void draw(QOpenGLShaderProgram* shader) = 0;
	void setModelMatrix(QMatrix4x4 matrix);

protected:
	QMatrix4x4 modelMatrix;
};

#endif // ENTITY_H
