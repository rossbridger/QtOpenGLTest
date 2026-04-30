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
#include <QOpenGLDebugLogger>
#include "entity.h"
#include "model.h"
#include "camera.h"

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
	Q_OBJECT
public:
	OpenGLWidget(QWidget *parent = nullptr);
	~OpenGLWidget();
public slots:
	void setSunAzimuth(float azimuth);
	void setSunAltitude(float altitude);
protected:
	virtual void initializeGL() override;
	virtual void paintGL() override;
	virtual void resizeGL(int w, int h) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void wheelEvent(QWheelEvent *event) override;
private:
	QElapsedTimer timer;
	QOpenGLShaderProgram *mesh_shader;
	QOpenGLShaderProgram *screen_shader;
	QOpenGLShaderProgram *skybox_shader;
	QOpenGLBuffer screen_vbo;
	QOpenGLBuffer skybox_vbo;
	QOpenGLVertexArrayObject screen_vao;
	QOpenGLVertexArrayObject skybox_vao;
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
	Camera camera;
	QVector<Entity*> entities;
	QAtomicInteger<bool> windowResized;
	GLuint fbo;
	GLuint depthfbo;
	GLuint screen_texture;
	GLuint depth_texture;
	GLuint cubeTexture;
	GLuint brdfTexture;
	float near_plane;
	float far_plane;
	float sun_azimuth;
	float sun_altitude;
	void updateCameraVectors();
	void resizeFramebufferTextures();
	void initializeSkybox();
	void initializePostProcessing();
	void initalizeMesh();
	void loadCubemap(const std::array<const char*, 6>& faces);
	void meshPass();
	void skyboxPass();
	void postProcessingPass();
};

#endif // OPENGLWIDGET_H
