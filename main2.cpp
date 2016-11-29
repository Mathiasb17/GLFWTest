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


int width = 1024;
int height = 768;

/**********************************************************************
 *                           BASIC SHADERS                            *
 **********************************************************************/

const char* vertex_shader_spheres =
"#version 400\n"
"uniform float pointScale;"
"uniform float pointRadius;"
"uniform mat4 MVP;"
"uniform mat4 MV;"
"in vec3 vp;"
"void main() {"
"  vec3 posEye = vec3(MVP*vec4(vp,1));"
"  float dist = length(posEye);"
"  gl_PointSize = 0.1 * (pointScale/dist);"
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
"in vec3 col;"
"out vec3 fcol;"
"void main() {"
"  fcol = col;"
"  gl_Position = vec4(vp, 1.0)*MVP;"
"}";

const char * fragment_shader_basic = 
"#version 400\n"
"in vec3 fcol;"
"out vec4 frag_colour;"
"void main() {"
//"  frag_colour = vec4(abs(gl_FragCoord.x)/1024, abs(gl_FragCoord.y)/768, 0.5, 1.0);"
"  frag_colour = vec4(fcol, 1.0);"
"}";

/**********************************************************************
 *                       SOME HELPER FUNCTIONS                        *
 **********************************************************************/

float particle_radius = 0.1;
glm::vec3 sphere_points[8]; glm::vec3 cube_points[36]; glm::vec3 cube_colors[36];

void initSpheres()
{
	//front face
	sphere_points[0] = glm::vec3(-1,-1,0); sphere_points[1] = glm::vec3(1,-1,0);
	sphere_points[2] = glm::vec3(-1,0,0); sphere_points[3] = glm::vec3(1,0,0);

	//back face
	sphere_points[4] = glm::vec3(-1,-1,1); sphere_points[5] = glm::vec3(1,-1,1);
	sphere_points[6] = glm::vec3(-1,0,1); sphere_points[7] = glm::vec3(1,0,1);
}

void initCube()
{
	//left face
	cube_points[0] = glm::vec3(-1,-1,1); cube_points[1] = glm::vec3(-1,-1,-1); cube_points[2] = glm::vec3(-1,1,-1);
	cube_points[3] = glm::vec3(-1,-1,1); cube_points[4] = glm::vec3(-1,1,1); cube_points[5] = glm::vec3(-1,1,-1);
	
	//right face
	cube_points[6] = glm::vec3(1,-1,1); cube_points[7] = glm::vec3(1,-1,-1); cube_points[8] = glm::vec3(1,1,-1);
	cube_points[9] = glm::vec3(1,-1,1); cube_points[10] = glm::vec3(1,1,1); cube_points[11] = glm::vec3(1,1,-1);

	//top face
	cube_points[12] = glm::vec3(-1,1,1); cube_points[13] = glm::vec3(1,1,1); cube_points[14] = glm::vec3(-1,1,-1);
	cube_points[15] = glm::vec3(1,1,1); cube_points[16] = glm::vec3(-1,1,-1); cube_points[17] = glm::vec3(1,1,-1);

	//bottom face
	cube_points[18] = glm::vec3(-1,-1,1); cube_points[19] = glm::vec3(1,-1,1); cube_points[20] = glm::vec3(-1,-1,-1);
	cube_points[21] = glm::vec3(1,-1,1); cube_points[22] = glm::vec3(-1,-1,-1); cube_points[23] = glm::vec3(1,-1,-1);

	//front face
	cube_points[24] = glm::vec3(-1,-1,1); cube_points[25] = glm::vec3(1,-1,1); cube_points[26] = glm::vec3(-1,1,1);
	cube_points[27] = glm::vec3(1,-1,1); cube_points[28] = glm::vec3(-1,1,1); cube_points[29] = glm::vec3(1,1,1);

	//back face
	cube_points[30] = glm::vec3(-1,-1,-1); cube_points[31] = glm::vec3(1,-1,-1); cube_points[32] = glm::vec3(-1,1,-1);
	cube_points[33] = glm::vec3(-1,1,-1); cube_points[34] = glm::vec3(-1,1,-1); cube_points[35] = glm::vec3(1,1,-1);

	//Colors

	//left face
	cube_colors[0] = glm::vec3(1,0,0); cube_colors[1] = glm::vec3(1,0,0); cube_colors[2] = glm::vec3(1,0,0);
	cube_colors[3] = glm::vec3(1,0,0); cube_colors[4] = glm::vec3(1,0,0); cube_colors[5] = glm::vec3(1,0,0);
	
	//right face
	cube_colors[6] = glm::vec3(1,0,0); cube_colors[7] = glm::vec3(1,0,0); cube_colors[8] = glm::vec3(1,0,0);
	cube_colors[9] = glm::vec3(1,0,0); cube_colors[10] = glm::vec3(1,0,0); cube_colors[11] = glm::vec3(1,0,0);

	//top face
	cube_colors[12] = glm::vec3(0,1,0); cube_colors[13] = glm::vec3(0,1,0); cube_colors[14] = glm::vec3(0,1,0);
	cube_colors[15] = glm::vec3(0,1,0); cube_colors[16] = glm::vec3(0,1,0); cube_colors[17] = glm::vec3(0,1,0);

	//bottom face
	cube_colors[18] = glm::vec3(0,1,0); cube_colors[19] = glm::vec3(0,1,0); cube_colors[20] = glm::vec3(0,1,0);
	cube_colors[21] = glm::vec3(0,1,0); cube_colors[22] = glm::vec3(0,1,0); cube_colors[23] = glm::vec3(0,1,0);

	//front face
	cube_colors[24] = glm::vec3(0,0,1); cube_colors[25] = glm::vec3(0,0,1); cube_colors[26] = glm::vec3(0,0,1);
	cube_colors[27] = glm::vec3(0,0,1); cube_colors[28] = glm::vec3(0,0,1); cube_colors[29] = glm::vec3(0,0,1);

	//back face
	cube_colors[30] = glm::vec3(0,0,1); cube_colors[31] = glm::vec3(0,0,1); cube_colors[32] = glm::vec3(0,0,1);
	cube_colors[33] = glm::vec3(0,0,1); cube_colors[34] = glm::vec3(0,0,1); cube_colors[35] = glm::vec3(0,0,1);
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
	glClearColor(0,0,0,1);
}

void getNewVbo(GLuint *newVbo, unsigned int bufferSize, const GLvoid* data, GLenum usage)
{
	glGenBuffers(1, newVbo);
	glBindBuffer(GL_ARRAY_BUFFER, *newVbo);
	glBufferData(GL_ARRAY_BUFFER, bufferSize, data, usage);
}

void getNewVao(GLuint *newVao, GLuint vbo_pos, GLuint vbo_col)
{
	glGenVertexArrays(1, newVao);
	glBindVertexArray(*newVao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_col);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
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

void displaySpheres(glm::mat4 mat_mvp, glm::mat4 mat_mv, GLuint shader_program, GLuint vao_pos)
{
	glUseProgram(shader_program);
	glBindVertexArray(vao_pos);

	glUniform1f(glGetUniformLocation(shader_program, "pointScale"), height / tanf(45.f*0.5f*(float)M_PI/180.0f));
	glUniform1f(glGetUniformLocation(shader_program, "pointRadius"), particle_radius);
	glUniformMatrix4fv(glGetUniformLocation(shader_program, "MVP"), 1, false, glm::value_ptr(mat_mvp));
	glUniformMatrix4fv(glGetUniformLocation(shader_program, "MV"), 1, false, glm::value_ptr(mat_mv));
	glDrawArrays(GL_POINTS, 0, 8); //TODO : replace 8 by a variable size

	glUseProgram(0);
}

void displayCube(glm::mat4 mat_mvp, GLuint shader_program, GLuint vao, GLuint vbo_pos, GLuint vbo_color)
{
	glUseProgram(shader_program);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glUniformMatrix4fv(glGetUniformLocation(shader_program, "MVP"), 1, false, glm::value_ptr(mat_mvp));

	glDrawArrays(GL_TRIANGLES, 0, 36); //TODO : replace 36 by a variable size
	glUseProgram(0);
}

/**********************************************************************
*                              GLOBALS                               *
**********************************************************************/

GLuint vbo_spheres, vao_spheres_pos;
GLuint vao_cube, vbo_cube_pos, vbo_cube_color;
GLuint vs_sphere, fs_sphere, vs_basic, fs_basic;

GLuint shader_program_spheres, shader_program_basic;

glm::mat4 mvp;


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

	glm::vec3 triangle[3]; 
	triangle[0] = glm::vec3(0,0,0);triangle[1] = glm::vec3(0,1,0);triangle[2] = glm::vec3(1,1,0);

	//opengl sphere buffers handling
	//getNewVbo(&vbo_spheres, 8 * sizeof(glm::vec3), sphere_points, GL_STATIC_DRAW);
	//getNewVao(&vao_spheres_pos, vbo_spheres, 0);

	//opengl cube buffers handling
	getNewVbo(&vbo_cube_pos, 36 * sizeof(glm::vec3), cube_points, GL_STATIC_DRAW);
	//getNewVbo(&vbo_cube_pos, vbo_cube, 0);
	getNewVbo(&vbo_cube_color, 36 * sizeof(glm::vec3), cube_colors, GL_STATIC_DRAW);

	getNewVao(&vao_cube, vbo_cube_pos, vbo_cube_color);
	
	//shaders handling
	compileVertexAndFragmentShaders(&vs_sphere, &fs_sphere, &vertex_shader_spheres, &fragment_shader_spheres);
	compileShaderProgram(&shader_program_spheres, vs_sphere, fs_sphere);
	compileVertexAndFragmentShaders(&vs_basic, &fs_basic, &vertex_shader_basic, &fragment_shader_basic);
	compileShaderProgram(&shader_program_basic, vs_basic, fs_basic);

	while(!glfwWindowShouldClose(window))
	{
		static float i = 1E-4f;
		//step 1 : clear screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//step 2 : handle mvp matrix
		glm::mat4 m(1.f);
		glm::mat4 v = glm::lookAt(glm::vec3(0,0,14), glm::vec3(0,0,0), glm::vec3(0,1,0));
		glm::mat4 p = glm::perspective(45.f,(float)width/float(height), 0.1f, 100.f);
		mvp = p*v*m;
		
		mvp = glm::rotate(mvp, i, glm::vec3(0,1,0));
		glm::mat4 mv = v*m;

		//step 3 : display spheres in associated shader program
		//displaySpheres(mvp, mv, shader_program_spheres, vao_spheres_pos);

		//step 4 : display cube in associated shader program
		displayCube(mvp, shader_program_basic, vao_cube, vbo_cube_pos, vbo_cube_color);

		//last step : read new events if some
		glfwPollEvents();
		glfwSwapBuffers(window);
		i+= 1E-4f;
	}

	glfwTerminate();
	return 0;

	exit(EXIT_SUCCESS);
}
