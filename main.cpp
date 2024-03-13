
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"
#include "Window.h";

#include <iostream>

gps::Window myWindow;

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

int glWindowWidth = 800;
int glWindowHeight = 600;
GLFWwindow* glWindow = NULL;

float lastX = glWindowWidth / 2.0f;
float lastY = glWindowHeight / 2.0f;
bool firstMouse = true;

const unsigned int SHADOW_WIDTH = 8192;
const unsigned int SHADOW_HEIGHT = 8192;

glm::mat4 model;
glm::mat4 model2;

GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;
glm::mat4 r_matrix;

glm::mat4 light2;
glm::vec3 light2Dir;
GLuint light2DirLoc;
glm::vec3 light2Color;
GLuint light2ColorLoc;



glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

gps::Camera myCamera(
	glm::vec3(0.0f, 2.0f, 5.5f),
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.4f;
float mouse_sensitivity = 0.05f;
float rotationSpeed = 0.7f;
float cameraAngle = 270;
float yaw = -90, pitch;

bool pressedKeys[1024];
GLfloat angle;
GLfloat anglePropiller;
GLfloat angleX;
GLfloat angleY;
GLfloat lightAngle;
GLfloat changeLightAngle=0.25f;
GLfloat lightIntensity = 1.0f;

gps::Model3D truck;
gps::Model3D ground;
gps::Model3D lightCube;
gps::Model3D cube2;
gps::Model3D scene;
gps::Model3D flowers;
gps::Model3D propiller;
gps::Model3D chicken;
gps::Model3D lady;

gps::Shader myBasicShader;
gps::Shader lightShader;
gps::Shader depthMapShader;


GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;
glm::mat4 truckModel;
glm::mat3 truckNormalMatrix;
glm::mat4 flowersModel;
glm::mat3 flowersNormalMatrix;
glm::mat4 propillerModel;
glm::mat3 propillerNormalMatrix;
glm::mat4 chickenModel;
glm::mat3 chisckenNormalMatrix;
glm::mat4 ladyModel;
glm::mat3 ladyNormalMatrix;
float truckSlide = 0.25f;
float truckStarting = 0.0f;

float startValueFlower = -3.0f;
float flowerSize = 0.1f;

float propillerStartValue = 0.0f;
float propillerRotation = 0.10f;

GLfloat intitalPoz = 0.0f;
float translationAmountZ;
float translationAmountX;

float fogDens = 0;
GLint logFogDens;

const float maxRotation = 360.0f; 
const float rotationIncrement = 90.0f;

bool cameraMovement = false;


GLenum glCheckError_(const char* file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	myBasicShader.useShaderProgram();
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 5000.0f);
	glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glViewport(0, 0, width, height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}


void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	int window_width, window_height;
	glfwGetWindowSize(window, &window_width, &window_height);

	float xoffset = xpos - window_width / 2;
	float yoffset = window_height / 2 - ypos;

	xoffset *= mouse_sensitivity;
	yoffset *= mouse_sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f) {
		pitch = 89.0f;
	}
	if (pitch < -89.0f) {
		pitch = -89.0f;
	}

	myCamera.rotate(pitch, yaw);

	view = myCamera.getViewMatrix();
	myBasicShader.useShaderProgram();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	glfwSetCursorPos(window, window_width / 2, window_height / 2);
}

void cameraMove()
{
	
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed / 3);
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		yaw -= 0.33f;
		myCamera.rotate(pitch, yaw);
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	
}


void renderLady(gps::Shader shader) {
	// Select active shader program
	shader.useShaderProgram();

	//chicken_model = glm::translate(chicken_model, glm::vec3(0.0f, 1.0f, 0.0f));

	// Send teapot model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(ladyModel));

	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(ladyNormalMatrix));
	// Draw flowers
	lady.Draw(shader);
}

void processMovement()
{
	if (pressedKeys[GLFW_KEY_G]) {
	cameraMovement = false;
}
	if (pressedKeys[GLFW_KEY_H]) {
		cameraMovement = true;
	}

	if (pressedKeys[GLFW_KEY_P])
	{
		cameraAngle += rotationSpeed;
		if (cameraAngle > 360.0f)
			cameraAngle -= 360.0f;

		myCamera.rotate(0, cameraAngle);

		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	// rotate camera to the left
	if (pressedKeys[GLFW_KEY_O])
	{
		cameraAngle -= rotationSpeed;
		if (cameraAngle < 0.0f)
			cameraAngle += 360.0f;
		myCamera.rotate(0, cameraAngle);

		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}


	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	/*if (pressedKeys[GLFW_KEY_Q]) {
		angle -= 1.0f;
	// update model matrix for teapot
		teapot_model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, translationAmount));
		teapot_model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
		// update normal matrix for teapot
		teapot_normal_matrix = glm::mat3(glm::inverseTranspose(view * teapot_model));
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angle += 1.0f;
		// update model matrix for teapot
		teapot_model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, translationAmount));
		teapot_model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
		// update normal matrix for teapot
		teapot_normal_matrix = glm::mat3(glm::inverseTranspose(view * teapot_model));
	}*/

	if (pressedKeys[GLFW_KEY_Q]) {
		angle -= rotationIncrement;

		// Ensure the rotation stays within the allowed range
		if (angle < 0.0f)
			angle += maxRotation;
		std::cout << angle;
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angle += rotationIncrement;

		// Ensure the rotation stays within the allowed range
		if (angle >= maxRotation)
			angle -= maxRotation;

		std::cout << angle;
	}


	r_matrix = glm::mat4(1.0f);

	if (pressedKeys[GLFW_KEY_Z])
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	if (pressedKeys[GLFW_KEY_X])
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}
	if (pressedKeys[GLFW_KEY_C])
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	//light
	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle -= 1.0f;
		model = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0, 1, 0));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 1.0f;
		model = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0, 1, 0));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_V]) {

		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		lightColor = lightColor * glm::vec3(1.0f, 0.0f, 0.0f);  // Red light

		glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	}


	if (pressedKeys[GLFW_KEY_UP]) {

		truckStarting += 1.0f;
		if (angle == 0) {
			translationAmountZ = truckStarting * 0.15f;
			//translationAmountX = 0.0f;
		}
		else if (angle == 180) {
			translationAmountX = truckStarting * 0.15f;
			//translationAmountZ = 0.0f;
		}
		else if (angle == 270) {
			translationAmountX = truckStarting * -0.15f;
			//translationAmountZ = 0.0f;
		}
		else if (angle == 90) {
			translationAmountZ = truckStarting * -0.15f;
			//translationAmountX = 0.0f;
		}
		//truckModel = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
		//truckModel = glm::translate(truckModel, glm::vec3(translationAmount * 0.1f, 0.0f, translationAmount * 0.1f));
		//truckNormalMatrix = glm::mat3(glm::inverseTranspose(view * truckModel));

	}

	if (pressedKeys[GLFW_KEY_DOWN]) {
		truckStarting -= 1.0f;

		// Update model matrix

		if (angle == 0) {
			translationAmountZ = truckStarting * 0.15f;
			//translationAmountX = 0.0f;
		}
		else if (angle == 180) {
			translationAmountX = truckStarting * 0.15f;
			//translationAmountZ = 0.0f;
		}
		else if (angle == 270) {
			translationAmountX = truckStarting * -0.15f;
			//translationAmountZ = 0.0f;
		}
		else if (angle == 90) {
			translationAmountZ = truckStarting * -0.15f;
			//translationAmountX = 0.0f;
		}
		
		//truckModel = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
		//truckModel = glm::translate(truckModel, glm::vec3(translationAmount * 0.1f, 0.0f, translationAmount * 0.1f));
		//truckNormalMatrix = glm::mat3(glm::inverseTranspose(view * truckModel));
	}


	if (pressedKeys[GLFW_KEY_F]) {


		fogDens += 0.003;
		fogDens = fogDens > 0.02 ? 0 : fogDens;
		myBasicShader.useShaderProgram();

		logFogDens = glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity");
		glUniform1f(logFogDens, (GLfloat)fogDens);
	}

	if (pressedKeys[GLFW_KEY_B]) {

		ladyModel = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		ladyModel = glm::translate(ladyModel, glm::vec3(0.0f, 0.0f, 0.0f));
		chisckenNormalMatrix = glm::mat3(glm::inverseTranspose(view * ladyModel));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(ladyNormalMatrix));
		renderLady(myBasicShader);
	}


}

void initOpenGLWindow()
{
	myWindow.Create(1229, 768, "OpenGL Project Core");

	
}

void setWindowCallbacks()
{
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
	glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
	glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);

}

void initOpenGLState()
{
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
	ground.LoadModel("objects/ground/grass.obj");
	lightCube.LoadModel("objects/sun/sun.obj");
	truck.LoadModel("objects/truck/truck.obj");
	scene.LoadModel("objects/scene/scene1.obj");
	flowers.LoadModel("objects/flowers/flowers.obj");
	propiller.LoadModel("objects/propiler/propiler.obj");
	chicken.LoadModel("objects/chicken/chicken.obj");
	lady.LoadModel("objects/lady/lady2.obj");
}

void initShaders() {
	myBasicShader.loadShader("shaders/basic.vert", "shaders/basic.frag");
	myBasicShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	depthMapShader.loadShader("shaders/depthMapShader.vert", "shaders/depthMapShader.frag");
	depthMapShader.useShaderProgram();
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
}

void initUniforms() {
	myBasicShader.useShaderProgram();

	//teapot_model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));

	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projection = glm::perspective(glm::radians(45.0f),
		(float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
		0.1f, 300.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::normalize(glm::vec3(1.0f, 1.0f, -1.0f)); // Example direction
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f);  //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
	
	//second light
	light2Dir = glm::vec3(1.0f, 0.0f, 0.0f);
	//lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	light2DirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "light2Dir");
	//glUniform3fv(light2DirLoc, 1, glm::value_ptr(light2Dir));
	glUniform3fv(light2DirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * light2Dir));


	light2Color = glm::vec3(1.0f, 0.0f, 0.0f);
	light2ColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "light2Color");
	glUniform3fv(light2ColorLoc, 1, glm::value_ptr(light2Color));

	logFogDens = glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity");
	glUniform1f(logFogDens, (GLfloat)fogDens);

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void initFBO() {
	// generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	// create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

	// Set the draw buffer to none since we don't need color information
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


glm::mat4 computeLight() {
	glm::mat4 lightView = glm::lookAt(glm::mat3(lightRotation) * lightDir, glm::vec3(0.0f), glm::vec3(0.0f, -1.5f, 0.0f));
	const GLfloat near_plane = 0.1f, far_plane = 20.0f; // Adjust far_plane to increase the range
	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

	return lightSpaceTrMatrix;
}


glm::mat4 computeLight2SpaceTrMatrix() {
	glm::mat4 lightView = glm::lookAt(light2Dir, glm::vec3(0.0f), glm::vec3(0.0f, -1.5f, 0.0f));
	const GLfloat near_plane = 0.1f, far_plane = 5.0f;
	glm::mat4 lightProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

	return lightSpaceTrMatrix;
}

void renderTruck(gps::Shader shader) {
	// select active shader program
	shader.useShaderProgram();

	truckModel = glm::translate(truckModel, glm::vec3(0.0f, 1.0f, 0.0f));
	//send teapot model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(truckModel));


	// draw teapot
	truck.Draw(shader);
}
void renderGround(gps::Shader shader) {

	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -15.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));


	ground.Draw(shader);

}



void renderFlowers(gps::Shader shader) {
	// Select active shader program
	shader.useShaderProgram();

	//flowers_model = glm::translate(flowers_model, glm::vec3(0.0f, 3.0f, 0.0f));
	// Send teapot model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(flowersModel));

	// Draw flowers
	flowers.Draw(shader);
}



void renderSceneObj(gps::Shader shader) {
	// select active shader program
	shader.useShaderProgram();

	// send scene model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	// send scene normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw teapot
	scene.Draw(shader);

}

void renderPropiller(gps::Shader shader) {
	// Select active shader program

	glm::mat4 start(1.0f);
	shader.useShaderProgram();
	propillerModel = glm::mat4(1.0f);

	float PozX = 17.3f;
	float PozY = 52.079f;
	float PozZ = 26.923f;

	// Get the current time
	float time = glfwGetTime();

	// Calculate the rotation angle based on time
	float rotationSpeed = 10.0f;
	float Prangle = glm::radians(rotationSpeed * time);

	propillerModel = glm::translate(propillerModel, glm::vec3(PozX, PozZ, PozY));
	propillerModel = glm::rotate(propillerModel, Prangle, glm::vec3(0.0f, 0.0f, 1.0f));
	propillerModel = glm::translate(propillerModel, glm::vec3(-PozX, -PozZ, -PozY));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(propillerModel));

	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * propillerModel))));

	propiller.Draw(shader);
}

void renderChicken(gps::Shader shader) {
	// Select active shader program
	shader.useShaderProgram();

	//chicken_model = glm::translate(chicken_model, glm::vec3(0.0f, 1.0f, 0.0f));

	// Send teapot model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(chickenModel));

	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(chisckenNormalMatrix));
	// Draw flowers
	chicken.Draw(shader);
}



void renderScene() {
	// Depth Map Rendering
	depthMapShader.useShaderProgram();

	glm::mat4 model_shadow = glm::rotate(glm::mat4(1.0f), glm::radians((GLfloat)0), glm::vec3(0.0, 1.0, 0.0));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model_shadow));
	renderSceneObj(depthMapShader);

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLight()));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "light2SpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLight2SpaceTrMatrix()));

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);



	// Render objects casting shadows
	truckModel = glm::translate(glm::mat4(1.0f), glm::vec3(translationAmountX, 0.0f, translationAmountZ));
	truckModel = glm::rotate(truckModel, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	//truckModel = glm::translate(truckModel, glm::vec3(0.0f, 0.0f, -translationAmount));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "truck_model"), 1, GL_FALSE, glm::value_ptr(truckModel));
	renderTruck(depthMapShader);

	flowersModel = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	flowersModel = glm::translate(flowersModel, glm::vec3(0.0f, startValueFlower, 0.0f));
	flowersNormalMatrix = glm::mat3(glm::inverseTranspose(view * flowersModel));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "flowers_model"), 1, GL_FALSE, glm::value_ptr(flowersModel));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(flowersNormalMatrix));

	// Draw flowers with shadows
	renderFlowers(depthMapShader);

	//renderGround(depthMapShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Main Scene Rendering
	glViewport(0, 0, (int)myWindow.getWindowDimensions().width, (int)myWindow.getWindowDimensions().height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	myBasicShader.useShaderProgram();

	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	model = glm::rotate(glm::mat4(1.0f), glm::radians((GLfloat)0), glm::vec3(15.0f, 15.0f, 15.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
	renderSceneObj(myBasicShader);

	// Set Light Uniforms for the main scene
	glUniform3fv(glGetUniformLocation(lightShader.shaderProgram, "lightDir"), 1, glm::value_ptr(lightDir));
	glUniform3fv(glGetUniformLocation(lightShader.shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));

	// Light Rotation and Teapot Rendering
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0, 1, 0));
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));


	// Bind the shadow map
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

	glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLight()));

	// Update normal matrix for truck
	truckModel = glm::translate(glm::mat4(1.0f), glm::vec3(translationAmountX, 0.0f, translationAmountZ));
	truckModel = glm::rotate(truckModel, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	//truckModel = glm::translate(truckModel, glm::vec3(0.0f, 0.0f, -translationAmount));
	renderTruck(myBasicShader);
	

	//flowers
	flowersModel = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	//flowers_model = glm::scale(flowers_model, glm::vec3(startValueFlower, startValueFlower, startValueFlower));
	flowersModel = glm::translate(flowersModel, glm::vec3(0.0f, startValueFlower, 0.0f));
	flowersNormalMatrix = glm::mat3(glm::inverseTranspose(view * flowersModel));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(flowersNormalMatrix));
	renderFlowers(myBasicShader);

	startValueFlower += flowerSize;
	if (startValueFlower > 0.1)
		flowerSize = -flowerSize;
	if (startValueFlower < -3.0)
		flowerSize = -flowerSize;

	//propiller
	renderPropiller(myBasicShader);

	//chicken
	chickenModel = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	chickenModel = glm::translate(chickenModel, glm::vec3(0.0f, 0.0f, 0.0f));
	chisckenNormalMatrix = glm::mat3(glm::inverseTranspose(view * chickenModel));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(chisckenNormalMatrix));
	renderChicken(myBasicShader);


	//lady
	ladyModel = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	ladyModel = glm::translate(ladyModel, glm::vec3(0.0f, 0.0f, 0.0f));
	ladyNormalMatrix = glm::mat3(glm::inverseTranspose(view * ladyModel));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(ladyNormalMatrix));
	renderLady(myBasicShader);

	// Draw a white cube around the light
	lightShader.useShaderProgram();


	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniform3fv(light2DirLoc, 1, glm::value_ptr(glm::inverseTranspose(view)));

	// Set polygon mode to GL_LINE for wireframe rendering
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Render the light cube
	model = lightRotation;
	model = glm::translate(model, glm::vec3(0.0f, 30.0f, 0.0f) * lightDir);
	model = glm::scale(model, glm::vec3(1.15f, 1.15f, 1.15f));
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	lightCube.Draw(lightShader);

	// Revert polygon mode back to GL_FILL
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	mySkyBox.Draw(skyboxShader, view, projection,fogDens);
}


void cleanup() {
	glDeleteTextures(1, &depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

void initSkyBox()
{
	std::vector<const GLchar*> faces;
	faces.push_back("skybox/posx.jpg");
	faces.push_back("skybox/negx.jpg");
	faces.push_back("skybox/posy.jpg");
	faces.push_back("skybox/negy.jpg");
	faces.push_back("skybox/posz.jpg");
	faces.push_back("skybox/negz.jpg");
	mySkyBox.Load(faces);
}

int main(int argc, const char* argv[]) {

	try
	{
		initOpenGLWindow();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	r_matrix = glm::mat4(1.0f);

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();
	initSkyBox();
	setWindowCallbacks();

	glCheckError();


	while (!glfwWindowShouldClose(myWindow.getWindow()))
	{
		if (cameraMovement == true) {
			cameraMove();
		}
		processMovement();
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

	}

	cleanup();

	return 0;
}
