#include "mesh.h"

Mesh::Mesh(const QVector<Vertex> vertices, const QVector<unsigned int> indices, const QVector<Texture> textures)
{
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;
}
