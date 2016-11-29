#define GLM_FORCE_RADIANS
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
//#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>


/**********************************************************************
 *                       SOME HELPER FUNCTIONS                        *
 **********************************************************************/



/**********************************************************************
 *                           BASIC SHADERS                            *
 **********************************************************************/


const char* vertex_shader =
"#version 400\n"
"uniform float pointScale;"
"uniform float pointRadius;"
"uniform mat4 MVP;"
"in vec3 vp;"
"void main() {"
"  gl_PointSize = 30;"
"  gl_Position = vec4(vp, 1.0)*MVP;"
"}";

const char* fragment_shader =
"#version 400\n"
"const float PI = 3.1415926535897932384626433832795;"
"out vec4 frag_colour;"
"void main() "
"{"
"if(dot(gl_PointCoord-0.5,gl_PointCoord-0.5)>0.25) "
"discard;"
"else"
"{"
"vec3 lightDir = vec3(0.3,0.3,0.9);"
"vec3 N;"
"N.xy = gl_PointCoord* 2.0 - vec2(1.0);"
"float mag = dot(N.xy, N.xy);"
"N.z = sqrt(1.0-mag);"
"float diffuse = max(0.0, dot(lightDir, N));"
"frag_colour = vec4(1.0, 0.0, 0.0, 1.0)*diffuse;"
"}"
"}";

/**********************************************************************
 *                WINDOW AND MOUSE/KEYBOARD PARAMETERS                *
 **********************************************************************/

glm::vec3 points[8];

int width = 640;
int height = 480;

float particle_radius = 0.02;

float horizontalAngle = 0.f;
float verticalAngle = 0.f;

float mouseSpeed = 1.f;
float deltaTime = 0.000001f;

/**********************************************************************
 *                            MAIN PROGRAM                            *
 **********************************************************************/

int main(void)
{
	if (!glfwInit())
	{
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return 1;
	} 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	GLFWwindow* window = glfwCreateWindow(width, height, "Hello Spheres", NULL, NULL);
	if (!window)
	{
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"

	//face gauche
	points[0] = glm::vec3(0,0,0);
	points[1] = glm::vec3(0,1,0);
	points[2] = glm::vec3(0.4,0,-1);
	points[3] = glm::vec3(0.4,1,-1);

	//face droite
	points[4] = glm::vec3(1,0,0);
	points[5] = glm::vec3(1,1,0);
	points[6] = glm::vec3(1.4,0,-1);
	points[7] = glm::vec3(1.4,1,-1);

	GLuint vbo = 0;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(glm::vec3), points, GL_STATIC_DRAW);

	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertex_shader, NULL);
	glCompileShader(vs);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragment_shader, NULL);
	glCompileShader(fs);

	GLuint shader_programme = glCreateProgram();
	glAttachShader(shader_programme, fs);
	glAttachShader(shader_programme, vs);
	glLinkProgram(shader_programme);

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

	glClearColor(1,1,1,1);

	glm::vec3 direction(0,0,1), right, up;
	glm::vec3 position(0,0,3);
	float diff_x = 0.f;
	float diff_y = 0.f;
	float sensitivity = 0.00001f;

	while(!glfwWindowShouldClose(window))
	{
		static float i = 0.0001f;
		// Reset mouse position for next frame

		// wipe the drawing surface clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shader_programme);
		glBindVertexArray(vao);

		glm::mat4 m(1.f);
		glm::mat4 v = glm::lookAt(position, glm::vec3(0,0,0), glm::vec3(0,1,0));
		glm::mat4 p = glm::perspective(45.f,(float)width/float(height), -1.0f, 100.f);

		glm::mat4 mvp = p*v*m;


		glUniform1f(glGetUniformLocation(shader_programme, "pointScale"), height / tanf(45.f*0.5f*(float)M_PI/180.0f));
		glUniform1f(glGetUniformLocation(shader_programme, "pointRadius"), particle_radius);
		glUniformMatrix4fv(glGetUniformLocation(shader_programme, "MVP"), 1, false, glm::value_ptr(mvp));

		// draw points 0-3 from the currently bound VAO with current in-use shader
		glDrawArrays(GL_POINTS, 0, 8);
		// update other events like input handling 
		glfwPollEvents();
		// put the stuff we've been drawing onto the display
		glfwSwapBuffers(window);
		i+= 0.0001f;
	}

	glfwTerminate();
	return 0;

	exit(EXIT_SUCCESS);
}
