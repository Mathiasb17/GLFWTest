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
"in vec3 col;"
"out vec3 fcol;"
"void main() {"
"  vec3 posEye = vec3(MVP*vec4(vp,1));"
"  float dist = length(posEye);"
"  gl_PointSize = 0.02 * (pointScale/dist);"
"  gl_Position = MVP*vec4(vp, 1.0);"
"  fcol = col;"
"}";

const char* fragment_shader_spheres =
"#version 400\n"
"const float PI = 3.1415926535897932384626433832795;"
"out vec4 frag_colour;"
"in vec3 fcol;"
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
"frag_colour = vec4(fcol, 1.0)*diffuse;"
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
"  gl_Position = MVP*vec4(vp, 1.0);"
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
glm::vec3 sphere_points[8], sphere_colors[8];
glm::vec3 cube_points[8];glm::vec3 cube_colors[8]; unsigned int cube_indices[36];

void initSpheres()
{
	//front face
	sphere_points[0] = glm::vec3(-0.5,-0.5,-0.5); sphere_points[1] = glm::vec3(-0.5,-0.5,0.5);
	sphere_points[2] = glm::vec3(-0.5,0,0); sphere_points[3] = glm::vec3(1,0,0);

	//back face
	sphere_points[4] = glm::vec3(-0.5,-0.5,1); sphere_points[5] = glm::vec3(1,-0.5,1);
	sphere_points[6] = glm::vec3(-0.5,0,1); sphere_points[7] = glm::vec3(1,0,1);

	sphere_colors[0] = glm::vec3(1,1,1);
	sphere_colors[1] = glm::vec3(0,1,1);
	sphere_colors[2] = glm::vec3(1,1,0);
	sphere_colors[3] = glm::vec3(1,0,0);
	sphere_colors[4] = glm::vec3(0,1,0);
	sphere_colors[5] = glm::vec3(0,0,1);
	sphere_colors[6] = glm::vec3(0,1,1);
	sphere_colors[7] = glm::vec3(1,0.5,1);
}

void initCube()
{
	cube_points[0] = glm::vec3(-0.5,-0.5,0.5);
	cube_points[1] = glm::vec3(-0.5,-0.5,-0.5);
	cube_points[2] = glm::vec3(-0.5,0.5,-0.5);
	cube_points[3] = glm::vec3(-0.5,0.5,0.5);
	cube_points[4] = glm::vec3(0.5,-0.5,0.5);
	cube_points[5] = glm::vec3(0.5,-0.5,-0.5);
	cube_points[6] = glm::vec3(0.5,0.5,-0.5);
	cube_points[7] = glm::vec3(0.5,0.5,0.5);

	cube_colors[0] = glm::vec3(1,0,0);
	cube_colors[1] = glm::vec3(1,0,0);
	cube_colors[2] = glm::vec3(0,1,0);
	cube_colors[3] = glm::vec3(0,1,0);
	cube_colors[4] = glm::vec3(0,0,1);
	cube_colors[5] = glm::vec3(0,0,1);
	cube_colors[6] = glm::vec3(1,0,0);
	cube_colors[7] = glm::vec3(1,0,0);

	//front
	cube_indices[0] = 0; cube_indices[1] = 4; cube_indices[2] = 3;
	cube_indices[3] = 4; cube_indices[4] = 3; cube_indices[5] = 7;

	//back
	cube_indices[6] = 1; cube_indices[7] = 5; cube_indices[8] = 2;
	cube_indices[9] = 5; cube_indices[10] = 2; cube_indices[11] = 6;

	//left
	cube_indices[12] = 0; cube_indices[13] = 1; cube_indices[14] = 2;
	cube_indices[15] = 0; cube_indices[16] = 2; cube_indices[17] = 3;

	//right
	cube_indices[18] = 4; cube_indices[19] = 5; cube_indices[20] = 6;
	cube_indices[21] = 4; cube_indices[22] = 6; cube_indices[23] = 7;

	//top
	cube_indices[24] = 3; cube_indices[25] = 7; cube_indices[26] = 6;
	cube_indices[27] = 3; cube_indices[28] = 6; cube_indices[29] = 2;

	//bottom
	cube_indices[30] = 0; cube_indices[31] = 4; cube_indices[32] = 5;
	cube_indices[33] = 0; cube_indices[34] = 5; cube_indices[35] = 1;
	
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

void getNewVbo(GLenum target, GLuint *newVbo, unsigned int bufferSize, const GLvoid* data, GLenum usage)
{
	glGenBuffers(1, newVbo);
	glBindBuffer(target, *newVbo);
	glBufferData(target, bufferSize, data, usage);
}

void getNewVao(GLuint *newVao, GLuint vbo_pos)
{
	glGenVertexArrays(1, newVao);
	glBindVertexArray(*newVao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
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

void getNewVao(GLuint *newVao, GLuint vbo_pos, GLuint vbo_col, GLuint vbo_indices)
{
	glGenVertexArrays(1, newVao);
	glBindVertexArray(*newVao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_col);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_indices);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
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

void displaySpheres(glm::mat4 mat_mvp, glm::mat4 mat_mv, GLuint shader_program, GLuint vao, GLuint vbo_pos)
{
	glUseProgram(shader_program);

	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glUniform1f(glGetUniformLocation(shader_program, "pointScale"), height / tanf(45.f*0.5f*(float)M_PI/180.0f));
	glUniform1f(glGetUniformLocation(shader_program, "pointRadius"), particle_radius);
	glUniformMatrix4fv(glGetUniformLocation(shader_program, "MVP"), 1, false, glm::value_ptr(mat_mvp));
	glUniformMatrix4fv(glGetUniformLocation(shader_program, "MV"), 1, false, glm::value_ptr(mat_mv));
	glDrawArrays(GL_POINTS, 0, 8); //TODO : replace 8 by a variable size

	glUseProgram(0);
}

void displayCube(glm::mat4 mat_mvp, GLuint shader_program, GLuint vao, GLuint vbo_pos, GLuint vbo_color, GLuint vbo_indices)
{
	glUseProgram(shader_program);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glUniformMatrix4fv(glGetUniformLocation(shader_program, "MVP"), 1, false, glm::value_ptr(mat_mvp));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_indices);

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0); //TODO : replace 36 by a variable size
	glUseProgram(0);
}

/**********************************************************************
*                              GLOBALS                               *
**********************************************************************/

GLuint vbo_spheres_pos, vbo_spheres_col, vao_spheres;
GLuint vao_cube, vbo_cube_pos, vbo_cube_color, vbo_cube_indices;
GLuint vs_sphere, fs_sphere, vs_basic, fs_basic;

GLuint shader_program_spheres, shader_program_basic;

glm::mat4 mvp;
float fov = 45.f;
glm::vec3 campos(4,0,0);

/**********************************************************************
 *                            KEY CALLBACK                            *
 **********************************************************************/

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
        glfwSetWindowShouldClose(window, GLFW_TRUE);
		std::cout << "bye !" << std::endl;
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	fov += yoffset*0.02;
}

void move_camera_direction(GLFWwindow* win, glm::vec3* dir)
{
	int mouse_left_state = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT);
	if (mouse_left_state == GLFW_PRESS || mouse_left_state == GLFW_REPEAT)
	{
		double xcenter = 1024. / 2.;
		double ycenter = 768. / 2.;
		double xpos, ypos;
		glfwGetCursorPos(win, &xpos, &ypos);

		if (xpos < xcenter)
		{
			dir->x -= 0.01f;
		}

		if (xpos > xcenter)
		{
			dir->x += 0.01f;
		}

		if (ypos < ycenter)
		{
			dir->y += 0.01f;
		}

		if (ypos > ycenter)
		{
			dir->y -= 0.01f;
		}

		glfwSetCursorPos(win, 1024/2, 768/2);
	}

}

void move_camera_rotate(GLFWwindow * win, glm::mat4 *mvp)
{
	static float rotateAroundY = 0.f;
	static float rotateAroundX = 0.f;

	int mouse_left_state = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT);
	if (mouse_left_state == GLFW_PRESS || mouse_left_state == GLFW_REPEAT)
	{
		double xcenter = 1024. / 2.;
		double ycenter = 768. / 2.;
		double xpos, ypos;
		glfwGetCursorPos(win, &xpos, &ypos);


		if (xpos < xcenter)
		{
			rotateAroundY -= 0.005f;
		}

		if (xpos > xcenter)
		{
			rotateAroundY += 0.005f;
		}


		if (ypos < ycenter)
		{
			rotateAroundX -= 0.005f;
		}

		if (ypos > ycenter)
		{
			rotateAroundX += 0.005f;
		}
		 

		glfwSetCursorPos(win, 1024/2, 768/2);
	}
	*mvp = glm::rotate(*mvp, rotateAroundY, glm::vec3(0,1,0));
	*mvp = glm::rotate(*mvp, rotateAroundX, glm::vec3(1,0,0));
}

glm::vec3 getCamMove(GLFWwindow *win, glm::vec3 camDir, glm::vec3 camUp)
{
	static glm::vec3 res(0,0,0);
	
	glm::vec3 right = glm::cross(camUp, camDir);


	int stateA = glfwGetKey(win, GLFW_KEY_A);
	if (stateA == GLFW_PRESS || stateA == GLFW_REPEAT)
	{
		res -= 0.0001f * right;
		//std::cout << right.x << " " << right.y << " " << right.z << std::endl;
	}

	int stateD = glfwGetKey(win, GLFW_KEY_D);
	if (stateD == GLFW_PRESS || stateD == GLFW_REPEAT)
	{
		res += 0.0001f * right;
		//std::cout << right.x << " " << right.y << " " << right.z << std::endl;
	}

	int stateW = glfwGetKey(win, GLFW_KEY_W);
	if (stateW == GLFW_PRESS || stateW == GLFW_REPEAT)
	{
		res -= 0.0001f * camDir;
	}

	int stateS = glfwGetKey(win, GLFW_KEY_S);
	if (stateS == GLFW_PRESS || stateS == GLFW_REPEAT)
	{
		res += 0.0001f * camDir;
	}


	return res;
}

/**********************************************************************
 *                            MAIN PROGRAM                            *
 **********************************************************************/

int main(void)
{
	//call to helper functions
	initWindow();
	glfwSetKeyCallback(window, key_callback);
	glEnableCapabilities();
	initSpheres();
	initCube();

	glm::vec3 triangle[3]; 
	triangle[0] = glm::vec3(0,0,0);triangle[1] = glm::vec3(0,1,0);triangle[2] = glm::vec3(1,1,0);

	//opengl sphere buffers handling
	getNewVbo(GL_ARRAY_BUFFER, &vbo_spheres_pos, 8 * sizeof(glm::vec3), sphere_points, GL_STATIC_DRAW);
	getNewVbo(GL_ARRAY_BUFFER, &vbo_spheres_col, 8 * sizeof(glm::vec3), sphere_colors, GL_STATIC_DRAW);
	getNewVao(&vao_spheres, vbo_spheres_pos, vbo_spheres_col);

	//opengl cube buffers handling
	getNewVbo(GL_ARRAY_BUFFER, &vbo_cube_pos, 8 *sizeof(glm::vec3), cube_points, GL_STATIC_DRAW);
	getNewVbo(GL_ARRAY_BUFFER, &vbo_cube_color, 8 *sizeof(glm::vec3), cube_colors, GL_STATIC_DRAW);
	getNewVbo(GL_ELEMENT_ARRAY_BUFFER, &vbo_cube_indices, 36 * sizeof(unsigned int), cube_indices, GL_STATIC_DRAW);
	getNewVao(&vao_cube, vbo_cube_pos, vbo_cube_color, vbo_cube_indices);

	//shaders handling
	compileVertexAndFragmentShaders(&vs_sphere, &fs_sphere, &vertex_shader_spheres, &fragment_shader_spheres);
	compileShaderProgram(&shader_program_spheres, vs_sphere, fs_sphere);
	compileVertexAndFragmentShaders(&vs_basic, &fs_basic, &vertex_shader_basic, &fragment_shader_basic);
	compileShaderProgram(&shader_program_basic, vs_basic, fs_basic);


	glm::vec3 direction(0,0,1);
	while(!glfwWindowShouldClose(window))
	{
		//step 1 : clear screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		move_camera_direction(window, &direction);

		//step 2 : handle mvp matrix
		glm::mat4 m(1.f);
		glm::vec3 camMove = getCamMove(window, direction, glm::vec3(0,1,0));
		glm::vec3 camPos = glm::vec3(0,0,4) + camMove;

		glm::mat4 v = glm::lookAt(camPos, direction, glm::vec3(0,1,0));
		glm::mat4 p = glm::perspective(fov,(float)width/float(height), 0.1f, 100.f);
		mvp = p*v*m;
		move_camera_rotate(window,&mvp);

		glm::mat4 mv = v*m;

		//step 3 : display spheres in associated shader program
		displaySpheres(mvp, mv, shader_program_spheres, vao_spheres, vbo_spheres_pos);

		//step 4 : display cube in associated shader program
		displayCube(mvp, shader_program_basic, vao_cube, vbo_cube_pos, vbo_cube_color, vbo_cube_indices);

		//last step : read new events if some
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;

	exit(EXIT_SUCCESS);
}
