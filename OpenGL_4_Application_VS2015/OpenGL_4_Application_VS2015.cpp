//
//  main.cpp
//  OpenGL_Shader_Example_step1
//
//  Created by CGIS on 02/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC
//#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glfw3.lib")

#include <iostream>
#include "glm/glm.hpp"//core glm functionality
#include "glm/gtc/matrix_transform.hpp"//glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLEW/glew.h"
#include "GLFW/glfw3.h"
#include <string>
#include "Shader.hpp"
#include "Camera.hpp"
#define TINYOBJLOADER_IMPLEMENTATION

#include "Model3D.hpp"
#include "Mesh.hpp"
#include "SkyBox.hpp"

//int glWindowWidth = 640;
//int glWindowHeight = 480;
int glWindowWidth = 1900;
int glWindowHeight = 1060;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const GLuint SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

glm::mat4 model;
GLint modelLoc;
glm::mat4 view;
GLint viewLoc;
glm::mat4 model2;
glm::mat4 model3;
glm::mat4 model4;
glm::mat4 model5;
glm::mat4 model6;
glm::mat4 model7;
glm::mat4 model8;
glm::mat4 projection;
GLint projLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat3 lightDirMatrix;
GLuint lightDirMatrixLoc;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;
glm::vec3 lightPos;
GLuint lightPosLoc;
GLuint lightColorLoc2;
glm::vec3 lightColor2;
glm::vec3 lightPos2;
GLuint lightPosLoc2;

gps::Camera myCamera(glm::vec3(0.0f, 5.0f, 70.0f), glm::vec3(0.0f, 2.0f, 0.0f));
float cameraSpeed = 1.0f;

bool pressedKeys[1024];
float angle = 0.0f;
GLfloat lightAngle;

bool firstMouse = true;
double lastX = 400, lastY = 300;
float yaw = -90.0f, pitch = 0;
float fov = 45.0f;

gps::Model3D houseModel;
gps::Model3D treeModel;
gps::Model3D treeModel2;
gps::Model3D ground;
gps::Model3D streetLight;
gps::Model3D dog_body;
gps::Model3D dog_tail;
gps::Model3D dog_legs1;
gps::Model3D dog_legs2;
gps::Model3D airplane;
gps::Model3D dog2;
gps::Model3D fence;

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader depthMapShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;
std::vector<const GLchar*> faces;

float moveAirplane = 0;
float moveDog = 0;
bool stopLegs = false;

float totalTime = 25.0f;
float t = 0;
bool startAnimation = false;

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM: error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE: error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION: error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW: error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW: error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY: error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}

#define glCheckError() glCheckError_(__FILE__, __LINE__)
void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO
	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	myCustomShader.useShaderProgram();

	//set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(fov), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.05f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	myCamera.rotate(pitch, yaw);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
}

void processMovement()
{
	if (!startAnimation) {

		if (pressedKeys[GLFW_KEY_Q]) {
			angle += 0.1f;
			if (angle > 360.0f)
				angle -= 360.0f;
		}

		if (pressedKeys[GLFW_KEY_E]) {
			angle -= 0.1f;
			if (angle < 0.0f)
				angle += 360.0f;
		}

		if (pressedKeys[GLFW_KEY_W]) {
			myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		}

		if (pressedKeys[GLFW_KEY_S]) {
			myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
		}

		if (pressedKeys[GLFW_KEY_A]) {
			myCamera.move(gps::MOVE_LEFT, cameraSpeed);
		}

		if (pressedKeys[GLFW_KEY_D]) {
			myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
		}

		if (pressedKeys[GLFW_KEY_J]) {

			lightAngle += 0.3f;
			if (lightAngle > 360.0f)
				lightAngle -= 360.0f;
			glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
			myCustomShader.useShaderProgram();
			glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
		}

		if (pressedKeys[GLFW_KEY_L]) {
			lightAngle -= 0.3f;
			if (lightAngle < 0.0f)
				lightAngle += 360.0f;
			glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
			myCustomShader.useShaderProgram();
			glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
		}

		//wireframe view
		if (pressedKeys[GLFW_KEY_X])
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		if (pressedKeys[GLFW_KEY_Z])
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		if (pressedKeys[GLFW_KEY_V])
		{
			moveAirplane = 0;
		}

		if (moveAirplane < 100000)
		{
			moveAirplane += 5;
		}
		else
		{

		}

		if (moveDog < 250)
		{
			moveDog += 0.5;
		}
		else
		{
			stopLegs = true;
		}

		if (pressedKeys[GLFW_KEY_P])
		{
			startAnimation = true;
		}
	}
}

void correctCameraPosition()
{
	if (myCamera.getCameraPosition().y >= 5.0f)
	{
		myCamera.setCameraPositionY(5.0f);
	}
	else if (myCamera.getCameraPosition().y <= 1.5f)
	{
		myCamera.setCameraPositionY(1.5f);
	}
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	//for Mac OS X
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwMakeContextCurrent(glWindow);

	glfwWindowHint(GLFW_SAMPLES, 4);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetScrollCallback(glWindow, scroll_callback);

	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initFBOs()
{
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix()
{
	const GLfloat near_plane = 1.0f, far_plane = 150.0f;
	glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, near_plane, far_plane);

	glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
	glm::mat4 lightView = glm::lookAt(1.5f * lightDirTr, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	return lightProjection * lightView;
}

void initModels()
{
	houseModel = gps::Model3D("objects/house/Farmhouse.obj", "objects/house/", false);
	treeModel = gps::Model3D("objects/trees1/Tree1.obj", "objects/trees1/", true);
	treeModel2 = gps::Model3D("objects/trees2/trees2.obj", "objects/trees2/", true);
	ground = gps::Model3D("objects/ground/ground.obj", "objects/ground/", false);
	streetLight = gps::Model3D("objects/street_light/Street-Light-result.obj", "objects/street_light/", true);
	airplane = gps::Model3D("objects/airplane/11803_Airplane_v1_l1.obj", "objects/airplane/",false);
	dog_body = gps::Model3D("objects/dog/dog_body2.obj", "objects/dog/", false);
	dog_tail = gps::Model3D("objects/dog/dog_tail.obj", "objects/dog/", false);
	dog_legs1 = gps::Model3D("objects/dog/dog_legs11.obj", "objects/dog/", false);
	dog_legs2 = gps::Model3D("objects/dog/dog_legs22.obj", "objects/dog/", false);
	dog2 = gps::Model3D("objects/dog2/12227_Dog_v1_L2.obj", "objects/dog2/", false);
	fence = gps::Model3D("objects/fence/13080_Wrought_Iron_fence_with_brick_v1_L2.obj", "objects/fence/", false);

	faces.push_back("textures/mountain/front.tga");
	faces.push_back("textures/mountain/back.tga");
	faces.push_back("textures/mountain/top.tga");
	faces.push_back("textures/mountain/bottom.tga");
	faces.push_back("textures/mountain/right.tga");
	faces.push_back("textures/mountain/left.tga");

	mySkyBox.Load(faces);
}

void initShaders()
{
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	depthMapShader.loadShader("shaders/simpleDepthMap.vert", "shaders/simpleDepthMap.frag");
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
}

void initUniforms()
{
	myCustomShader.useShaderProgram();

	//initialize the model matrix
	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//initialize the view matrix
	view = myCamera.getViewMatrix();
	//send matrix data to shader
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");

	lightDirMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDirMatrix");

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 10.0f, 10.0f);
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	skyboxShader.useShaderProgram();
	glUniform3fv(glGetUniformLocation(skyboxShader.shaderProgram, "lightDir"), 1, glm::value_ptr(lightDir));
	myCustomShader.useShaderProgram();

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	//set light color 2
	lightColor2 = glm::vec3(1.0f, 1.0f, 0.2f);
	lightColorLoc2 = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor2");
	glUniform3fv(lightColorLoc2, 1, glm::value_ptr(lightColor2));

	lightPos = glm::vec3(-17, 20, 30);
	lightPosLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPos");
	glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));

	lightPos2 = glm::vec3(17, 20, 30);
	lightPosLoc2 = glGetUniformLocation(myCustomShader.shaderProgram, "lightPos2");
	glUniform3fv(lightPosLoc2, 1, glm::value_ptr(lightPos2));
}

void renderHouseDepthMap(gps::Shader shader)
{
	//create model matrix for house
	model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0));
	model = glm::rotate(model, 3.14f, glm::vec3(0, 1, 0));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	houseModel.Draw(shader);
}

void renderHouse(gps::Shader shader)
{
	//create model matrix for house
	model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0));
	model = glm::rotate(model, 3.14f, glm::vec3(0, 1, 0));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	//initialize the projection matrix
	projection = glm::perspective(glm::radians(fov), (float)glWindowWidth / (float)glWindowHeight, 0.1f, 1000.0f);
	//send matrix data to shader
	projLoc = glGetUniformLocation(shader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	houseModel.Draw(shader);
}

void renderGroundDepthMap(gps::Shader shader)
{
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
		{
			//create model matrix for ground
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(-80 + 20 * i, 0, -80 + 20 * j));
			//send model matrix to shader
			glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

			ground.Draw(shader);
		}
}

void renderGround(gps::Shader shader)
{
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
		{
			//model for ground
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(-80 + 20*i, 0, -80 + 20*j));
			//model = glm::scale(model, glm::vec3(5, 5, 5));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

			//create normal matrix
			normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
			//send normal matrix data to shader
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

			ground.Draw(shader);
		}
}

void renderTrees1DepthMap(gps::Shader shader)
{
	model2 = glm::mat4(1.0f);
	model2 = glm::translate(model2, glm::vec3(15, 0, 0));
	//create rotation matrix
	model2 = glm::rotate(model2, angle, glm::vec3(0, 1, 0));
	model2 = glm::rotate(model2, 3.14f, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model2));

	treeModel.Draw(shader);

	model3 = glm::mat4(1.0f);
	model3 = glm::translate(model3, glm::vec3(-15, 0, 0));
	//create rotation matrix
	model3 = glm::rotate(model3, angle, glm::vec3(0, 1, 0));
	model3 = glm::rotate(model3, 3.14f, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model3));

	treeModel.Draw(shader);
}

void renderTrees1(gps::Shader shader)
{
	model2 = glm::mat4(1.0f);
	model2 = glm::translate(model2, glm::vec3(15, 0, 0));
	//create rotation matrix
	model2 = glm::rotate(model2, angle, glm::vec3(0, 1, 0));
	model2 = glm::rotate(model2, 3.14f, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));

	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model2));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	treeModel.Draw(shader);

	model3 = glm::mat4(1.0f);
	model3 = glm::translate(model3, glm::vec3(-15, 0, 0));
	//create rotation matrix
	model3 = glm::rotate(model3, angle, glm::vec3(0, 1, 0));
	model3 = glm::rotate(model3, 3.14f, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model3));

	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model3));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	treeModel.Draw(shader);
}

void renderTrees2DepthMap(gps::Shader shader)
{
	model4 = glm::mat4(1.0f);
	model4 = glm::translate(model4, glm::vec3(-25, 0, 5));
	//create rotation matrix
	model4 = glm::rotate(model4, angle + 0.1f, glm::vec3(0, 1, 0));
	model4 = glm::rotate(model4, 3.14f, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model4));

	treeModel2.Draw(shader);

	model5 = glm::mat4(1.0f);
	model5 = glm::translate(model5, glm::vec3(25, 0, 5));
	//create rotation matrix
	model5 = glm::rotate(model5, angle - 0.1f, glm::vec3(0, 1, 0));
	model5 = glm::rotate(model5, 3.14f, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model5));

	treeModel2.Draw(shader);
}

void renderTrees2(gps::Shader shader)
{
	model4 = glm::mat4(1.0f);
	model4 = glm::translate(model4, glm::vec3(-25, 0, 5));
	//create rotation matrix
	model4 = glm::rotate(model4, angle + 0.1f, glm::vec3(0, 1, 0));
	model4 = glm::rotate(model4, 3.14f, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model4));

	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model4));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	treeModel2.Draw(shader);

	model5 = glm::mat4(1.0f);
	model5 = glm::translate(model5, glm::vec3(25, 0, 5));
	//create rotation matrix
	model5 = glm::rotate(model5, angle - 0.1f, glm::vec3(0, 1, 0));
	model5 = glm::rotate(model5, 3.14f, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model5));

	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model5));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	treeModel2.Draw(shader);
}

void renderStreetLightDepthMap(gps::Shader shader)
{
	model3 = glm::translate(glm::mat4(1.0f), glm::vec3(-10, 0, 30));
	model3 = glm::scale(model3, glm::vec3(4, 4, 4));
	model3 = glm::rotate(model3, glm::radians(90.0f), glm::vec3(0, 1, 0));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model3));

	streetLight.Draw(shader);

	model3 = glm::translate(glm::mat4(1.0f), glm::vec3(10, 0, 30));
	model3 = glm::scale(model3, glm::vec3(4, 4, 4));
	model3 = glm::rotate(model3, glm::radians(-90.0f), glm::vec3(0, 1, 0));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model3));

	streetLight.Draw(shader);
}

void renderStreetLight(gps::Shader shader)
{
	model3 = glm::translate(glm::mat4(1.0f), glm::vec3(-10, 0, 30));
	model3 = glm::scale(model3, glm::vec3(4, 4, 4));
	model3 = glm::rotate(model3, glm::radians(90.0f), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model3));

	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model3));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	//initialize the projection matrix
	projection = glm::perspective(glm::radians(fov), (float)glWindowWidth / (float)glWindowHeight, 0.1f, 1000.0f);
	//send matrix data to shader
	projLoc = glGetUniformLocation(shader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	streetLight.Draw(shader);

	model3 = glm::translate(glm::mat4(1.0f), glm::vec3(10, 0, 30));
	model3 = glm::scale(model3, glm::vec3(4, 4, 4));
	model3 = glm::rotate(model3, glm::radians(-90.0f), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model3));

	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model3));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	//initialize the projection matrix
	projection = glm::perspective(glm::radians(fov), (float)glWindowWidth / (float)glWindowHeight, 0.1f, 1000.0f);
	//send matrix data to shader
	projLoc = glGetUniformLocation(shader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	streetLight.Draw(shader);
}

float deltaDog = 0;
int sens = 1;
void updateDeltaDog() {
	deltaDog += sens * 0.025;
	if (deltaDog >= 0.05)
		sens = -1;
	if (deltaDog <= -0.05)
		sens = 1;
}

void renderDogsDepthMap(gps::Shader shader)
{
	model3 = glm::scale(model3, glm::vec3(0.02, 0.02, 0.02));
	model3 = glm::rotate(model3, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	model3 = glm::translate(model3, glm::vec3(0, -moveDog, 0));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model3));

	dog_body.Draw(shader);

	model6 = model3;
	model7 = model3;
	model8 = model3;

	model3 = glm::translate(model3, glm::vec3(0, -5, 0.5));
	model3 = glm::rotate(model3, deltaDog, glm::vec3(0, 0, 1));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model3));
	dog_tail.Draw(shader);

	if (stopLegs == false)
	{
		model7 = glm::rotate(model7, deltaDog, glm::vec3(1, 0, 0));
		model7 = glm::translate(model7, glm::vec3(0, 0, 0.75));
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model7));
	}
	else
	{
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model6));
	}

	dog_legs1.Draw(shader);

	if (stopLegs == false)
	{
		model8 = glm::rotate(model8, -deltaDog, glm::vec3(1, 0, 0));
		model8 = glm::translate(model8, glm::vec3(0, 0, 0.75));
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model8));
	}
	else
	{
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model6));
	}

	dog_legs2.Draw(shader);

	//render second dog

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-15, 0, 30));
	model = glm::scale(model, glm::vec3(0.06, 0.06, 0.06));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0, 0, 1));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	dog2.Draw(shader);
}

void renderDogs(gps::Shader shader)
{
	model3 = glm::scale(model3, glm::vec3(0.02, 0.02, 0.02));
	model3 = glm::rotate(model3, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	model3 = glm::translate(model3, glm::vec3(0, -moveDog, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model3));

	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model3));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	//initialize the projection matrix
	projection = glm::perspective(glm::radians(fov), (float)glWindowWidth / (float)glWindowHeight, 0.1f, 1000.0f);
	//send matrix data to shader
	projLoc = glGetUniformLocation(shader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	dog_body.Draw(shader);

	model6 = model3;
	model7 = model3;
	model8 = model3;

	//model3 = glm::rotate(model3, glm::radians(10.0f), glm::vec3(0, 0, 1));
	model3 = glm::translate(model3, glm::vec3(0, -5, 0.5));
	model3 = glm::rotate(model3, deltaDog, glm::vec3(0, 0, 1));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model3));
	dog_tail.Draw(shader);

	if (stopLegs == false)
	{
		model7 = glm::rotate(model7, deltaDog, glm::vec3(1, 0, 0));
		model7 = glm::translate(model7, glm::vec3(0, 0, 0.75));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model7));
	}
	else
	{
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model6));
	}
	dog_legs1.Draw(shader);

	if (stopLegs == false)
	{
		model8 = glm::rotate(model8, -deltaDog, glm::vec3(1, 0, 0));
		model8 = glm::translate(model8, glm::vec3(0, 0, 0.75));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model8));
	}
	else
	{
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model6));
	}
	dog_legs2.Draw(shader);

	//render second dog

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-15, 0, 30));
	model = glm::scale(model, glm::vec3(0.06, 0.06, 0.06));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0, 0, 1));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	dog2.Draw(shader);

	updateDeltaDog();
}

void renderAirplaneDepthMap(gps::Shader shader)
{
	model6 = glm::translate(model6, glm::vec3(-500 - moveAirplane, 200, 1000));
	model6 = glm::scale(model6, glm::vec3(0.5, 0.5, 0.5));
	model6 = glm::rotate(model6, glm::radians(-180.0f), glm::vec3(0, 0, 1));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model6));

	airplane.Draw(shader);
}

void renderAirplane(gps::Shader shader)
{
	model6 = glm::translate(model6, glm::vec3(-300 - moveAirplane, 300, 500));
	model6 = glm::scale(model6, glm::vec3(0.25, 0.25, 0.25));
	model6 = glm::rotate(model6, glm::radians(-180.0f), glm::vec3(0, 0, 1));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model6));

	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model6));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	//initialize the projection matrix
	projection = glm::perspective(glm::radians(fov), (float)glWindowWidth / (float)glWindowHeight, 0.1f, 1000.0f);
	//send matrix data to shader
	projLoc = glGetUniformLocation(shader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	airplane.Draw(shader);
}

void renderFenceDepthMap(gps::Shader shader)
{
	for (int i = -3; i < 4; i++)
	{
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(42, 0, i * 14));
		model = glm::scale(model, glm::vec3(0.05, 0.05, 0.05));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 0, 1));
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		fence.Draw(shader);
	}
	for (int i = -3; i < 4; i++)
	{
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-42, 0, i * 14));
		model = glm::scale(model, glm::vec3(0.05, 0.05, 0.05));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 0, 1));
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		fence.Draw(shader);
	}

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(35, 0, 49));
	model = glm::scale(model, glm::vec3(0.05, 0.05, 0.05));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	fence.Draw(shader);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-35, 0, 49));
	model = glm::scale(model, glm::vec3(0.05, 0.05, 0.05));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	fence.Draw(shader);

	for (int i = 0; i <= 5; i++)
	{
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-35 + i * 14, 0, -49));
		model = glm::scale(model, glm::vec3(0.05, 0.05, 0.05));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		fence.Draw(shader);
	}

}

void renderFence(gps::Shader shader)
{
	for (int i = -3; i < 4; i++)
	{
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(42, 0, i*14));
		model = glm::scale(model, glm::vec3(0.05, 0.05, 0.05));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 0, 1));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		//create normal matrix
		normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
		//send normal matrix data to shader
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

		fence.Draw(shader);
	}
	for (int i = -3; i < 4; i++)
	{
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-42, 0, i * 14));
		model = glm::scale(model, glm::vec3(0.05, 0.05, 0.05));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 0, 1));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		//create normal matrix
		normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
		//send normal matrix data to shader
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

		fence.Draw(shader);
	}
	
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(35, 0, 49));
	model = glm::scale(model, glm::vec3(0.05, 0.05, 0.05));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	fence.Draw(shader);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-35, 0, 49));
	model = glm::scale(model, glm::vec3(0.05, 0.05, 0.05));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	fence.Draw(shader);

	for (int i = 0; i <= 5; i++)
	{
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-35 + i*14, 0, -49));
		model = glm::scale(model, glm::vec3(0.05, 0.05, 0.05));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		//create normal matrix
		normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
		//send normal matrix data to shader
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

		fence.Draw(shader);
	}
}

void renderSkybox()
{
	skyboxShader.useShaderProgram();

	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

	projection = glm::perspective(glm::radians(fov), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	mySkyBox.Draw(skyboxShader, view, projection);
}

float deltaSun = 0;

void renderScene()
{
	if (!startAnimation) 
	{
		correctCameraPosition();
	}

	processMovement();

	//render the scene to the depth buffer (first pass)

	depthMapShader.useShaderProgram();

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));

	renderHouseDepthMap(depthMapShader);
	renderGroundDepthMap(depthMapShader);
	renderTrees1DepthMap(depthMapShader);
	renderTrees2DepthMap(depthMapShader);
	renderStreetLightDepthMap(depthMapShader);
	renderDogsDepthMap(depthMapShader);
	renderAirplaneDepthMap(depthMapShader);
	renderFenceDepthMap(depthMapShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, retina_width, retina_height);

	myCustomShader.useShaderProgram();

	//send lightSpace matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));

	//initialize the view matrix
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	//compute light direction transformation matrix
	lightDirMatrix = glm::mat3(glm::inverseTranspose(view));
	//send lightDir matrix data to shader
	glUniformMatrix3fv(lightDirMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightDirMatrix));

	//bind the depth map
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

	deltaSun+=0.01;
	if (deltaSun> 360)
		deltaSun -= 360;

	glm::mat4 m = glm::mat4(1);
	m = glm::rotate(m, deltaSun, glm::vec3(-1,0,0));
	//sau 15 15 2 1
	lightDir = glm::vec3(m * glm::vec4(0,15,15,1));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	skyboxShader.useShaderProgram();

	glUniform3fv(glGetUniformLocation(skyboxShader.shaderProgram, "lightDir"), 1, glm::value_ptr(lightDir));

	myCustomShader.useShaderProgram();

	float dotProduct = glm::dot(glm::normalize(lightDir), glm::vec3(0.0f, 1.0f, 0.0f));

	if (dotProduct<0) {
		lightColor = glm::vec3(0.0f, 0.0f, 0.0f); //dark light
		lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
		glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
	}
	else if (dotProduct < 0.1) {
		lightColor = glm::vec3(dotProduct/10, dotProduct/10, dotProduct/10); //day light
		lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
		glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
	}
	else
	{
		lightColor = glm::vec3(dotProduct, dotProduct, dotProduct); //day light
		lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
		glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
	}

	renderHouse(myCustomShader);
	renderGround(myCustomShader);
	renderTrees1(myCustomShader);
	renderTrees2(myCustomShader);
	renderStreetLight(myCustomShader);
	renderDogs(myCustomShader);
	renderAirplane(myCustomShader);
	renderFence(myCustomShader);

	renderSkybox();
}

int main(int argc, const char * argv[]) {

	initOpenGLWindow();
	initOpenGLState();
	initFBOs();
	initModels();
	initShaders();
	initUniforms();
	glCheckError();

	while (!glfwWindowShouldClose(glWindow)) {
		renderScene();

		if (startAnimation) {
			std::vector<glm::vec3> controlPoints;

			controlPoints.push_back(glm::vec3(0.0f, 5.0f, 70.0f));
			controlPoints.push_back(glm::vec3(115.0f, 5.0f, -70.0f));
			controlPoints.push_back(glm::vec3(-100.0f, 5.0f, -70.0f));
			controlPoints.push_back(glm::vec3(-20.0f, 5.0f, 40.0f));
			
			if (t <= (1 + (1.0f / totalTime)))
			{
				myCamera.moveAlongBezierCurve(controlPoints, t);
				t += 1.0f / (10*totalTime);
			}
			else {
				t = 0;
				startAnimation = false;

				/*std::vector<glm::vec3> controlPoints2;
				controlPoints2.push_back(glm::vec3(-25.0f, 5.0f, 30.0f));
				controlPoints2.push_back(glm::vec3(-20.0f, 5.0f, 40.0f));
				controlPoints2.push_back(glm::vec3(-20.0f, 5.0f, 40.0f));
				controlPoints2.push_back(glm::vec3(-20.0f, 5.0f, 40.0f));
				t = 1.0f / ( 25.0f);
				myCamera.moveAlongBezierCurve(controlPoints2, t);*/
			}
		}

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	//close GL context and any other GLFW resources
	glfwTerminate();

	return 0;
}
