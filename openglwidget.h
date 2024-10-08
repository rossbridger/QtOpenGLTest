#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QElapsedTimer>
#include <QVector3D>
#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>
#include "mesh.h"
#include "model.h"

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
	Q_OBJECT
public:
	OpenGLWidget(QWidget *parent = nullptr);
	~OpenGLWidget();
protected:
	virtual void initializeGL() override;
	virtual void paintGL() override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void wheelEvent(QWheelEvent *event) override;
private:
	QElapsedTimer timer;
	QOpenGLShaderProgram *shader;
	QOpenGLShaderProgram *screen_shader;
	QOpenGLBuffer screen_vbo;
	QOpenGLVertexArrayObject screen_vao;
	float lastX;
	float lastY;
	QVector3D Position;
	QVector3D Front;
	QVector3D Up;
	QVector3D Right;
	QVector3D WorldUp;
	float Yaw;
	float Pitch;
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;
	Model *model;
	QOpenGLFramebufferObject *fbo;
	void updateCameraVectors();
	QMatrix4x4 GetViewMatrix();
	void setupMesh(Mesh &mesh);
	void setupModel();
	void drawMesh(Mesh &mesh);
	void drawModel();
};

#endif // OPENGLWIDGET_H
