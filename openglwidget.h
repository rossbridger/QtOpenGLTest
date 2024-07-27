#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QElapsedTimer>
#include <QVector3D>

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
	Q_OBJECT
public:
	OpenGLWidget(QWidget *parent = nullptr);
	~OpenGLWidget();
protected:
	virtual void initializeGL() override;
	virtual void paintGL() override;
	virtual void resizeGL(int w, int h) override;
	virtual void timerEvent(QTimerEvent *event) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void wheelEvent(QWheelEvent *event) override;
private:
	QElapsedTimer timer;
	QOpenGLShaderProgram *program;
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;
	//unsigned int texture;
	QOpenGLTexture *texture[2];
	QVector3D cameraPos;
	QVector3D cameraFront;
	QVector3D cameraUp;
	float lastX;
	float lastY;
	float pitch;
	float yaw;
	float fov;
};

#endif // OPENGLWIDGET_H
