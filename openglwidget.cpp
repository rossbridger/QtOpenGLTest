#include <QOpenGLShader>
#include "openglwidget.h"


OpenGLWidget::OpenGLWidget(QWidget *parent): QOpenGLWidget(parent), QOpenGLExtraFunctions(context())
{
	m_program = new QOpenGLShaderProgram(context());
	timer.start();
	startTimer(50);
}

OpenGLWidget::~OpenGLWidget()
{
	delete texture[0];
	delete texture[1];
}

void OpenGLWidget::initializeGL()
{
	initializeOpenGLFunctions();
	assert(m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, QString("vertex.vert")));
	assert(m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, QString("fragment.frag")));
	m_program->link();

	float vertices[] = {
		// positions          // colors           // texture coords
		 0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
		 0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	//glGenTextures(1, &texture);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

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
}

void OpenGLWidget::paintGL()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	m_program->bind();
	m_program->setUniformValue("texture1", 0);
	m_program->setUniformValue("texture2", 1);

	glActiveTexture(GL_TEXTURE0);
	texture[0]->bind();
	glActiveTexture(GL_TEXTURE1);
	texture[1]->bind();
	// glBindTexture(GL_TEXTURE_2D, texture);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void OpenGLWidget::resizeGL(int w, int h)
{
}

void OpenGLWidget::timerEvent(QTimerEvent *event)
{
	update();
}
