#ifndef TERRAIN_H
#define TERRAIN_H

#include "entity.h"
#include <qimage.h>

class Terrain : public Entity
{
	Q_OBJECT
public:
	Terrain(QOpenGLContext* context, QString path);
	void draw(QOpenGLShaderProgram* shader) override;
private:
	void generateBuffers(QImage heightmap);

	unsigned int num_strips;
	unsigned int num_verts_per_strip;
	GLuint terrainVAO, terrainVBO, terrainEBO;
	GLuint terrainTexture;
};

#endif // TERRAIN_H
