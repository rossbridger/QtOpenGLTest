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
}

void OpenGLWidget::initializeGL()
{
	initializeOpenGLFunctions();
	assert(m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, QString("vertex.vert")));
	assert(m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, QString("fragment.frag")));
	m_program->link();

	const float vertices[] = {
		 0.5f,  0.5f, 0.0f,  // top right
		 0.5f, -0.5f, 0.0f,  // bottom right
		-0.5f, -0.5f, 0.0f,  // bottom left
		-0.5f,  0.5f, 0.0f   // top left
	};
	const unsigned int indices[] = {  // note that we start from 0!
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
}

void OpenGLWidget::paintGL()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	m_program->bind();

	// update the uniform color
	float timeValue = timer.elapsed() / 1000.0f;
	float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
	int vertexColorLocation = glGetUniformLocation(m_program->programId(), "ourColor");
	glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);
	qDebug() << "greenValue" << greenValue << "vertexColorLocation" << vertexColorLocation;
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
