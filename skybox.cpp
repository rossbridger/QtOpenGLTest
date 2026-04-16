#include "skybox.h"

static const float skyboxVertices[] = {
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

Skybox::Skybox(QOpenGLContext* context): RenderObject(context)
{
	setShader("skybox.vs", "skybox.fs");
	std::array<const char*, 6> faces = {
		"right.jpg",
		"left.jpg",
		"top.jpg",
		"bottom.jpg",
		"front.jpg",
		"back.jpg"
	};
	loadCubemap(faces);
	shader->bind();
	vao.create();
	vao.bind();
	vbo.create();
	vbo.bind();
	vbo.allocate(skyboxVertices, sizeof(skyboxVertices));
	shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 3);
	shader->enableAttributeArray(0);
	shader->setUniformValue("skybox", 0);
	shader->release();
	vbo.release();
	vao.release();

}

void Skybox::onDraw()
{
	glDepthMask(GL_FALSE);
	shader->bind();
	shader->setUniformValue("projection", projectionMatrix);
	shader->setUniformValue("view", viewMatrix);

	vao.bind();
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthMask(GL_TRUE);
}

void Skybox::loadCubemap(const std::array<const char*, 6>& faces)
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
