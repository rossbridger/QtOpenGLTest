#ifndef SKYBOX_H
#define SKYBOX_H

#include <array>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include "renderobject.h"

class Skybox : public RenderObject
{
	Q_OBJECT
public:
	Skybox(QOpenGLContext* context);
	void onDraw() override;
	void setviewMatrix(QMatrix4x4 viewMatrix) {this->viewMatrix = viewMatrix;}
	void setProjectMatrix(QMatrix4x4 projectionMatrix) {this->projectionMatrix = projectionMatrix;}
private:
	void loadCubemap(const std::array<const char*, 6>& faces);
	GLuint cubeTexture;
	QOpenGLBuffer vbo;
	QOpenGLVertexArrayObject vao;
	QMatrix4x4 viewMatrix, projectionMatrix;
};

#endif // SKYBOX_H
