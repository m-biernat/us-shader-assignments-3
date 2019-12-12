// Micha³ Biernat IN¯ III PGK 1 - Zestaw 3 - Zadanie 2
// NUMPAD: 1 - ADS_FRAGMENT, 2 - WARD_PHONG, 3 - WARD_GOURAUD

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <iostream>

#include "shaders.h"
#include "model.h"

constexpr int WIDTH = 600;
constexpr int HEIGHT = 600;
constexpr int NUM_SHADERS = 3;
constexpr float ROT_STEP = 10.0f;
constexpr float ZOOM_FACTOR = 1.1f;

//******************************************************************************************
const std::string modelName = "models/dragon.obj";

Model* model;
enum Shader { ADS_FRAGMENT, WARD_PHONG, WARD_GOURAUD };

const std::string shader_str[] =
{
	"ambient-diffuse-specular (per fragment)",
	"ward phong",
	"ward gouraud"
};

Shader shader = ADS_FRAGMENT;
GLuint shaderProgram[NUM_SHADERS];

GLuint projMatrixLoc;
GLuint mvMatrixLoc;
GLuint normalMatrixLoc;

GLuint lightPositionLoc;
GLuint lightAmbientLoc;
GLuint lightDiffuseLoc;
GLuint lightSpecularLoc;

GLuint materialAmbientLoc;
GLuint materialDiffuseLoc;
GLuint materialSpecularLoc;
GLuint materialShininessLoc;

glm::mat4 projMatrix;
glm::mat4 mvMatrix;

// parametry swiatla
glm::vec4 lightPosition = glm::vec4(0.0f, 0.0f, 10.0f, 1.0f); // pozycja we ukladzie swiata
glm::vec3 lightAmbient = glm::vec3(0.2f, 0.2f, 0.2f);
glm::vec3 lightDiffuse = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 lightSpecular = glm::vec3(1.0, 1.0, 1.0);

// material obiektu
glm::vec3 materialAmbient = glm::vec3(1.0f, 0.5f, 0.0f);
glm::vec3 materialDiffuse = glm::vec3(0.34615f, 0.3143f, 0.0903f);
glm::vec3 materialSpecular = glm::vec3(0.797357, 0.723991, 0.208006);
float shininess = 83.2f;

bool wireframe = false; // czy rysowac siatke (true) czy wypelnienie (false)
glm::vec3 rotationAngles = glm::vec3(-80.0, 0.0, 0.0); // katy rotacji wokol poszczegolnych osi

glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

float aspectRatio = (float)WIDTH / HEIGHT;
float fovy = 45.0f;
//******************************************************************************************

void errorCallback(int error, const char* description);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void onShutdown();
void initGL();
void printStatus();
void setupShaders();
void getUniformsLocations(Shader shader);
void renderScene();
void updateProjectionMatrix();

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

	window = glfwCreateWindow(WIDTH, HEIGHT, "Zadanie 2", nullptr, nullptr);
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

	// glowna petla programu
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
** window - okno, które otrzymalo zdarzenie
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

		case GLFW_KEY_1:
		case GLFW_KEY_2:
		case GLFW_KEY_3:
			shader = (Shader)(key - 49);
			getUniformsLocations(shader);
			printStatus();
			break;

		case GLFW_KEY_F1:
			wireframe = !wireframe;
			break;
		}
	}
}

/*------------------------------------------------------------------------------------------
** funkcja zwrotna do obslugi zmiany rozmiary bufora ramku
** window - okno, które otrzymalo zdarzenie
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
** funkcja wykonywana przed zamknieciem programu
**------------------------------------------------------------------------------------------*/
void onShutdown()
{
	for (int i = 0; i < NUM_SHADERS; ++i)
		glDeleteProgram(shaderProgram[i]);

	delete model;
}

/*------------------------------------------------------------------------------------------
** funkcja inicjujaca ustawienia OpenGL
**------------------------------------------------------------------------------------------*/
void initGL()
{
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	setupShaders();

	updateProjectionMatrix();

	model = new Model(modelName);

	glm::vec3 extent = glm::abs(model->getBBmax() - model->getBBmin());
	float maxExtent = glm::max(glm::max(extent.x, extent.y), extent.z);
	scale = glm::vec3(7.0 / maxExtent);

	printStatus();
}

/*------------------------------------------------------------------------------------------
** funkcja wypisujaca podstawowe informacje o ustawieniach programu
**------------------------------------------------------------------------------------------*/
void printStatus()
{
	system("cls");

	std::cout << "GLEW = " << glewGetString(GLEW_VERSION) << std::endl;
	std::cout << "GL_VENDOR = " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "GL_RENDERER = " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "GL_VERSION = " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL = " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl << std::endl;

	std::cout << "Program cieniowania: " << shader_str[shader] << std::endl;
}

/*------------------------------------------------------------------------------------------
** funkcja tworzaca program cieniowania skladajacy sie z shadera wierzcholkow i fragmentow
**------------------------------------------------------------------------------------------*/
void setupShaders()
{
	if (!setupShaders("shaders/vertex.vert", "shaders/fragment.frag", shaderProgram[ADS_FRAGMENT]))
		exit(3);

	if (!setupShaders("shaders/ward_phong.vert", "shaders/ward_phong.frag", shaderProgram[WARD_PHONG]))
		exit(3);

	if (!setupShaders("shaders/ward_gouraud.vert", "shaders/ward_gouraud.frag", shaderProgram[WARD_GOURAUD]))
		exit(3);

	getUniformsLocations(shader);
}

/*------------------------------------------------------------------------------------------
** funkcja pobiera lokalizacje zmiennych jednorodnych dla wybranego programu cieniowania
** shader - indeks programu cieniowania w tablicy z identyfikatorami programow
**------------------------------------------------------------------------------------------*/
void getUniformsLocations(Shader shader)
{
	projMatrixLoc = glGetUniformLocation(shaderProgram[shader], "projectionMatrix");
	mvMatrixLoc = glGetUniformLocation(shaderProgram[shader], "modelViewMatrix");
	normalMatrixLoc = glGetUniformLocation(shaderProgram[shader], "normalMatrix");

	lightPositionLoc = glGetUniformLocation(shaderProgram[shader], "lightPosition");

	lightAmbientLoc = glGetUniformLocation(shaderProgram[shader], "lightAmbient");
	lightDiffuseLoc = glGetUniformLocation(shaderProgram[shader], "lightDiffuse");
	lightSpecularLoc = glGetUniformLocation(shaderProgram[shader], "lightSpecular");

	materialAmbientLoc = glGetUniformLocation(shaderProgram[shader], "materialAmbient");
	materialDiffuseLoc = glGetUniformLocation(shaderProgram[shader], "materialDiffuse");
	materialSpecularLoc = glGetUniformLocation(shaderProgram[shader], "materialSpecular");

	if (shader == ADS_FRAGMENT)
		materialShininessLoc = glGetUniformLocation(shaderProgram[shader], "materialShininess");
}

/*------------------------------------------------------------------------------------------
** funkcja rysujaca scene
**------------------------------------------------------------------------------------------*/
void renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mvMatrix = glm::lookAt(glm::vec3(0, 0, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	glUseProgram(shaderProgram[shader]);
	glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, glm::value_ptr(projMatrix));

	glm::vec4 lightPos = mvMatrix * lightPosition;
	glUniform4fv(lightPositionLoc, 1, glm::value_ptr(lightPos));

	glUniform3fv(lightAmbientLoc, 1, glm::value_ptr(lightAmbient));
	glUniform3fv(lightDiffuseLoc, 1, glm::value_ptr(lightDiffuse));
	glUniform3fv(lightSpecularLoc, 1, glm::value_ptr(lightSpecular));

	glUniform3fv(materialDiffuseLoc, 1, glm::value_ptr(materialDiffuse));
	glUniform3fv(materialAmbientLoc, 1, glm::value_ptr(materialAmbient));
	glUniform3fv(materialSpecularLoc, 1, glm::value_ptr(materialSpecular));

	if (shader == ADS_FRAGMENT)
		glUniform1f(materialShininessLoc, shininess);

	if (wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	mvMatrix = glm::scale(mvMatrix, scale);
	mvMatrix = glm::rotate(mvMatrix, glm::radians(rotationAngles.z), glm::vec3(0.0f, 0.0f, 1.0f));
	mvMatrix = glm::rotate(mvMatrix, glm::radians(rotationAngles.y), glm::vec3(0.0f, 1.0f, 0.0f));
	mvMatrix = glm::rotate(mvMatrix, glm::radians(rotationAngles.x), glm::vec3(1.0f, 0.0f, 0.0f));
	mvMatrix = glm::translate(mvMatrix, -model->getCentroid());

	glUniformMatrix4fv(mvMatrixLoc, 1, GL_FALSE, glm::value_ptr(mvMatrix));

	glm::mat3 normalMat = glm::inverseTranspose(glm::mat3(mvMatrix));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMat));

	model->draw();
}

/*------------------------------------------------------------------------------------------
** funkcja aktualizuje macierz projekcji
**------------------------------------------------------------------------------------------*/
void updateProjectionMatrix()
{
	projMatrix = glm::perspective(glm::radians(fovy), aspectRatio, 0.1f, 100.0f);
}