#include <iostream>
#include <fstream>

#include "model.h"

Model::Model()
{
	bbMin = glm::vec3( 0.0 );
	bbMax = glm::vec3( 0.0 );
	centroid = glm::vec3( 0.0 );
}

Model::Model(const std::string& filename, GLint vertexLoc, GLint normalLoc)
{
	Assimp::Importer importer;
	const aiScene* scene = nullptr;

	if( importModelFromFile( filename, importer, &scene ) )
		modelFromScene( scene, vertexLoc, normalLoc );
}

Model::Model(const aiScene *scene, GLint vertexLoc, GLint normalLoc)
{
	modelFromScene( scene, vertexLoc, normalLoc );
}

Model::~Model()
{
	for( unsigned int i = 0; i < meshes.size(); ++i )
		delete meshes[i];
}

void Model::draw() const
{
	for( unsigned int i = 0; i < meshes.size(); ++i )
		meshes[i]->draw();
}

Mesh* Model::getMesh(unsigned int n) const
{
	return ( n < meshes.size() ) ? meshes[n] : nullptr;
}

unsigned int Model::getNumberOfMeshes() const
{
	return meshes.size();
}

glm::vec3 Model::getBBmin() const
{
	return bbMin;
}

glm::vec3 Model::getBBmax() const
{
	return bbMax;
}

glm::vec3 Model::getCentroid() const
{
	return centroid;
}

bool Model::importModelFromFile( const std::string& filename, Assimp::Importer& importer, const aiScene **scene )
{
	std::ifstream fin( filename.c_str() );

	if( !fin.fail() )
		fin.close();
	else
	{
		std::cerr << "Nie moge otworzyc pliku: " << filename.c_str() << std::endl;
		std::cerr << importer.GetErrorString() << std::endl;
		return false;
	}

	*scene = importer.ReadFile( filename, 0 | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality | aiProcess_GenSmoothNormals );
	//*scene = importer.ReadFile( filename, 0 | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality | aiProcess_GenNormals );

	if( !*scene )
	{
		std::cerr << importer.GetErrorString() << std::endl;
		return false;
	}

	return true;
}

void Model::modelFromScene(const aiScene *scene, GLint vertexLoc, GLint normalLoc)
{
	for( unsigned int i = 0; i < scene->mNumMeshes; ++i )
		meshes.push_back( new Mesh( scene->mMeshes[i], vertexLoc, normalLoc ) );

	computeBoundingBox();
	computeCentroid( scene );
}

void Model::computeBoundingBox()
{
	if( meshes.size() == 0 )
		return;

	bbMin = meshes[0]->getBBmin();
	bbMax = meshes[0]->getBBmax();

	for( unsigned int i = 1; i < meshes.size(); ++i )
	{
		glm::vec3 bbmin = meshes[i]->getBBmin();
		glm::vec3 bbmax = meshes[i]->getBBmax();

		if( bbmin.x < bbMin.x )
			bbMin.x = bbmin.x;
		if( bbmin.y < bbMin.y )
			bbMin.y = bbmin.y;
		if( bbmin.z < bbMin.z )
			bbMin.z = bbmin.z;

		if( bbmax.x > bbMax.x )
			bbMax.x = bbmax.x;
		if( bbmax.y > bbMax.y )
			bbMax.y = bbmax.y;
		if( bbmax.z > bbMax.z )
			bbMax.z = bbmax.z;
	}
}

void Model::computeCentroid(const aiScene *scene)
{
	glm::vec3 c = glm::vec3( 0.0f, 0.0f, 0.0f );
	unsigned int n = 0;
	for( unsigned int i = 0; i < scene->mNumMeshes; ++i )
	{
		const aiMesh *mesh = scene->mMeshes[i];

		for( unsigned int j = 0; j < mesh->mNumVertices; ++j )
		{
			const aiVector3D *vertex = &mesh->mVertices[j];

			c.x += vertex->x;
			c.y += vertex->y;
			c.z += vertex->z;

			++n;
		}
	}

	centroid = ( n != 0 ) ? c * (1.0f / n) : c;
}
