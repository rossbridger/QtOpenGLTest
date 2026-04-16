#ifndef MODEL_H
#define MODEL_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <QString>
#include <QOpenGLShaderProgram>
#include "mesh.h"
#include "renderobject.h"

class QOpenGLTexture;

class Model: public RenderObject
{
	Q_OBJECT
public:
	Model(QOpenGLContext* context, QString path);
	~Model();
	void setModelMatrix(QMatrix4x4 modelMatrix) {this->modelMatrix = modelMatrix;}
	void setviewMatrix(QMatrix4x4 viewMatrix) {this->viewMatrix = viewMatrix;}
	void setProjectMatrix(QMatrix4x4 projectionMatrix) {this->projectionMatrix = projectionMatrix;}
	void onDraw() override;
	friend class OpenGLWidget;
private:
	QVector<Mesh> meshes;
	QString directory;
	QVector<Texture> textures_loaded;
	QMatrix4x4 modelMatrix, viewMatrix, projectionMatrix;
	void loadModel(QString path);
	void processNode(aiNode *node, const aiScene *scene);
	Mesh processMesh(aiMesh *mesh, const aiScene *scene);
	QVector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, QString typeName);
	void setupMesh(Mesh& mesh);
	void drawMesh(Mesh& mesh);
};

#endif // MODEL_H
