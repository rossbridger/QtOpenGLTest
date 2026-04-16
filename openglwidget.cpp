#include <QOpenGLShader>
#include <QKeyEvent>
#include <QVector4D>
#include <QMatrix4x4>
#include <QOpenGLFramebufferObjectFormat>
#include "openglwidget.h"
#include "model.h"


// Default camera values
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;

const float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};

OpenGLWidget::OpenGLWidget(QWidget *parent): QOpenGLWidget(parent),
	QOpenGLExtraFunctions(context()),
	screen_vbo(QOpenGLBuffer::VertexBuffer),
	Front(0.0f, 0.0f, -1.0f),
	MovementSpeed(SPEED),
	MouseSensitivity(SENSITIVITY),
	Zoom(ZOOM)
{
	constexpr int fps = 60;
	screen_shader = new QOpenGLShaderProgram(context());
	timer.start();
	startTimer(1000/fps);

	Position = QVector3D(0.0f, 0.0f, 3.0f);
	WorldUp = QVector3D(0.0f, 1.0f, 0.0f);
	Yaw = YAW;
	Pitch = PITCH;
	lastX = width()/2.0f;
	lastY = height()/2.0f;
	updateCameraVectors();
	grabKeyboard(); // so that it receives keyboard event
	grabMouse();
}

OpenGLWidget::~OpenGLWidget()
{
	delete model;
	glDeleteFramebuffers(1, &fbo);
}

void OpenGLWidget::initializeGL()
{
	initializeOpenGLFunctions();

	// initialize fbo
	glGenFramebuffers(1, &fbo);
	// generate texture
	glGenTextures(1, &screen_texture);
	glGenTextures(1, &depthstencil_texture);
	resizeFramebufferTextures();

	// start binding
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glBindTexture(GL_TEXTURE_2D, screen_texture);
	// attach it to currently bound framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screen_texture, 0);

	// now depth texture
	glBindTexture(GL_TEXTURE_2D, depthstencil_texture);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthstencil_texture, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	model = new Model(context(), "backpack/backpack.obj");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	screen_vao.create();
	screen_vao.bind();
	screen_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, "screen_texture.vs");
	screen_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, "screen_texture.fs");
	screen_shader->link();
	screen_shader->bind();
	screen_vbo.create();
	screen_vbo.bind();
	screen_vbo.allocate(quadVertices, sizeof(quadVertices));
	screen_shader->setAttributeBuffer(0, GL_FLOAT, 0, 2, sizeof(float) * 4);
	screen_shader->enableAttributeArray(0);
	screen_shader->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 2, 2, sizeof(float) * 4);
	screen_shader->enableAttributeArray(1);
	screen_shader->setUniformValue("screenTexture", 0);
	screen_shader->release();
	screen_vbo.release();
	screen_vao.release();
}

void OpenGLWidget::paintGL()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	if (windowResized) { // recreate image object
		resizeFramebufferTextures();
		windowResized = false;
	}

	// TODO: no need to set these matrices every frame
	QMatrix4x4 modelMatrix, viewMatrix, projectionMatrix;
	viewMatrix.setToIdentity();
	projectionMatrix.setToIdentity();
	projectionMatrix.perspective(Zoom, float(width())/height(), 0.1f, 100.0f);
	viewMatrix = GetViewMatrix();

	modelMatrix.setToIdentity();
	modelMatrix.translate(QVector3D(0.0f, 0.0f, 0.0f));
	modelMatrix.scale(QVector3D(1.0f, 1.0f, 1.0f));

	model->setModelMatrix(modelMatrix);
	model->setviewMatrix(viewMatrix);
	model->setProjectMatrix(projectionMatrix);
	model->onDraw();

	// second pass
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, screen_texture);
	screen_shader->bind();
	screen_vao.bind();

	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	update();
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
	float xoffset = event->position().x() - lastX;
	float yoffset = lastY - event->position().y();
	lastX = event->position().x();
	lastY = event->position().y();
	xoffset *= MouseSensitivity;
	yoffset *= MouseSensitivity;
	Yaw   += xoffset;
	Pitch += yoffset;
	if(Pitch > 89.0f)
		Pitch =  89.0f;
	if(Pitch < -89.0f)
		Pitch = -89.0f;
	updateCameraVectors();
	event->accept();
}

void OpenGLWidget::mousePressEvent(QMouseEvent *event)
{
	if(event->button() & Qt::LeftButton) {
		lastX = event->position().x();
		lastY = event->position().y();
	}
	if(event->button() & Qt::RightButton) {
		QImage image(width(), height(), QImage::Format::Format_RGB888);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glBindTexture(GL_TEXTURE_2D, screen_texture);
		glReadPixels(0, 0, width(), height(), GL_RGB, GL_UNSIGNED_BYTE, image.bits());
		glBindTexture(GL_TEXTURE_2D, 0);
		image.flip(); // in order to translate opengl coordinate to QImage
		image.save("output.png");
	}
	event->accept();
}

void OpenGLWidget::keyPressEvent(QKeyEvent *event)
{
	static bool wireframe_mode = false;
	const float velocity = 0.05f;
	switch(event->key()) {
	case Qt::Key_W:
		Position += Front * velocity;
		break;
	case Qt::Key_S:
		Position -= Front * velocity;
		break;
	case Qt::Key_A:
		Position -= Right * velocity;
		break;
	case Qt::Key_D:
		Position += Right * velocity;
		break;
	case Qt::Key_Space:
		// if(!wireframe_mode) {
		// 	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		// } else {
		// 	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		// }
		wireframe_mode = !wireframe_mode;
	default:
		QOpenGLWidget::keyPressEvent(event);
		return;
	}
	event->accept();
}

void OpenGLWidget::wheelEvent(QWheelEvent *event)
{
	float yoffset = event->angleDelta().y() / 8;
	Zoom -= (float)yoffset;
	if (Zoom < 1.0f)
		Zoom = 1.0f;
	if (Zoom > 45.0f)
		Zoom = 45.0f;
}

void OpenGLWidget::resizeGL(int w, int h)
{
	windowResized = true;
}

void OpenGLWidget::updateCameraVectors()
{
	QVector3D front;
	front.setX(cos(qDegreesToRadians(Yaw)) * cos(qDegreesToRadians(Pitch)));
	front.setY(sin(qDegreesToRadians(Pitch)));
	front.setZ(sin(qDegreesToRadians(Yaw)) * cos(qDegreesToRadians(Pitch)));
	Front = front.normalized();
	Right = QVector3D::crossProduct(Front, WorldUp).normalized();
	Up = QVector3D::crossProduct(Right, Front).normalized();
}

void OpenGLWidget::resizeFramebufferTextures()
{
	glBindTexture(GL_TEXTURE_2D, screen_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width(), height(), 0,
		     GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, depthstencil_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width(), height(), 0,
		     GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
}

QMatrix4x4 OpenGLWidget::GetViewMatrix()
{
	QMatrix4x4 look_at;
	look_at.setToIdentity();
	look_at.lookAt(Position, Position + Front, Up);
	return look_at;
}
