#include <QOpenGLShader>
#include <QKeyEvent>
#include <QVector4D>
#include <QMatrix4x4>
#include "openglwidget.h"


OpenGLWidget::OpenGLWidget(QWidget *parent): QOpenGLWidget(parent), QOpenGLExtraFunctions(context())
{
	constexpr int fps = 60;
	program = new QOpenGLShaderProgram(context());
	timer.start();
	startTimer(1000/fps);
	is_cooldown = false;
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

	texture[0] = new QOpenGLTexture(QImage("container.jpg").convertToFormat(QImage::Format_RGB888).mirrored());
	texture[0]->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
	texture[0]->setMagnificationFilter(QOpenGLTexture::Linear);

	texture[1] = new QOpenGLTexture(QImage("awesomeface.png").convertToFormat(QImage::Format_RGB888).mirrored());
	texture[1]->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
	texture[1]->setMagnificationFilter(QOpenGLTexture::Linear);
	glEnable(GL_DEPTH_TEST);
}

void OpenGLWidget::paintGL()
{
	QMatrix4x4 model, view, projection;
	model.setToIdentity();
	view.setToIdentity();
	projection.setToIdentity();
	model.rotate(qRadiansToDegrees(timer.elapsed()/1000.0f), 0.5f, 1.0f, 0.0f);
	view.translate(0.0f, 0.0f, -3.0f);
	projection.perspective(45.0f, 800.0f / 600.0f, 0.1f, 100.0f);


	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	program->bind();
	program->setUniformValue("texture1", 0);
	program->setUniformValue("texture2", 1);
	program->setUniformValue("model", model);
	program->setUniformValue("view", view);
	program->setUniformValue("projection", projection);

	glActiveTexture(GL_TEXTURE0);
	texture[0]->bind();
	glActiveTexture(GL_TEXTURE1);
	texture[1]->bind();
	// glBindTexture(GL_TEXTURE_2D, texture);
	glBindVertexArray(VAO);
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

void OpenGLWidget::resizeGL(int w, int h)
{
}

void OpenGLWidget::timerEvent(QTimerEvent *event)
{
	if(is_cooldown) {
		is_cooldown = false;
	}
	update();
}
