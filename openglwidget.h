#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QElapsedTimer>

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
private:
	QElapsedTimer timer;
	QOpenGLShaderProgram *program;
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;
	//unsigned int texture;
	QOpenGLTexture *texture[2];
	QMatrix4x4 trans;
	bool is_cooldown;
};

#endif // OPENGLWIDGET_H
