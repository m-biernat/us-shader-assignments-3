#ifndef __MESH_H__
#define __MESH_H__

#include <glm/glm.hpp>
#include <GL/glcorearb.h>
#include <assimp/Scene.h>

class Mesh
{
	static constexpr int VBOS = 3;

	GLuint vao;
	GLuint vbo[VBOS];
	unsigned int numTriangles;
	glm::vec3 bbMin;
	glm::vec3 bbMax;

public:
	Mesh();
	Mesh(const aiMesh *mesh, GLint vertexLoc = 0, GLint normalLoc = 1);
	~Mesh();

	void draw() const;
	glm::vec3 getBBmin() const;
	glm::vec3 getBBmax() const;
};

#endif /*__MESH_H__*/