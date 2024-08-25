#include <QOpenGLTexture>
#include <QDir>
#include "model.h"

static QOpenGLTexture *TextureFromFile(const char *path, const QString &directory);

Model::~Model()
{
	for(Texture &texture: textures_loaded) {
		delete texture.id;
	}
}

void Model::loadModel(QString path)
{
	Assimp::Importer import;
	const aiScene *scene = import.ReadFile(path.toLocal8Bit().constData(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		qDebug() << "ERROR::ASSIMP::" << import.GetErrorString();
		return;
	}
	QDir dir(path);
	dir.cdUp();
	;	directory = dir.path();

	processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, const aiScene *scene)
{
	// process all the node's meshes (if any)
	for(unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.append(processMesh(mesh, scene));
	}
	// then do the same for each of its children
	for(unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
	// data to fill
	QVector<Vertex> vertices;
	QVector<unsigned int> indices;
	QVector<Texture> textures;

	// walk through each of the mesh's vertices
	for(unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		QVector3D vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
		// positions
		vector[0] = mesh->mVertices[i].x;
		vector[1] = mesh->mVertices[i].y;
		vector[2] = mesh->mVertices[i].z;
		vertex.Position = vector;
		// normals
		if (mesh->HasNormals())
		{
			vector[0] = mesh->mNormals[i].x;
			vector[1] = mesh->mNormals[i].y;
			vector[2] = mesh->mNormals[i].z;
			vertex.Normal = vector;
		}
		// texture coordinates
		if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			QVector2D vec;
			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vec[0] = mesh->mTextureCoords[0][i].x;
			vec[1] = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
			// tangent
			vector[0] = mesh->mTangents[i].x;
			vector[1] = mesh->mTangents[i].y;
			vector[2] = mesh->mTangents[i].z;
			vertex.Tangent = vector;
			// bitangent
			vector[0] = mesh->mBitangents[i].x;
			vector[1] = mesh->mBitangents[i].y;
			vector[2] = mesh->mBitangents[i].z;
			vertex.Bitangent = vector;
		}
		else
			vertex.TexCoords = QVector2D(0.0f, 0.0f);

		vertices.push_back(vertex);
	}
	// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
	for(unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		// retrieve all indices of the face and store them in the indices vector
		for(unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
	// process materials
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	// we assume a convention for sampler names in the shaders. Each diffuse texture should be named
	// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
	// Same applies to other texture as the following list summarizes:
	// diffuse: texture_diffuseN
	// specular: texture_specularN
	// normal: texture_normalN

	// 1. diffuse maps
	QVector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
	textures.append(diffuseMaps);
	// // 2. specular maps
	// QVector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
	// textures.append(specularMaps);
	// // 3. normal maps
	// QVector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
	// textures.append(normalMaps);
	// // 4. height maps
	// QVector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
	// textures.append(heightMaps);

	// return a mesh object created from the extracted mesh data
	return Mesh(vertices, indices, textures);
}


QVector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, QString typeName)
{
	QVector<Texture> textures;
	for(unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;
		mat->GetTexture(type, i, &str);
		bool skip = false;
		for(unsigned int j = 0; j < textures_loaded.size(); j++) {
			if(textures_loaded[j].path == str.C_Str()) {
				textures.push_back(textures_loaded[j]);
				skip = true;
				break;
			}
		}
		if(!skip) {   // if texture hasn't been loaded already, load it
			Texture texture;
			texture.id = TextureFromFile(str.C_Str(), directory);
			texture.type = typeName;
			texture.path = str.C_Str();
			textures.push_back(texture);
			textures_loaded.push_back(texture); // add to loaded textures
		}
	}
	return textures;
}

QOpenGLTexture *TextureFromFile(const char *path, const QString &directory)
{
	QString filename = path;
	filename = directory + '/' + filename;

	QOpenGLTexture *texture = nullptr;
	qDebug() << "loading texture file:" << filename;
	QImage data(filename);
	if(!data.isNull()) {
		texture = new QOpenGLTexture(data.mirrored());
		texture->bind();
		texture->setWrapMode(QOpenGLTexture::DirectionS, QOpenGLTexture::Repeat);
		texture->setWrapMode(QOpenGLTexture::DirectionT, QOpenGLTexture::Repeat);
		texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
		texture->setMagnificationFilter(QOpenGLTexture::Linear);
		return texture;
	} else {
		qDebug() << "Texture failed to load at path:" << path;
		return nullptr;
	}
}
