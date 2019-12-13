// Michał Biernat INŻ III PGK 1 - Zestaw 3 - Zadanie 3

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

constexpr float ROT_STEP = 10.0f;
constexpr float ZOOM_FACTOR = 1.1f;

//******************************************************************************************
const std::string modelName = "models/dragon.obj";

Model* model;

GLuint shaderProgram;

GLuint projMatrixLoc;
GLuint mvMatrixLoc;
GLuint normalMatrixLoc;

const int NUM_OF_LIGHTS = 2;

struct LightLoc 
{
	GLuint position;
	GLuint theta;
	
	GLuint ambient;
	GLuint diffuse;
	GLuint specular;

	GLuint enabled;
};

LightLoc lightLoc[NUM_OF_LIGHTS];

GLuint materialAmbientLoc;
GLuint materialDiffuseLoc;
GLuint materialSpecularLoc;
GLuint materialShininessLoc;

glm::mat4 projMatrix;
glm::mat4 mvMatrix;

// parametry swiatla
glm::vec4 lightPosition[] {
	glm::vec4(0.0f, 0.0f, 5.0f, 0.0f), glm::vec4(0.0f, 0.0f, 5.0f, 1.0f)
};
float lightTheta[] { 
	180.0f, 30.0f 
};
glm::vec3 lightAmbient[] { 
	glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.2f, 0.0f, 0.0f)
};
glm::vec3 lightDiffuse[] { 
	glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f)
};
glm::vec3 lightSpecular[] {
	glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f)
};
bool lightEnabled[] {
	true, false
};

// material obiektu
glm::vec3 materialAmbient = glm::vec3(1.0f, 0.5f, 0.0f);
glm::vec3 materialDiffuse = glm::vec3(0.34615f, 0.3143f, 0.0903f);
glm::vec3 materialSpecular = glm::vec3(0.797357, 0.723991, 0.208006);
float shininess = 83.2f;

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
void setupShaders();
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

	window = glfwCreateWindow(WIDTH, HEIGHT, "Zadanie 3", nullptr, nullptr);
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
			lightPosition[0].w = 1.0f;
			lightTheta[0] = 180.0f;
			break;
		case GLFW_KEY_F2:
			lightPosition[0].w = 0.0f;
			break;
		case GLFW_KEY_F3:
			lightPosition[0].w = 1.0f;
			lightTheta[0] = 30.0f;
			break;
		case GLFW_KEY_F4:
			lightEnabled[0] = !lightEnabled[0];
			break;

		case GLFW_KEY_F5:
			lightPosition[1].w = 1.0f;
			lightTheta[1] = 180.0f;
			break;
		case GLFW_KEY_F6:
			lightPosition[1].w = 0.0f;
			break;
		case GLFW_KEY_F7:
			lightPosition[1].w = 1.0f;
			lightTheta[1] = 30.0f;
			break;
		case GLFW_KEY_F8:
			lightEnabled[1] = !lightEnabled[1];
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
** funkcja wykonywana przed zamknieciem programu
**------------------------------------------------------------------------------------------*/
void onShutdown()
{
	glDeleteProgram(shaderProgram);

	delete model;
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

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	setupShaders();

	updateProjectionMatrix();

	model = new Model(modelName);

	glm::vec3 extent = glm::abs(model->getBBmax() - model->getBBmin());
	float maxExtent = glm::max(glm::max(extent.x, extent.y), extent.z);
	scale = glm::vec3(7.0 / maxExtent);
}

/*------------------------------------------------------------------------------------------
** funkcja tworzaca program cieniowania skladajacy sie z shadera wierzcholkow i fragmentow
**------------------------------------------------------------------------------------------*/
void setupShaders()
{
	if (!setupShaders("shaders/vertex.vert", "shaders/fragment.frag", shaderProgram))
		exit(3);

	projMatrixLoc = glGetUniformLocation(shaderProgram, "projectionMatrix");
	mvMatrixLoc = glGetUniformLocation(shaderProgram, "modelViewMatrix");
	normalMatrixLoc = glGetUniformLocation(shaderProgram, "normalMatrix");

	materialAmbientLoc = glGetUniformLocation(shaderProgram, "materialAmbient");
	materialDiffuseLoc = glGetUniformLocation(shaderProgram, "materialDiffuse");
	materialSpecularLoc = glGetUniformLocation(shaderProgram, "materialSpecular");
	materialShininessLoc = glGetUniformLocation(shaderProgram, "materialShininess");

	lightLoc[0].position = glGetUniformLocation(shaderProgram, "light[0].position");
	lightLoc[0].theta = glGetUniformLocation(shaderProgram, "light[0].theta");

	lightLoc[0].ambient = glGetUniformLocation(shaderProgram, "light[0].ambient");
	lightLoc[0].diffuse = glGetUniformLocation(shaderProgram, "light[0].diffuse");
	lightLoc[0].specular = glGetUniformLocation(shaderProgram, "light[0].specular");

	lightLoc[0].enabled = glGetUniformLocation(shaderProgram, "light[0].enabled");

	lightLoc[1].position = glGetUniformLocation(shaderProgram, "light[1].position");
	lightLoc[1].theta = glGetUniformLocation(shaderProgram, "light[1].theta");

	lightLoc[1].ambient = glGetUniformLocation(shaderProgram, "light[1].ambient");
	lightLoc[1].diffuse = glGetUniformLocation(shaderProgram, "light[1].diffuse");
	lightLoc[1].specular = glGetUniformLocation(shaderProgram, "light[1].specular");

	lightLoc[1].enabled = glGetUniformLocation(shaderProgram, "light[1].enabled");
}

/*------------------------------------------------------------------------------------------
** funkcja rysujaca scene
**------------------------------------------------------------------------------------------*/
void renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mvMatrix = glm::lookAt(glm::vec3(0, 0, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	glUseProgram(shaderProgram);
	glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, glm::value_ptr(projMatrix));

	for (int i = 0; i < NUM_OF_LIGHTS; i++)
	{ 
		glm::vec4 lightPos = mvMatrix * lightPosition[i];
		glUniform4fv(lightLoc[i].position, 1, glm::value_ptr(lightPos));

		glUniform1f(lightLoc[i].theta, glm::radians(lightTheta[i]));

		glUniform3fv(lightLoc[i].ambient, 1, glm::value_ptr(lightAmbient[i]));
		glUniform3fv(lightLoc[i].diffuse, 1, glm::value_ptr(lightDiffuse[i]));
		glUniform3fv(lightLoc[i].specular, 1, glm::value_ptr(lightSpecular[i]));

		glUniform1i(lightLoc[i].enabled, lightEnabled[i]);
	}

	glUniform3fv(materialDiffuseLoc, 1, glm::value_ptr(materialDiffuse));
	glUniform3fv(materialAmbientLoc, 1, glm::value_ptr(materialAmbient));
	glUniform3fv(materialSpecularLoc, 1, glm::value_ptr(materialSpecular));
	glUniform1f(materialShininessLoc, shininess);

	

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