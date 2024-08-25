#ifndef MODEL_H
#define MODEL_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <QString>
#include <QOpenGLShaderProgram>
#include "mesh.h"

class QOpenGLTexture;

class Model
{
public:
	Model(QString path) { loadModel(path); }
	~Model();
	friend class OpenGLWidget;
private:
	QVector<Mesh> meshes;
	QString directory;
	QVector<Texture> textures_loaded;
	void loadModel(QString path);
	void processNode(aiNode *node, const aiScene *scene);
	Mesh processMesh(aiMesh *mesh, const aiScene *scene);
	QVector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, QString typeName);
};

#endif // MODEL_H
