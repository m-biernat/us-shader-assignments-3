﻿// Michał Biernat INŻ III PGK 1 - Zestaw 3 - Zadanie 1

#define _USE_MATH_DEFINES

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <iostream>
#include <vector>
#include <cmath>

#include "shaders.h"


const int V_MAX = 12;
const int U_MAX = 16;

const float RADIUS = 1.0f;
const float Z_MIN = -1.0f;
const float Z_MAX = 1.0f;
const float THETA_MAX = 360.0f * ((float)M_PI / 180);


constexpr int WIDTH = 600; // szerokosc okna
constexpr int HEIGHT = 600; // wysokosc okna
constexpr float ROT_STEP = 15.0f; // kat obrotu (w stopniach)
constexpr float ZOOM_FACTOR = 1.1f; // wspolczynnik do zmiany kata fovy

//******************************************************************************************
GLuint vao; // identyfikatory VAO
GLuint buffers[2]; // identyfikatory VBO

GLuint shaderProgram; // identyfikator programu cieniowania

GLuint vertexLoc; // lokalizacja atrybutu wierzcholka - wspolrzedne wierzcholkow
GLuint colorLoc; // lokalizacja zmiennej jednorodnej - kolor rysowania prymitywu

GLuint projMatrixLoc; // lokalizacja zmiennej jednorodnej - macierz projekcji
GLuint mvMatrixLoc; // lokalizacja zmiennej jednorodnej - macierz model-widok

glm::mat4 projMatrix; // macierz projekcji
glm::mat4 mvMatrix; // macierz model-widok

bool wireframe = true; // czy rysowac siatke (true) czy wypelnienie (false)
glm::vec3 rotationAngles = glm::vec3(0.0, 0.0, 0.0); // katy rotacji wokol poszczegolnych osi
float fovy = 15.0f; // kat patrzenia (uzywany do skalowania sceny)
float aspectRatio = static_cast<float>(WIDTH) / HEIGHT;

GLint indicesNumber = 0; // liczba indeksow definiujacych obiekt

float color[] = { 0.0f, 1.0f, 0.0f, 1.0f }; // kolor jakim rysowac siatke
float lineWidth = 1.5f; // grubosc linii
//******************************************************************************************

void errorCallback(int error, const char* description);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void updateProjectionMatrix();
void onShutdown();
void initGL();
void setupShaders();
void setupBuffers();
void renderScene();

void generateSphereVertices(std::vector<float> &vertices, std::vector<float> &normals);
void generateSphereIndices(std::vector<unsigned int> &vertices);

int main(int argc, char* argv[])
{
	atexit(onShutdown);

	GLFWwindow* window;

	glfwSetErrorCallback(errorCallback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Zadanie 1", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, keyCallback);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		std::cerr << "Blad: " << glewGetErrorString(err) << std::endl;
		exit(1);
	}

	if (!GLEW_VERSION_3_3)
	{
		std::cerr << "Brak obslugi OpenGL 3.3\n";
		exit(2);
	}

	glfwSwapInterval(1); // v-sync on

	initGL();

	while (!glfwWindowShouldClose(window))
	{
		renderScene();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);

	return 0;
}

/*------------------------------------------------------------------------------------------
** funkcja zwrotna do obslugi bledow biblioteki GLFW
** error - kod bledu
** description - opis bledu
**------------------------------------------------------------------------------------------*/
void errorCallback(int error, const char* description)
{
	std::cerr << "Error: " << description << std::endl;
}

/*------------------------------------------------------------------------------------------
** funkcja zwrotna do obslugi klawiatury
** window - okno, kt�re otrzymalo zdarzenie
** key - klawisz jaki zostal nacisniety lub zwolniony
** scancode - scancode klawisza specyficzny dla systemu
** action - zachowanie klawisza (GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT)
** mods - pole bitowe zawierajace informacje o nacisnietych modyfikatorach (GLFW_MOD_SHIFT, GLFW_MOD_CONTROL, GLFW_MOD_ALT, GLFW_MOD_SUPER)
**------------------------------------------------------------------------------------------*/
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		switch (key)
		{
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			break;

		case GLFW_KEY_W:
			rotationAngles.x -= ROT_STEP;
			if (rotationAngles.x < 0.0f)
				rotationAngles.x += 360.0f;
			break;

		case GLFW_KEY_S:
			rotationAngles.x += ROT_STEP;
			if (rotationAngles.x > 360.0f)
				rotationAngles.x -= 360.0f;
			break;

		case GLFW_KEY_A:
			rotationAngles.y -= ROT_STEP;
			if (rotationAngles.y < 0.0f)
				rotationAngles.y += 360.0f;
			break;

		case GLFW_KEY_D:
			rotationAngles.y += ROT_STEP;
			if (rotationAngles.y > 360.0f)
				rotationAngles.y -= 360.0f;
			break;

		case GLFW_KEY_E:
			rotationAngles.z -= ROT_STEP;
			if (rotationAngles.z < 0.0f)
				rotationAngles.z += 360.f;
			break;

		case GLFW_KEY_Q:
			rotationAngles.z += ROT_STEP;
			if (rotationAngles.z > 360.0f)
				rotationAngles.z -= 360.0f;
			break;

		case GLFW_KEY_EQUAL: // =
		case GLFW_KEY_KP_ADD: // + na klawiaturze numerycznej
			fovy /= ZOOM_FACTOR;
			updateProjectionMatrix();
			break;

		case GLFW_KEY_MINUS: // -
		case GLFW_KEY_KP_SUBTRACT: // - na klawiaturze numerycznej
			if (ZOOM_FACTOR * fovy < 180.0f)
			{
				fovy *= ZOOM_FACTOR;
				updateProjectionMatrix();
			}
			break;

		case GLFW_KEY_F1:
			wireframe = !wireframe;
			break;
		}
	}
}

/*------------------------------------------------------------------------------------------
** funkcja zwrotna do obslugi zmiany rozmiary bufora ramku
** window - okno, kt�re otrzymalo zdarzenie
** width - szerokosc bufora ramki
** height - wysokosc bufora ramki
**------------------------------------------------------------------------------------------*/
void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);

	aspectRatio = static_cast<float>(width) / ((height == 0) ? 1 : height);
	updateProjectionMatrix();
}

/*------------------------------------------------------------------------------------------
** funkcja aktualizuje macierz projekcji
**------------------------------------------------------------------------------------------*/
void updateProjectionMatrix()
{
	projMatrix = glm::perspective(glm::radians(fovy), aspectRatio, 0.1f, 100.0f);
}

/*------------------------------------------------------------------------------------------
** funkcja wykonywana przed zamknieciem programu
**------------------------------------------------------------------------------------------*/
void onShutdown()
{
	glDeleteBuffers(2, buffers);
	glDeleteVertexArrays(1, &vao);
	glDeleteProgram(shaderProgram);
}

/*------------------------------------------------------------------------------------------
** funkcja inicjujaca ustawienia OpenGL
**------------------------------------------------------------------------------------------*/
void initGL()
{
	std::cout << "GLEW = " << glewGetString(GLEW_VERSION) << std::endl;
	std::cout << "GL_VENDOR = " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "GL_RENDERER = " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "GL_VERSION = " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL = " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	updateProjectionMatrix();

	setupShaders();

	setupBuffers();
}

/*------------------------------------------------------------------------------------------
** funkcja tworzaca program cieniowania skladajacy sie z shadera wierzcholkow i fragmentow
**------------------------------------------------------------------------------------------*/
void setupShaders()
{
	if (!setupShaders("shaders/vertex.vert", "shaders/fragment.frag", shaderProgram))
		exit(3);

	vertexLoc = glGetAttribLocation(shaderProgram, "vPosition");
	colorLoc = glGetUniformLocation(shaderProgram, "color");

	projMatrixLoc = glGetUniformLocation(shaderProgram, "projectionMatrix");
	mvMatrixLoc = glGetUniformLocation(shaderProgram, "modelViewMatrix");
}

/*------------------------------------------------------------------------------------------
** funkcja inicjujaca VAO oraz zawarte w nim VBO z danymi o modelu
**------------------------------------------------------------------------------------------*/
void setupBuffers()
{
	std::vector<float> vertices;
	std::vector<float> normals;

	generateSphereVertices(vertices, normals);

	std::vector<unsigned int> indices;

	generateSphereIndices(indices);

	indicesNumber = (GLuint)indices.size();

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(2, buffers);
	// VBO dla wierzcholkow
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), reinterpret_cast<GLfloat*>(&vertices[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexLoc);
	glVertexAttribPointer(vertexLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);

	// VBO dla indeksow
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), reinterpret_cast<GLuint*>(&indices[0]), GL_STATIC_DRAW);

	glBindVertexArray(0);
}

/*------------------------------------------------------------------------------------------
** funkcja rysujaca scene
**------------------------------------------------------------------------------------------*/
void renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mvMatrix = glm::lookAt(glm::vec3(0, 0, 8), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));

	glUseProgram(shaderProgram);
	glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, glm::value_ptr(projMatrix));

	if (wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(lineWidth);
	}
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	mvMatrix = glm::rotate(mvMatrix, glm::radians(rotationAngles.z), glm::vec3(0.0f, 0.0f, 1.0f));
	mvMatrix = glm::rotate(mvMatrix, glm::radians(rotationAngles.y), glm::vec3(0.0f, 1.0f, 0.0f));
	mvMatrix = glm::rotate(mvMatrix, glm::radians(rotationAngles.x), glm::vec3(1.0f, 0.0f, 0.0f));

	glUniformMatrix4fv(mvMatrixLoc, 1, GL_FALSE, glm::value_ptr(mvMatrix));

	glBindVertexArray(vao);
	glUniform4fv(colorLoc, 1, color);
	glDrawElements(GL_TRIANGLES, indicesNumber, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void generateSphereVertices(std::vector<float> &vertices, std::vector<float> &normals)
{
	float fi, fiMin, fiMax, theta;

	if (Z_MIN > -RADIUS)
		fiMin = asin(Z_MIN / RADIUS);
	else if (Z_MIN <= -RADIUS)
		fiMin = -90.0f * ((float)M_PI / 180);

	if (Z_MAX < RADIUS)
		fiMax = asin(Z_MAX / RADIUS);
	else if (Z_MAX >= RADIUS)
		fiMax = 90.0f * ((float)M_PI / 180);

	const float V_STEP = 1.0f / V_MAX;
	const float U_STEP = 1.0f / U_MAX;

	float v = 0.0f, u = 0.0f;
	
	float x, y, z, w = 1.0f;

	const float LENGTH = 1.0f / RADIUS;

	//std::cout << fiMin << " " << fiMax << ", " << V_STEP << " " << U_STEP << std::endl;

	for (int i = 0; i < V_MAX + 1; i++)
	{
		fi = fiMin + v * (fiMax - fiMin);

		for (int j = 0; j < U_MAX; j++)
		{
			theta = u * THETA_MAX;

			x = RADIUS * cos(theta) * cos(fi);
			y = RADIUS * sin(theta) * cos(fi);
			z = RADIUS * sin(fi);

			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);
			vertices.push_back(w);

			normals.push_back(x * LENGTH);
			normals.push_back(y * LENGTH);
			normals.push_back(z * LENGTH);
			normals.push_back(w);

			u += U_STEP;
		}
		v += V_STEP;
	}
}

void generateSphereIndices(std::vector<unsigned int> &indices)
{
	int i = 0, j = 0, k;

	while (j < U_MAX)
	{
		j++;
		k = j + 1;

		indices.push_back(i); //	0, 0, 0 ...
		indices.push_back(j); //	1, 2, 3 ...		domkniecie gory
		indices.push_back(k); //	2, 3, 4 ...

		//std::cout << i << " " << j << " " << k << std::endl;
	}

	for (i = 2; i < V_MAX * U_MAX; i++)
	{
		j++;
		k = j + 1;

		indices.push_back(i); //		  2,	   3,	    4 ...
		indices.push_back(j); //	  U_MAX, U_MAX+1, U_MAX+2 ...	1/2 quada
		indices.push_back(k); //	U_MAX+1, U_MAX+2, U_MAX+3 ...

		indices.push_back(i); //		  2,	   3,	    4 ...
		indices.push_back(j); //	  U_MAX, U_MAX+1, U_MAX+2 ...	2/2 quada
		indices.push_back(i - 1); //	  1,	   2,		3 ...

		//std::cout << i << " " << j << " " << k << std::endl;
	}
}