#include <QOpenGLShader>
#include <QKeyEvent>
#include <QVector4D>
#include <QMatrix4x4>
#include "openglwidget.h"


// Default camera values
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;

OpenGLWidget::OpenGLWidget(QWidget *parent): QOpenGLWidget(parent), QOpenGLExtraFunctions(context()),
	Front(0.0f, 0.0f, -1.0f), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
{
	constexpr int fps = 60;
	lightingShader = new QOpenGLShaderProgram(context());
	lightCubeShader = new QOpenGLShaderProgram(context());
	timer.start();
	startTimer(1000/fps);
	diffuseMap = nullptr;
	specularMap = nullptr;

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
	if(diffuseMap != nullptr) {
		delete diffuseMap;
	}
	if(specularMap != nullptr) {
		delete specularMap;
	}
}

void OpenGLWidget::initializeGL()
{
	initializeOpenGLFunctions();
	glEnable(GL_DEPTH_TEST);

	assert(lightingShader->addShaderFromSourceFile(QOpenGLShader::Vertex, QString("colors.vs")));
	assert(lightingShader->addShaderFromSourceFile(QOpenGLShader::Fragment, QString("colors.fs")));

	assert(lightCubeShader->addShaderFromSourceFile(QOpenGLShader::Vertex, QString("light_cube.vs")));
	assert(lightCubeShader->addShaderFromSourceFile(QOpenGLShader::Fragment, QString("light_cube.fs")));
	lightingShader->link();
	lightCubeShader->link();

	float vertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};

	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(cubeVAO);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// position attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
	glGenVertexArrays(1, &lightCubeVAO);
	glBindVertexArray(lightCubeVAO);

	// we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
	//glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	diffuseMap = new QOpenGLTexture(QImage("container2.png").convertedTo(QImage::Format_RGB888));
	diffuseMap->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
	diffuseMap->setMagnificationFilter(QOpenGLTexture::Linear);


	specularMap = new QOpenGLTexture(QImage("container2_specular.png").convertedTo(QImage::Format_RGB888));
	specularMap->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
	specularMap->setMagnificationFilter(QOpenGLTexture::Linear);
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
	QVector3D lightPos(1.2f, 1.0f, 2.0f);
	view.setToIdentity();
	projection.setToIdentity();

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	lightingShader->bind();
	lightingShader->setUniformValue("material.diffuse", 0);
	lightingShader->setUniformValue("material.specular", 1);

	lightingShader->setUniformValue("material.shininess", 64.0f);

	lightingShader->setUniformValue("light.ambient", 0.2f, 0.2f, 0.2f);
	lightingShader->setUniformValue("light.diffuse", 0.5f, 0.5f, 0.5f); // darken diffuse light a bit
	lightingShader->setUniformValue("light.specular", 1.0f, 1.0f, 1.0f);
	lightingShader->setUniformValue("light.direction", -0.2f, -1.0f, -0.3f);

	projection.setToIdentity();
	projection.perspective(Zoom, width()/height(), 0.1f, 100.0f);
	view = GetViewMatrix();

	lightingShader->setUniformValue("projection", projection);
	lightingShader->setUniformValue("view", view);
	lightingShader->setUniformValue("viewPos", Position);

	diffuseMap->bind(0);
	specularMap->bind(1);

	// render the cube
	glBindVertexArray(cubeVAO);
	for(unsigned int i = 0; i < 10; i++)
	{
		model.setToIdentity();
		model.translate(cubePositions[i]);
		float angle = 20.0f * i;
		model.rotate(angle, 1.0f, 0.3f, 0.5f);
		lightingShader->setUniformValue("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	lightCubeShader->bind();
	lightCubeShader->setUniformValue("projection", projection);
	lightCubeShader->setUniformValue("view", view);
	model.setToIdentity();
	model.translate(lightPos);
	model.scale(0.2f);
	lightCubeShader->setUniformValue("model", model);

	glBindVertexArray(lightCubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	update();
}

void OpenGLWidget::resizeGL(int w, int h)
{
}

void OpenGLWidget::timerEvent(QTimerEvent *event)
{
	//update();
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
	lastX = event->position().x();
	lastY = event->position().y();

	event->accept();
}

void OpenGLWidget::keyPressEvent(QKeyEvent *event)
{
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

QMatrix4x4 OpenGLWidget::GetViewMatrix()
{
	QMatrix4x4 look_at;
	look_at.setToIdentity();
	look_at.lookAt(Position, Position + Front, Up);
	return look_at;
}

