#include <QOpenGLShader>
#include <QKeyEvent>
#include <QVector4D>
#include <QMatrix4x4>
#include <QOpenGLFramebufferObjectFormat>
#include "openglwidget.h"
#include "model.h"
#include "terrain.h"


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

const float skyboxVertices[] = {
    // positions
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

OpenGLWidget::OpenGLWidget(QWidget *parent): QOpenGLWidget(parent),
	QOpenGLExtraFunctions(context()),
	screen_vbo(QOpenGLBuffer::VertexBuffer),
	Front(0.0f, 0.0f, -1.0f),
	MovementSpeed(SPEED),
	MouseSensitivity(SENSITIVITY),
	Zoom(ZOOM),
	near_plane(0.1f),
	far_plane(100.0f)
{
	constexpr int fps = 60;
	timer.start();
	startTimer(1000/fps);

	Position = QVector3D(0.0f, 0.0f, 3.0f);
	WorldUp = QVector3D(0.0f, 1.0f, 0.0f);

	Yaw = YAW;
	Pitch = PITCH;
	lastX = width()/2.0f;
	lastY = height()/2.0f;
	updateCameraVectors();
	camera.setPosition(Position);
	camera.setFov(Zoom);
	camera.setAspectRatio(float(width())/height());
	camera.setNearPlane(near_plane);
	camera.setFarPlane(far_plane);
	grabKeyboard(); // so that it receives keyboard event
}

OpenGLWidget::~OpenGLWidget()
{
	for (Entity* entity: entities) {
		delete entity;
	}
	glDeleteFramebuffers(1, &fbo);
}

void OpenGLWidget::setSunAltitude(float altitude)
{
	sun_altitude = altitude;
}

void OpenGLWidget::setSunAzimuth(float azimuth)
{
	sun_azimuth = azimuth;
}

void OpenGLWidget::initializeGL()
{
	initializeOpenGLFunctions();

	// initialize fbo
	glGenFramebuffers(1, &fbo);
	// generate texture
	glGenTextures(1, &screen_texture);
	glGenTextures(1, &depth_texture);
	resizeFramebufferTextures();

	// start binding
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glBindTexture(GL_TEXTURE_2D, screen_texture);
	// attach it to currently bound framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screen_texture, 0);

	// now depth texture
	glBindTexture(GL_TEXTURE_2D, depth_texture);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	initializeSkybox();
	initalizeMesh();
	initializePostProcessing();
}

void OpenGLWidget::paintGL()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	if (windowResized) { // recreate image object
		resizeFramebufferTextures();
		windowResized = false;
	}

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	meshPass();
	skyboxPass();
	postProcessingPass();

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
	camera.setPosition(Position);
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

	camera.setFov(Zoom);
}

void OpenGLWidget::resizeGL(int w, int h)
{
	camera.setAspectRatio(float(width())/height());
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
	camera.setOrientation(Front, Right, Up);
}

void OpenGLWidget::resizeFramebufferTextures()
{
	glBindTexture(GL_TEXTURE_2D, screen_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width(), height(), 0,
		     GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, depth_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width(), height(), 0,
		     GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLWidget::initializeSkybox()
{
	skybox_shader = new QOpenGLShaderProgram(context());
	assert(skybox_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, "skybox.vs"));
	assert(skybox_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, "skybox.fs"));
	skybox_shader->link();
	const std::array<const char*, 6> faces = {
		"right.jpg",
		"left.jpg",
		"top.jpg",
		"bottom.jpg",
		"front.jpg",
		"back.jpg"
	};
	loadCubemap(faces);
	skybox_shader->bind();
	skybox_vao.create();
	skybox_vao.bind();
	skybox_vbo.create();
	skybox_vbo.bind();
	skybox_vbo.allocate(skyboxVertices, sizeof(skyboxVertices));
	skybox_shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 3);
	skybox_shader->enableAttributeArray(0);
	skybox_shader->setUniformValue("skybox", 0);
	skybox_shader->release();
	skybox_vbo.release();
	skybox_vao.release();
}

void OpenGLWidget::initializePostProcessing()
{
	screen_shader = new QOpenGLShaderProgram(context());
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
	screen_shader->setUniformValue("depthTexture", 1);
	screen_shader->release();
	screen_vbo.release();
	screen_vao.release();
}

void OpenGLWidget::initalizeMesh()
{
	mesh_shader = new QOpenGLShaderProgram(context());
	assert(mesh_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, "model.vs"));
	assert(mesh_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, "model.fs"));
	mesh_shader->link();

	entities.push_back(new Model(context(), "backpack/backpack.obj"));
	entities.push_back(new Terrain(context(), "iceland_heightmap.png"));
}

void OpenGLWidget::meshPass()
{
	mesh_shader->bind();
	mesh_shader->setUniformValue("projection", camera.getProjectionMatrix());
	mesh_shader->setUniformValue("view", camera.getViewMatrix());
	mesh_shader->setUniformValue("viewPos", camera.getPosition());
	mesh_shader->setUniformValue("dirLight.direction", QVector3D(-0.2f, -1.0f, -0.3f));
	mesh_shader->setUniformValue("dirLight.ambient", QVector3D(0.2f, 0.2f, 0.2f));
	mesh_shader->setUniformValue("dirLight.diffuse", QVector3D(0.5f, 0.5f, 0.5f)); // darken diffuse light a bit
	mesh_shader->setUniformValue("dirLight.specular", QVector3D(1.0f, 1.0f, 1.0f));
	QVector3D pointLightPositions[] = {
		QVector3D( 0.7f,  0.2f,  2.0f),
		QVector3D( 2.3f, -3.3f, -4.0f),
		QVector3D(-4.0f,  2.0f, -12.0f),
		QVector3D( 0.0f,  0.0f, -3.0f)
	};

	for (int i = 0; i < 4; i++) {
		mesh_shader->setUniformValue(QString("pointLights[%1].position").arg(i).toUtf8(), pointLightPositions[i]);
		mesh_shader->setUniformValue(QString("pointLights[%1].constant").arg(i).toUtf8(), 1.0f);
		mesh_shader->setUniformValue(QString("pointLights[%1].linear").arg(i).toUtf8(), 0.09f);
		mesh_shader->setUniformValue(QString("pointLights[%1].quadratic").arg(i).toUtf8(), 0.032f);
		mesh_shader->setUniformValue(QString("pointLights[%1].ambient").arg(i).toUtf8(), QVector3D(0.2f, 0.2f, 0.2f));
		mesh_shader->setUniformValue(QString("pointLights[%1].diffuse").arg(i).toUtf8(), QVector3D(0.5f, 0.5f, 0.5f));
		mesh_shader->setUniformValue(QString("pointLights[%1].specular").arg(i).toUtf8(), QVector3D(1.0f, 1.0f, 1.0f));
	}

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	for (Entity* entity: entities) {
		entity->draw(mesh_shader);
	}
}

void OpenGLWidget::skyboxPass()
{
	// needs to remove the "translation" part of the skybox view matrix
	QMatrix4x4 skyboxViewMatrix = camera.getViewMatrix();
	skyboxViewMatrix.setColumn(3, QVector4D(0, 0, 0, 1));

	glDepthMask(GL_FALSE);
	skybox_shader->bind();
	skybox_shader->setUniformValue("projection", camera.getProjectionMatrix());
	skybox_shader->setUniformValue("view", skyboxViewMatrix);

	skybox_vao.bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthMask(GL_TRUE);
}

void OpenGLWidget::postProcessingPass()
{
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, screen_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depth_texture);
	glEnable(GL_FRAMEBUFFER_SRGB);
	screen_shader->bind();
	screen_vao.bind();
	screen_shader->setUniformValue("near_plane", near_plane);
	screen_shader->setUniformValue("far_plane", far_plane);

	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void OpenGLWidget::loadCubemap(const std::array<const char*, 6>& faces)
{
	glGenTextures(1, &cubeTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexture);
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		QString filename = faces[i];
		QImage image(filename);
		qDebug() << "image " << i << ": " << image.width() << "x" << image.height() << ", format = " << image.format();
		image.convertTo(QImage::Format_RGB888);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, image.width(), image.height(),
			     0, GL_RGB, GL_UNSIGNED_BYTE, image.bits());
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}
