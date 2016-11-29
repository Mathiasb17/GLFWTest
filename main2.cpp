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
 *                WINDOW AND MOUSE/KEYBOARD PARAMETERS                *
 **********************************************************************/

GLFWwindow* window;

glm::vec3 points[8];

glm::vec3 cube_points[8];
unsigned int cube_indices[36];

int width = 1024;
int height = 768;

float particle_radius = 0.02;

float horizontalAngle = 0.f;
float verticalAngle = 0.f;

float mouseSpeed = 1.f;
float deltaTime = 0.000001f;

GLuint vbo_spheres, vao_spheres_pos;
GLuint vbo_cube, vao_cube_pos, vao_cube_indices;
GLuint vs_sphere, fs_sphere, vs_basic, fs_basic;

GLuint shader_program_spheres, shader_program_basic;

glm::mat4 mvp;

/**********************************************************************
 *                           BASIC SHADERS                            *
 **********************************************************************/

const char* vertex_shader_spheres =
"#version 400\n"
"uniform float pointScale;"
"uniform float pointRadius;"
"uniform mat4 MVP;"
"in vec3 vp;"
"void main() {"
"  gl_PointSize = 30;"
"  gl_Position = vec4(vp, 1.0)*MVP;"
"}";

const char* fragment_shader_spheres =
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

const char * vertex_shader_basic =
"#version 400\n"
"uniform mat4 MVP;"
"in vec3 vp;"
"void main(){"
"	gl_Position = vec4(vp, 1)* MVP;"
"}";

const char * fragment_shader_basic = 
"#version 400\n"
"#out vec4 frag_colour;"
"void main(){"
"frag_colour = vec4(0,0,1,1);"
"}";

/**********************************************************************
 *                       SOME HELPER FUNCTIONS                        *
 **********************************************************************/

void initSpheres()
{
	//left face
	points[0] = glm::vec3(-1,-1,0);
	points[1] = glm::vec3(-1,1,0);
	points[2] = glm::vec3(-1.4,-1,-1);
	points[3] = glm::vec3(-1.4,1,-1);

	//right face
	points[4] = glm::vec3(1,-1,0);
	points[5] = glm::vec3(1,1,0);
	points[6] = glm::vec3(1.4,-1,-1);
	points[7] = glm::vec3(1.4,1,-1);
}

void initCube()
{
	//face gauche
	cube_points[0] = glm::vec3(-1,-1,0);
	cube_points[1] = glm::vec3(-1,-1,-1);
	cube_points[2] = glm::vec3(-1,1,-1);
	cube_points[3] = glm::vec3(-1,1,0);

	//face droite
	cube_points[4] = glm::vec3(1,-1,0);
	cube_points[5] = glm::vec3(1,-1,-1);
	cube_points[6] = glm::vec3(1,1,-1);
	cube_points[7] = glm::vec3(1,1,0);
}

void initWindow()
{
	if (!glfwInit())
	{
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		exit(1);
	} 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	window = glfwCreateWindow(width, height, "Hello Spheres", NULL, NULL);
	if (!window)
	{
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		exit(1);
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
}

void glEnableCapabilities()
{
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glClearColor(1,1,1,1);
}

void getNewVbo(GLuint *newVbo, unsigned int bufferSize, const GLvoid* data, GLenum usage)
{
	glGenBuffers(1, newVbo);
	glBindBuffer(GL_ARRAY_BUFFER, *newVbo);
	glBufferData(GL_ARRAY_BUFFER, bufferSize, data, usage);
}

void getNewVao(GLuint *newVao, GLuint vbo, int attribNb)
{
	glGenVertexArrays(1, newVao);
	glBindVertexArray(*newVao);
	glEnableVertexAttribArray(attribNb);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

}

void compileVertexAndFragmentShaders(GLuint *vs, GLuint *fs, const GLchar **string_vs, const GLchar **string_fs)
{
	*vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(*vs, 1, string_vs, NULL);
	glCompileShader(*vs);
	*fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(*fs, 1, string_fs, NULL);
	glCompileShader(*fs);
}

void compileShaderProgram(GLuint *sp, GLuint vs, GLuint fs)
{
	*sp = glCreateProgram();
	glAttachShader(*sp, fs);
	glAttachShader(*sp, vs);
	glLinkProgram(*sp);
}

void displaySpheres(glm::mat4 mat_mvp, GLuint shader_program, GLuint vao_pos)
{
	glUseProgram(shader_program);
	glBindVertexArray(vao_pos);

	glUniform1f(glGetUniformLocation(shader_program, "pointScale"), height / tanf(45.f*0.5f*(float)M_PI/180.0f));
	glUniform1f(glGetUniformLocation(shader_program, "pointRadius"), particle_radius);
	glUniformMatrix4fv(glGetUniformLocation(shader_program, "MVP"), 1, false, glm::value_ptr(mat_mvp));
	glDrawArrays(GL_POINTS, 0, 8); //TODO : replace 8 by a variable size

	glUseProgram(0);
}

void displayCube(glm::mat4 mat_mvp, GLuint shader_program, GLuint vao_pos)
{
	glUseProgram(shader_program);
	glUseProgram(0);
}

/**********************************************************************
 *                            MAIN PROGRAM                            *
 **********************************************************************/

int main(void)
{
	//call to helper functions
	initWindow();
	glEnableCapabilities();
	initSpheres();
	initCube();

	//opengl sphere buffers handling
	getNewVbo(&vbo_spheres, 8 * sizeof(glm::vec3), points, GL_STATIC_DRAW);
	getNewVao(&vao_spheres_pos, vbo_spheres, 0);

	//shaders handling
	compileVertexAndFragmentShaders(&vs_sphere, &fs_sphere, &vertex_shader_spheres, &fragment_shader_spheres);
	compileShaderProgram(&shader_program_spheres, vs_sphere, fs_sphere);
	compileVertexAndFragmentShaders(&vs_basic, &fs_basic, &vertex_shader_basic, &fragment_shader_basic);
	compileShaderProgram(&shader_program_basic, vs_basic, fs_basic);

	while(!glfwWindowShouldClose(window))
	{
		//step 1 : clear screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//step 2 : handle mvp matrix
		glm::mat4 m(1.f);
		glm::mat4 v = glm::lookAt(glm::vec3(0,0,5), glm::vec3(0,0,0), glm::vec3(0,1,0));
		glm::mat4 p = glm::perspective(45.f,(float)width/float(height), -1.0f, 100.f);
		mvp = p*v*m;

		//step 3 : display spheres in associated shader program
		displaySpheres(mvp, shader_program_spheres, vao_spheres_pos);

		//step 4 : display cube in associated shader program

		//last step : read new events if some
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;

	exit(EXIT_SUCCESS);
}
