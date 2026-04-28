#ifndef MODEL_H
#define MODEL_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <QString>
#include <QOpenGLShaderProgram>
#include <QOpenGLExtraFunctions>
#include "entity.h"
#include "mesh.h"
class QOpenGLTexture;

class Model: public Entity
{
	Q_OBJECT
public:
	Model(QOpenGLContext* context, QString path);
	~Model();
	void draw(QOpenGLShaderProgram* shader) override;
	friend class OpenGLWidget;
private:
	QVector<Mesh> meshes;
	QString directory;
	QVector<Texture> textures_loaded;
	void loadModel(QString path);
	void processNode(aiNode *node, const aiScene *scene);
	Mesh processMesh(aiMesh *mesh, const aiScene *scene);
	QVector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, QString typeName);
	void setupMesh(Mesh& mesh);
	void drawMesh(QOpenGLShaderProgram* shader, Mesh& mesh);
};

#endif // MODEL_H
