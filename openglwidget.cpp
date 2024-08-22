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
	VBO.destroy();
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

	VBO = QOpenGLBuffer();
	VBO.create();
	VBO.bind();
	VBO.allocate(vertices, sizeof(vertices));

	lightingShader->bind();
	lightingShader->setAttributeBuffer(0, GL_FLOAT, 0, 3, 8 * sizeof(float));
	lightingShader->enableAttributeArray(0);
	lightingShader->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 3, 8 * sizeof(float));
	lightingShader->enableAttributeArray(1);
	lightingShader->setAttributeBuffer(2, GL_FLOAT, 6 * sizeof(float), 2, 8 * sizeof(float));
	lightingShader->enableAttributeArray(2);
	lightCubeShader->bind();
	lightCubeShader->setAttributeBuffer(0, GL_FLOAT, 0, 3, 8 * sizeof(float));
	lightCubeShader->enableAttributeArray(0);

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

	QVector3D pointLightPositions[] = {
		QVector3D(0.7f,  0.2f,  2.0f),
		QVector3D(2.3f, -3.3f, -4.0f),
		QVector3D(-4.0f,  2.0f, -12.0f),
		QVector3D(0.0f,  0.0f, -3.0f)
	};

	QMatrix4x4 model, view, projection;
	view.setToIdentity();
	projection.setToIdentity();

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	lightingShader->bind();
	lightingShader->setUniformValue("material.diffuse", 0);
	lightingShader->setUniformValue("material.specular", 1);

	lightingShader->setUniformValue("material.shininess", 32.0f);

	lightingShader->setUniformValue("dirLight.direction", -0.2f, -1.0f, -0.3f);
	lightingShader->setUniformValue("dirLight.ambient", 0.05f, 0.05f, 0.05f); // darken diffuse light a bit
	lightingShader->setUniformValue("dirLight.specular", 0.5f, 0.5f, 0.5f);
	// point light 1
	lightingShader->setUniformValue("pointLights[0].position", pointLightPositions[0]);
	lightingShader->setUniformValue("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
	lightingShader->setUniformValue("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
	lightingShader->setUniformValue("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
	lightingShader->setUniformValue("pointLights[0].constant", 1.0f);
	lightingShader->setUniformValue("pointLights[0].linear", 0.09f);
	lightingShader->setUniformValue("pointLights[0].quadratic", 0.032f);
	// point light 2
	lightingShader->setUniformValue("pointLights[1].position", pointLightPositions[1]);
	lightingShader->setUniformValue("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
	lightingShader->setUniformValue("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
	lightingShader->setUniformValue("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
	lightingShader->setUniformValue("pointLights[1].constant", 1.0f);
	lightingShader->setUniformValue("pointLights[1].linear", 0.09f);
	lightingShader->setUniformValue("pointLights[1].quadratic", 0.032f);
	// point light 3
	lightingShader->setUniformValue("pointLights[2].position", pointLightPositions[2]);
	lightingShader->setUniformValue("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
	lightingShader->setUniformValue("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
	lightingShader->setUniformValue("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
	lightingShader->setUniformValue("pointLights[2].constant", 1.0f);
	lightingShader->setUniformValue("pointLights[2].linear", 0.09f);
	lightingShader->setUniformValue("pointLights[2].quadratic", 0.032f);
	// point light 4
	lightingShader->setUniformValue("pointLights[3].position", pointLightPositions[3]);
	lightingShader->setUniformValue("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
	lightingShader->setUniformValue("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
	lightingShader->setUniformValue("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
	lightingShader->setUniformValue("pointLights[3].constant", 1.0f);
	lightingShader->setUniformValue("pointLights[3].linear", 0.09f);
	lightingShader->setUniformValue("pointLights[3].quadratic", 0.032f);
	// spotLight
	lightingShader->setUniformValue("spotLight.position", Position);
	lightingShader->setUniformValue("spotLight.direction", Front);
	lightingShader->setUniformValue("spotLight.ambient", 0.0f, 0.0f, 0.0f);
	lightingShader->setUniformValue("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
	lightingShader->setUniformValue("spotLight.specular", 1.0f, 1.0f, 1.0f);
	lightingShader->setUniformValue("spotLight.constant", 1.0f);
	lightingShader->setUniformValue("spotLight.linear", 0.09f);
	lightingShader->setUniformValue("spotLight.quadratic", 0.032f);
	lightingShader->setUniformValue("spotLight.cutOff", cos(qDegreesToRadians(12.5f)));
	lightingShader->setUniformValue("spotLight.outerCutOff", cos(qDegreesToRadians(15.0f)));

	projection.setToIdentity();
	projection.perspective(Zoom, width()/height(), 0.1f, 100.0f);
	view = GetViewMatrix();

	lightingShader->setUniformValue("projection", projection);
	lightingShader->setUniformValue("view", view);
	lightingShader->setUniformValue("viewPos", Position);

	diffuseMap->bind(0);
	specularMap->bind(1);

	// render the cube
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
	for(unsigned int i = 0; i < 4; i++) {
		model.setToIdentity();
		model.translate(pointLightPositions[i]);
		model.scale(0.2f);
		lightCubeShader->setUniformValue("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

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

