#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h> 
#include <GL/freeglut.h>

#include "Shaders/LoadShaders.h"
GLuint h_ShaderProgram; // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

// include glm/*.hpp only if necessary
//#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, ortho, perspective, etc.
glm::mat4 ModelViewProjectionMatrix;
glm::mat4 ModelViewMatrix, ViewMatrix, ProjectionMatrix;

#define TO_RADIAN 0.01745329252f 
#define TO_DEGREE 57.295779513f
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))

#define LOC_VERTEX 0
#define LOC_NORMAL 1
#define LOC_TEXCOORD 2

// codes for the axes object
GLuint axes_VBO, axes_VAO;
GLfloat axes[6][3] = {
	{ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }
};
GLfloat axes_color[3][3] = { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };

void prepare_axes(void) { // Draw coordinate axes.
	// Initialize vertex buffer object.
	glGenBuffers(1, &axes_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &axes_VAO);
	glBindVertexArray(axes_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_axes(void) {
	glBindVertexArray(axes_VAO);
	glUniform3fv(loc_primitive_color, 1, axes_color[0]);
	glDrawArrays(GL_LINES, 0, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[1]);
	glDrawArrays(GL_LINES, 2, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[2]);
	glDrawArrays(GL_LINES, 4, 2);
	glBindVertexArray(0);
}

// codes for the 'general' triangular-mesh object
typedef enum _GEOM_OBJ_TYPE { GEOM_OBJ_TYPE_V = 0, GEOM_OBJ_TYPE_VN, GEOM_OBJ_TYPE_VNT } GEOM_OBJ_TYPE;
// GEOM_OBJ_TYPE_V: (x, y, z), GEOM_OBJ_TYPE_VN: (x, y, z, nx, ny, nz), GEOM_OBJ_TYPE_VNT: (x, y, z, nx, ny, nz, s, t)
int GEOM_OBJ_ELEMENTS_PER_VERTEX[3] = { 3, 6, 8 };

int read_geometry_file(GLfloat **object, char *filename, GEOM_OBJ_TYPE geom_obj_type) {
	int i, n_triangles;
	float *flt_ptr;
	FILE *fp;

	fprintf(stdout, "Reading geometry from the geometry file %s...\n", filename);
	fp = fopen(filename, "r");
	if (fp == NULL){
		fprintf(stderr, "Cannot open the geometry file %s ...", filename);
		return -1;
	}
	fscanf(fp, "%d", &n_triangles);
	*object = (float *)malloc(3*n_triangles*GEOM_OBJ_ELEMENTS_PER_VERTEX[geom_obj_type]*sizeof(float));
	if (*object == NULL){
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...", filename);
		return -1;
	}

	flt_ptr = *object;
	for (i = 0; i < 3 * n_triangles * GEOM_OBJ_ELEMENTS_PER_VERTEX[geom_obj_type]; i++)  
		fscanf(fp, "%f", flt_ptr++);
	fclose(fp);

	fprintf(stdout, "Read %d primitives successfully.\n\n", n_triangles);

	return n_triangles;
}

#define N_GEOMETRY_OBJECTS 2
#define GEOM_OBJ_ID_TEAPOT 0
#define GEOM_OBJ_ID_DONUT 1

GLuint geom_obj_VBO[N_GEOMETRY_OBJECTS];
GLuint geom_obj_VAO[N_GEOMETRY_OBJECTS];

int geom_obj_n_triangles[N_GEOMETRY_OBJECTS];
GLfloat *geom_obj_vertices[N_GEOMETRY_OBJECTS];

void prepare_geom_obj(int geom_obj_ID, char *filename, GEOM_OBJ_TYPE geom_obj_type) {
	int n_bytes_per_vertex;

	n_bytes_per_vertex = GEOM_OBJ_ELEMENTS_PER_VERTEX[geom_obj_type] * sizeof(float);

	geom_obj_n_triangles[geom_obj_ID] = read_geometry_file(&geom_obj_vertices[geom_obj_ID], filename, geom_obj_type);
	// Assume all geometry files are effective.

	glGenBuffers(1, &geom_obj_VBO[geom_obj_ID]);

	glBindBuffer(GL_ARRAY_BUFFER, geom_obj_VBO[geom_obj_ID]);
	glBufferData(GL_ARRAY_BUFFER, 3 * geom_obj_n_triangles[geom_obj_ID] * n_bytes_per_vertex,
		geom_obj_vertices[geom_obj_ID], GL_STATIC_DRAW);

	free(geom_obj_vertices[geom_obj_ID]);

	// Initialize vertex array object.
	glGenVertexArrays(1, &geom_obj_VAO[geom_obj_ID]);

	glBindVertexArray(geom_obj_VAO[geom_obj_ID]);
	glBindBuffer(GL_ARRAY_BUFFER, geom_obj_VBO[geom_obj_ID]);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	if (geom_obj_type >= GEOM_OBJ_TYPE_VN) {
		glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}
	if (geom_obj_type >= GEOM_OBJ_TYPE_VNT) {
		glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_geom_obj(int geom_obj_ID) {
	glBindVertexArray(geom_obj_VAO[geom_obj_ID]);
	glDrawArrays(GL_TRIANGLES, 0, 3 * geom_obj_n_triangles[geom_obj_ID]);
	glBindVertexArray(0);
}

// callbacks
float window_aspect_ratio = 1.0f; // set when the window opens for the first time or its size changes
int cull_face_mode = 1; // remove back-faced triangles
int fill_face_mode = 0; // fill inside of triangles

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT);
	 
	ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(50.0f, 50.0f, 50.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(2.0f);
	draw_axes();
	glLineWidth(1.0f);

 	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, 0.0f, 1.5f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniform3f(loc_primitive_color, 1.0f, 1.0f, 0.0f);  
	draw_geom_obj(GEOM_OBJ_ID_TEAPOT);

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-6.0f, -5.0f, 1.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(2.0f, 2.0f, 2.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniform3f(loc_primitive_color, 1.0f, 0.0f, 1.0f);
	draw_geom_obj(GEOM_OBJ_ID_DONUT);

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, -6.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -45.0f*TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(1.0f, 1.0f, -1.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 45.0f*TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniform3f(loc_primitive_color, 0.0f, 1.0f, 1.0f);
	draw_geom_obj(GEOM_OBJ_ID_DONUT);
 
	glFlush();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'q':  
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	case 'o':
		// orthographic projection
		ProjectionMatrix = glm::ortho(-2.7f*window_aspect_ratio, 2.7f*window_aspect_ratio, -2.7f, 2.7f, 5.0f, 19.0f);
		glutPostRedisplay();
		break;
	case 'p':
		// perspective projection
		ProjectionMatrix = glm::perspective(28.0f*TO_RADIAN, window_aspect_ratio, 5.0f, 19.0f);
		glutPostRedisplay();
		break;
	case 'c' :
		cull_face_mode = 1 - cull_face_mode;
		if (cull_face_mode)
			glEnable(GL_CULL_FACE); // turn the face-culling feature on
		else
			glDisable(GL_CULL_FACE); // turn the face-culling feature off
		glutPostRedisplay();
		break;
	case 'f':
		fill_face_mode = 1 - fill_face_mode;
		if (fill_face_mode)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  
		glutPostRedisplay();
		break;
	}
}

void reshape(int width, int height) {
	// this callback function is called 
	//  1. when the window pops up on the screen for the first time, or
	//  2. when the window sizes chages
	// tell the user know the right time when various initializations (what kinds?) are performed

	// set up the initial viewport transformation 
	glViewport(0, 0, width, height);
	
	window_aspect_ratio = (float)width / height;
	// set up the initial projection transformation 
	ProjectionMatrix = glm::perspective(28.0f*TO_RADIAN, window_aspect_ratio, 5.0f, 19.0f);

	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &axes_VAO);
	glDeleteBuffers(1, &axes_VBO);

	glDeleteVertexArrays(N_GEOMETRY_OBJECTS, geom_obj_VAO);
	glDeleteBuffers(N_GEOMETRY_OBJECTS, geom_obj_VBO);
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutCloseFunc(cleanup);
}

void prepare_shader_program(void) {
	ShaderInfo shader_info[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram, "u_primitive_color");
}

void initialize_OpenGL(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // draw edges of triangles

	glFrontFace(GL_CCW); // vertices are enumerated counterclock-wise seen from outside
	glCullFace(GL_BACK); // remove back-faces when the face-culling feature in on
	glEnable(GL_CULL_FACE); // turn the face-culling feature on

	glClearColor(14 / 255.0f, 98 / 255.0f, 81 / 255.0f, 1.0f); // background color of the window

	// set up the initial viewing transformation 
	ViewMatrix = glm::lookAt(glm::vec3(2.0f, 8.0f, 8.0f), glm::vec3(0.0f, -4.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)); 
}

void prepare_scene(void) {
	prepare_axes();
	prepare_geom_obj(GEOM_OBJ_ID_TEAPOT, "Data/teapot_triangles_v.txt", GEOM_OBJ_TYPE_V); // vertex coordinates only
	prepare_geom_obj(GEOM_OBJ_ID_DONUT, "Data/donut_triangles_v.txt", GEOM_OBJ_TYPE_V);
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "*********************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "*********************************************************\n\n");
}

void greetings(char *program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 1
void main(int argc, char *argv[]) {
	char program_name[64] = "Sogang CSE4170 CHAP2SEC3_GLSL";
	char messages[N_MESSAGE_LINES][256] = { "    - Keys used: 'p', 'o', 'c', 'f', 'q'" };

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowSize(800, 600);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}
