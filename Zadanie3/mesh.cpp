#include <iostream>
#include <limits>
#include <GL/glew.h>

#include "mesh.h"

Mesh::Mesh()
{
	vao = 0;
	for( int i = 0; i < VBOS; ++i )
		vbo[i] = 0;

	numTriangles = 0;
	bbMin = glm::vec3( 0.0 );
	bbMax = glm::vec3( 0.0 );
}

Mesh::Mesh(const aiMesh *mesh, GLint vertexLoc, GLint normalLoc)
{
	numTriangles = mesh->mNumFaces;

	unsigned int *indices = new unsigned int[ mesh->mNumFaces * 3 ];
	unsigned int k = 0;
	for( unsigned int i = 0; i < mesh->mNumFaces; ++i )
	{
		const aiFace *face = &mesh->mFaces[i];

		for( unsigned int j = 0; j < 3; ++j )
		{
			indices[k] = face->mIndices[j];
			++k;
		}
	}

	bbMin = glm::vec3( (std::numeric_limits<float>::max)() );
	bbMax = glm::vec3( (std::numeric_limits<float>::min)() );

	GLfloat *vertices = new GLfloat[ mesh->mNumVertices * 4 ];
	k = 0;
	for( unsigned int i = 0; i < mesh->mNumVertices; ++i )
	{
		const aiVector3D *vertex = &mesh->mVertices[i];

		vertices[k++] = vertex->x;
		vertices[k++] = vertex->y;
		vertices[k++] = vertex->z;
		vertices[k++] = 1.0f;

		if( vertex->x < bbMin.x )
			bbMin.x = vertex->x;
		if( vertex->y < bbMin.y )
			bbMin.y = vertex->y;
		if( vertex->z < bbMin.z )
			bbMin.z = vertex->z;

		if( vertex->x > bbMax.x )
			bbMax.x = vertex->x;
		if( vertex->y > bbMax.y )
			bbMax.y = vertex->y;
		if( vertex->z > bbMax.z )
			bbMax.z = vertex->z;
	}

	GLfloat *normals = new GLfloat[ mesh->mNumVertices * 3 ];
	k = 0;
	for( unsigned int i = 0; i < mesh->mNumVertices; ++i )
	{
		const aiVector3D *normal = &mesh->mNormals[i];

		normals[k++] = normal->x;
		normals[k++] = normal->y;
		normals[k++] = normal->z;
	}

	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );

    glGenBuffers( VBOS, vbo );

	glBindBuffer( GL_ARRAY_BUFFER, vbo[0] );
	glBufferData( GL_ARRAY_BUFFER, 4 * sizeof( GLfloat ) * mesh->mNumVertices, vertices, GL_STATIC_DRAW );
	glEnableVertexAttribArray( vertexLoc );
    glVertexAttribPointer( vertexLoc, 4, GL_FLOAT, GL_FALSE, 0, 0 );

	glBindBuffer( GL_ARRAY_BUFFER, vbo[1] );
	glBufferData( GL_ARRAY_BUFFER, 3 * sizeof( GLfloat ) * mesh->mNumVertices, normals, GL_STATIC_DRAW );
	glEnableVertexAttribArray( normalLoc );
    glVertexAttribPointer( normalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0 );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbo[2] );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof( unsigned int ) * mesh->mNumFaces, indices, GL_STATIC_DRAW );

	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	delete []normals;
	delete []vertices;
	delete []indices;
}

Mesh::~Mesh()
{
	glDeleteBuffers( VBOS, vbo );
	glDeleteVertexArrays( 1, &vao );
}

void Mesh::draw() const
{
	glBindVertexArray( vao );
	glDrawElements( GL_TRIANGLES, 3 * numTriangles, GL_UNSIGNED_INT, 0 ); 
	glBindVertexArray( 0 );
}

glm::vec3 Mesh::getBBmin() const
{
	return bbMin;
}

glm::vec3 Mesh::getBBmax() const
{
	return bbMax;
}
