#include <QOpenGLShader>
#include <QKeyEvent>
#include <QVector4D>
#include <QMatrix4x4>
#include "openglwidget.h"
#include "model.h"


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
	shader = new QOpenGLShaderProgram(context());
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
}

void OpenGLWidget::initializeGL()
{
	initializeOpenGLFunctions();
	glEnable(GL_DEPTH_TEST);

	assert(shader->addShaderFromSourceFile(QOpenGLShader::Vertex, QString("model.vs")));
	assert(shader->addShaderFromSourceFile(QOpenGLShader::Fragment, QString("model.fs")));
	shader->link();
	model = new Model("backpack/backpack.obj");
	setupModel();
}

void OpenGLWidget::paintGL()
{
	shader->bind();
	QMatrix4x4 model, view, projection;
	view.setToIdentity();
	projection.setToIdentity();

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	projection.perspective(Zoom, width()/height(), 0.1f, 100.0f);
	view = GetViewMatrix();
	shader->setUniformValue("projection", projection);
	shader->setUniformValue("view", view);

	model.setToIdentity();
	model.translate(QVector3D(0.0f, 0.0f, 0.0f));
	model.scale(QVector3D(1.0f, 1.0f, 1.0f));
	shader->setUniformValue("model", model);

	drawModel();
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

void OpenGLWidget::setupMesh(Mesh &mesh)
{
	mesh.VAO = new QOpenGLVertexArrayObject(this);
	mesh.VAO->create();
	mesh.VAO->bind();
	mesh.VBO = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	mesh.VBO.create();
	mesh.VBO.setUsagePattern(QOpenGLBuffer::StaticDraw);
	mesh.EBO = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
	mesh.EBO.create();
	mesh.EBO.setUsagePattern(QOpenGLBuffer::StaticDraw);

	mesh.VBO.bind();
	mesh.VBO.allocate(mesh.vertices.constData(), (int)(mesh.vertices.size() * sizeof(Vertex)));
	mesh.EBO.bind();
	mesh.EBO.allocate(mesh.indices.constData(), (int)(mesh.indices.size() * sizeof(unsigned int)));

	shader->enableAttributeArray(0);
	shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(Vertex));
	shader->enableAttributeArray(1);
	shader->setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex, Normal), 3, sizeof(Vertex));
	shader->enableAttributeArray(2);
	shader->setAttributeBuffer(2, GL_FLOAT, offsetof(Vertex, TexCoords), 2, sizeof(Vertex));
	shader->enableAttributeArray(3);
	shader->setAttributeBuffer(3, GL_FLOAT, offsetof(Vertex, Tangent), 3, sizeof(Vertex));
	shader->enableAttributeArray(4);
	shader->setAttributeBuffer(4, GL_FLOAT, offsetof(Vertex, Bitangent), 3, sizeof(Vertex));
	shader->enableAttributeArray(5);
	shader->setAttributeBuffer(5, GL_INT, offsetof(Vertex, m_BoneIDs), 3, sizeof(Vertex));
	shader->enableAttributeArray(6);
	shader->setAttributeBuffer(6, GL_FLOAT, offsetof(Vertex, m_Weights), 4, sizeof(Vertex));
	mesh.VAO->release();
}

void OpenGLWidget::setupModel()
{
	for(unsigned int i = 0; i < model->meshes.length(); i++) {
		setupMesh(model->meshes[i]);
	}
}

void OpenGLWidget::drawMesh(Mesh &mesh)
{
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	unsigned int normalNr   = 1;
        unsigned int heightNr   = 1;
	for(unsigned int i = 0; i < mesh.textures.size(); i++) {
		QString number;
		QString name = mesh.textures[i].type;
		if(name == "texture_diffuse") {
			number = QString::number(diffuseNr++);
		// } else if(name == "texture_specular") {
		// 	number = QString::number(specularNr++);
		// } else if(name == "texture_normal") {
		// 	number = QString::number(normalNr++);
		// } else if(name == "texture_height") {
		// 	number = QString::number(heightNr++);
		}
		shader->setUniformValue((name + number).toLocal8Bit().constData(), i);
		if(mesh.textures[i].id) {
			mesh.textures[i].id->bind();
		}
	}
	mesh.VAO->bind();
	glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
	mesh.VAO->release();
}

void OpenGLWidget::drawModel()
{
	for(unsigned int i = 0; i < model->meshes.length(); i++) {
		drawMesh(model->meshes[i]);
	}
}

