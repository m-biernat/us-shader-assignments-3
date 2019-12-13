#ifndef __MODEL_H__
#define __MODEL_H__

#include <glm\glm.hpp>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/PostProcess.h>
#include <assimp/Scene.h>

#include "mesh.h"

class Model
{
	std::vector<Mesh*> meshes;
	glm::vec3 bbMin;
	glm::vec3 bbMax;
	glm::vec3 centroid;

public:
	Model();
	Model(const std::string& filename, GLint vertexLoc = 0, GLint normalLoc = 1);
	Model(const aiScene *scene, GLint vertexLoc = 0, GLint normalLoc = 1);
	~Model();

	void draw() const;
	Mesh* getMesh(unsigned int n) const;
	unsigned int getNumberOfMeshes() const;
	glm::vec3 getBBmin() const;
	glm::vec3 getBBmax() const;
	glm::vec3 getCentroid() const;

private:
	bool importModelFromFile( const std::string& filename, Assimp::Importer& importer, const aiScene **scene );
	void modelFromScene(const aiScene *scene, GLint vertexLoc, GLint normalLoc);
	void computeBoundingBox();
	void computeCentroid(const aiScene *scene);
};

#endif /*__MODEL_H__*/