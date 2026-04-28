#include "terrain.h"


Terrain::Terrain(QOpenGLContext* context, QString path)
	: Entity(context)
{
	QImage heightmap = QImage(path);
	generateBuffers(heightmap);
}

void Terrain::draw(QOpenGLShaderProgram* shader)
{
	// draw mesh
	shader->setUniformValue("model", modelMatrix);
	shader->setUniformValue("texture_diffuse1", 0);
	glBindVertexArray(terrainVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, terrainTexture);
	// render the mesh triangle strip by triangle strip - each row at a time
	for(unsigned int strip = 0; strip < num_strips; ++strip)
	{
		glDrawElements(GL_TRIANGLE_STRIP,   // primitive type
			       num_verts_per_strip, // number of indices to render
			       GL_UNSIGNED_INT,     // index data type
			       (void*)(sizeof(unsigned int)
				       * num_verts_per_strip
				       * strip)); // offset to starting index
	}
}

void Terrain::generateBuffers(QImage heightmap)
{
	// vertex generation
	int width, height;
	QVector<float> vertices;
	QVector<float> indices;
	width = heightmap.width();
	height = heightmap.height();

	qDebug() << "Heightmap " << width << "x" << height;

	heightmap.convertTo(QImage::Format_RGB888);
	float yScale = 64.0f / 256.0f, yShift = 32.0f;  // apply a scale+shift to the height data
	for(unsigned int i = 0; i < height; i++)
	{
		for(unsigned int j = 0; j < width; j++)
		{
			QColor texel = heightmap.pixelColor(j, i);
			// raw height at coordinate
			unsigned char y = texel.red();
			// vertex
			vertices.push_back( -height/2.0f + i );        // v.x
			vertices.push_back( (int)y * yScale - yShift); // v.y
			vertices.push_back( -width/2.0f + j );        // v.z
			vertices.push_back(0); // normal
			vertices.push_back(1);
			vertices.push_back(0);

			vertices.push_back(float(j) / width); // uv
			vertices.push_back(float(i) / height);
		}
	}

	// index generation
	for(unsigned int i = 0; i < height-1; i++)       // for each row a.k.a. each strip
	{
	    for(unsigned int j = 0; j < width; j++)      // for each column
	    {
		for(unsigned int k = 0; k < 2; k++)      // for each side of the strip
		{
		    indices.push_back(j + width * (i + k));
		}
	    }
	}

	qDebug() << "Generated " << vertices.size() << " vertices and " << indices.size() << " indices";

	num_strips = height-1;
	num_verts_per_strip = width*2;
	// register VAO
	glGenVertexArrays(1, &terrainVAO);
	glBindVertexArray(terrainVAO);

	glGenBuffers(1, &terrainVBO);
	glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
	glBufferData(GL_ARRAY_BUFFER,
		     vertices.size() * sizeof(float),       // size of vertices buffer
		     vertices.data(),                     // pointer to first element
		     GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); // position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // normal
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); // uv
	glEnableVertexAttribArray(2);

	glGenBuffers(1, &terrainEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		     indices.size() * sizeof(unsigned int), // size of indices buffer
		     indices.data(),                        // pointer to first element
		     GL_STATIC_DRAW);
	// for debugging purpose
	glGenTextures(1, &terrainTexture);
	glBindTexture(GL_TEXTURE_2D, terrainTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,
		     0, GL_RGB, GL_UNSIGNED_BYTE, heightmap.bits());
}
