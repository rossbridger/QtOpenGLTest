#ifndef MESH_H
#define MESH_H
#include <QVector>
#include <QVector3D>
#include <QVector2D>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>

#define MAX_BONE_INFLUENCE 4

struct Vertex {
	QVector3D Position;
	QVector3D Normal;
	QVector2D TexCoords;
	QVector3D Tangent;
	// bitangent
	QVector3D Bitangent;
	//bone indexes which will influence this vertex
	int m_BoneIDs[MAX_BONE_INFLUENCE];
	//weights from each bone
	float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture {
	QOpenGLTexture *id;
	QString type;
	QString path;  // we store the path of the texture to compare with other textures
};

class Mesh {
public:
	QVector<Vertex> vertices;
	QVector<unsigned int> indices;
	QVector<Texture> textures;
	QOpenGLVertexArrayObject *VAO;
	QOpenGLBuffer VBO, EBO;
	Mesh(const QVector<Vertex> vertices, const QVector<unsigned int> indices, const QVector<Texture> textures);
};

#endif // MESH_H
