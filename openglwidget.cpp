#include <QOpenGLShader>
#include <QKeyEvent>
#include <QVector4D>
#include <QMatrix4x4>
#include "openglwidget.h"


OpenGLWidget::OpenGLWidget(QWidget *parent): QOpenGLWidget(parent), QOpenGLExtraFunctions(context()),
	cameraPos(0.0f, 0.0f, 3.0f),
	cameraFront(0.0f, 0.0f, -1.0f),
	cameraUp(0.0f, 1.0f, 0.0f),
	pitch(0.0f),
	yaw(-90.0f),
	fov(45.0f)
{
	constexpr int fps = 60;
	program = new QOpenGLShaderProgram(context());
	timer.start();
	startTimer(1000/fps);
	grabKeyboard(); // so that it receives keyboard event
	grabMouse();
}

OpenGLWidget::~OpenGLWidget()
{
	delete texture[0];
	delete texture[1];
}

void OpenGLWidget::initializeGL()
{
	initializeOpenGLFunctions();
	assert(program->addShaderFromSourceFile(QOpenGLShader::Vertex, QString("vertex.vert")));
	assert(program->addShaderFromSourceFile(QOpenGLShader::Fragment, QString("fragment.frag")));
	program->link();

	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};
	// unsigned int indices[] = {
	// 	0, 1, 3, // first triangle
	// 	1, 2, 3  // second triangle
	// };
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// glGenBuffers(1, &EBO);
	// glGenTextures(1, &texture);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	// glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// QImage image = QImage("container.jpg").convertToFormat(QImage::Format_RGB888);
	// glBindTexture(GL_TEXTURE_2D, texture);
	// // set the texture wrapping/filtering options (on the currently bound texture object)
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width(), image.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, image.constBits());
	// glGenerateMipmap(GL_TEXTURE_2D);

	texture[0] = new QOpenGLTexture(QImage("container.jpg").convertToFormat(QImage::Format_RGB888));
	texture[0]->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
	texture[0]->setMagnificationFilter(QOpenGLTexture::Linear);

	texture[1] = new QOpenGLTexture(QImage("awesomeface.png").convertToFormat(QImage::Format_RGB888));
	texture[1]->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
	texture[1]->setMagnificationFilter(QOpenGLTexture::Linear);
	glEnable(GL_DEPTH_TEST);
}

void OpenGLWidget::paintGL()
{
	QVector3D cubePositions[] = {
		QVector3D( 0.0f,  0.0f,  0.0f),
		QVector3D( 2.0f,  5.0f, -15.0f),
		QVector3D(-1.5f, -2.2f, -2.5f),
		QVector3D(-3.8f, -2.0f, -12.3f),
		QVector3D( 2.4f, -0.4f, -3.5f),
		QVector3D(-1.7f,  3.0f, -7.5f),
		QVector3D( 1.3f, -2.0f, -2.5f),
		QVector3D( 1.5f,  2.0f, -2.5f),
		QVector3D( 1.5f,  0.2f, -1.5f),
		QVector3D(-1.3f,  1.0f, -1.5f)
	};
	QMatrix4x4 model, view, projection;
	view.setToIdentity();
	projection.setToIdentity();
	float elapsed_ms = timer.elapsed()/1000.0f;
	model.rotate(qRadiansToDegrees(elapsed_ms), 0.5f, 1.0f, 0.0f);
	projection.perspective(fov, 800.0f / 600.0f, 0.1f, 100.0f);
	view.lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	program->bind();
	program->setUniformValue("texture1", 0);
	program->setUniformValue("texture2", 1);
	program->setUniformValue("view", view);
	program->setUniformValue("projection", projection);

	glActiveTexture(GL_TEXTURE0);
	texture[0]->bind();
	glActiveTexture(GL_TEXTURE1);
	texture[1]->bind();
	// glBindTexture(GL_TEXTURE_2D, texture);
	glBindVertexArray(VAO);
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	for(unsigned int i = 0; i < 10; i++) {
		model.setToIdentity();
		model.translate(cubePositions[i]);
		float angle = 20.0f * i;
		model.rotate(qRadiansToDegrees(angle), 1.0f, 0.3f, 0.5f);
		program->setUniformValue("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}

void OpenGLWidget::resizeGL(int w, int h)
{
}

void OpenGLWidget::timerEvent(QTimerEvent *event)
{
	update();
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
	float xoffset = event->position().x() - lastX;
	float yoffset = lastY - event->position().y();
	const float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;
	yaw   += xoffset;
	pitch += yoffset;
	if(pitch > 89.0f)
	  pitch =  89.0f;
	if(pitch < -89.0f)
	  pitch = -89.0f;
	QVector3D direction;
	direction.setX(cos(qDegreesToRadians(yaw)) * cos(qDegreesToRadians(pitch)));
	direction.setY(sin(qDegreesToRadians(pitch)));
	direction.setZ(sin(qDegreesToRadians(yaw)) * cos(qDegreesToRadians(pitch)));
	cameraFront = direction.normalized();
}

void OpenGLWidget::mousePressEvent(QMouseEvent *event)
{
	lastX = event->position().x();
	lastY = event->position().y();
	event->accept();
}

void OpenGLWidget::keyPressEvent(QKeyEvent *event)
{
	const float cameraSpeed = 0.05f;
	switch(event->key()) {
	case Qt::Key_W:
		cameraPos += cameraSpeed * cameraFront;
		break;
	case Qt::Key_S:
		cameraPos -= cameraSpeed * cameraFront;
		break;
	case Qt::Key_A:
		cameraPos -= QVector3D::crossProduct(cameraFront, cameraUp).normalized() * cameraSpeed;
		break;
	case Qt::Key_D:
		cameraPos += QVector3D::crossProduct(cameraFront, cameraUp).normalized() * cameraSpeed;
		break;
	default:
		QOpenGLWidget::keyPressEvent(event);
		return;
	}
	event->accept();
}

void OpenGLWidget::wheelEvent(QWheelEvent *event)
{
	float yoffset = event->angleDelta().y() / 8;
	fov -= (float)yoffset;
	if (fov < 1.0f)
		fov = 1.0f;
	if (fov > 45.0f)
		fov = 45.0f;
}

