#pragma warning(disable : 4996)

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Shaders/LoadShaders.h"
 
GLuint h_ShaderProgram_simple, h_ShaderProgram_PS; // handles to shader programs

// for simple shaders
GLint loc_ModelViewProjectionMatrix_simple, loc_primitive_color;

// for Simple Phone Shading shaders
GLint loc_ModelViewProjectionMatrix_PS, loc_ModelViewMatrix_PS, loc_ModelViewMatrixInvTrans_PS;
GLint loc_Light_Position, loc_Light_La, loc_Light_L;
GLint loc_Material_Ka, loc_Material_Kd, loc_Material_Ks, loc_Material_Shininess;
GLint loc_use_halfway_vector;

// include glm/*.hpp only if necessary
//#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.
#include <glm/gtc/matrix_inverse.hpp> // inverseTranspose, etc.
glm::mat4 ModelViewProjectionMatrix, ModelViewMatrix;
glm::mat3 ModelViewMatrixInvTrans;
glm::mat4 ViewMatrix, ProjectionMatrix;

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))

#define LOC_POSITION 0
#define LOC_NORMAL 1

// for tiger animation
int cur_frame_tiger = 0;
int rotation_angle_tiger = 0;

// axes object
GLuint axes_VBO, axes_VAO;
GLfloat axes_vertices[6][3] = {
	{ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }
};
GLfloat axes_color[3][3] = { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };

void prepare_axes(void) { // draw coordinate axes
	// initialize vertex buffer object
	glGenBuffers(1, &axes_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes_vertices), &axes_vertices[0][0], GL_STATIC_DRAW);

	// initialize vertex array object
	glGenVertexArrays(1, &axes_VAO);
	glBindVertexArray(axes_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glVertexAttribPointer(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

 void draw_axes(void) {
	 // assume ShaderProgram_simple is used
	 glBindVertexArray(axes_VAO);
	 glUniform3fv(loc_primitive_color, 1, axes_color[0]);
	 glDrawArrays(GL_LINES, 0, 2);
	 glUniform3fv(loc_primitive_color, 1, axes_color[1]);
	 glDrawArrays(GL_LINES, 2, 2);
	 glUniform3fv(loc_primitive_color, 1, axes_color[2]);
	 glDrawArrays(GL_LINES, 4, 2);
	 glBindVertexArray(0);
 }

 // floor object
 GLuint rectangle_VBO, rectangle_VAO;
 GLfloat rectangle_vertices[12][3] = {  // vertices enumerated counterclockwise
	 { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f },
	 { 1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f },
	 { 1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }
 };

  void prepare_floor(void) { // Draw coordinate axes.
	 // Initialize vertex buffer object.
	 glGenBuffers(1, &rectangle_VBO);

	 glBindBuffer(GL_ARRAY_BUFFER, rectangle_VBO);
	 glBufferData(GL_ARRAY_BUFFER, sizeof(rectangle_vertices), &rectangle_vertices[0][0], GL_STATIC_DRAW);

	 // Initialize vertex array object.
	 glGenVertexArrays(1, &rectangle_VAO);
	 glBindVertexArray(rectangle_VAO);

	 glBindBuffer(GL_ARRAY_BUFFER, rectangle_VBO);
	 glVertexAttribPointer(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), BUFFER_OFFSET(0));
	 glEnableVertexAttribArray(0);
	 glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float)));
	 glEnableVertexAttribArray(1);

	 glBindBuffer(GL_ARRAY_BUFFER, 0);
	 glBindVertexArray(0);

 }

 void set_material_floor(void) {
	 glUniform3f(loc_Material_Ka, 0.0f, 0.05f, 0.0f);
	 glUniform3f(loc_Material_Kd, 0.4f, 0.5f, 0.4f);
	 glUniform3f(loc_Material_Ks, 0.04f, 0.7f, 0.04f);
	 glUniform1f(loc_Material_Shininess, 2.5f); // [0.0, 128.0]
 }

 void draw_floor(void) {
	 glFrontFace(GL_CCW);

	 glBindVertexArray(rectangle_VAO);
	 glDrawArrays(GL_TRIANGLES, 0, 6);
	 glBindVertexArray(0);
 }

 // tiger object
#define N_TIGER_FRAMES 12
GLuint tiger_VBO, tiger_VAO;
int tiger_n_triangles[N_TIGER_FRAMES];
int tiger_vertex_offset[N_TIGER_FRAMES];
GLfloat *tiger_vertices[N_TIGER_FRAMES];

int read_geometry(GLfloat **object, int bytes_per_primitive, char *filename) {
	int n_triangles;
	FILE *fp;

	// fprintf(stdout, "Reading geometry from the geometry file %s...\n", filename);
	fp = fopen(filename, "rb");
	if (fp == NULL){
		fprintf(stderr, "Cannot open the object file %s ...", filename);
		return -1;
	}
	fread(&n_triangles, sizeof(int), 1, fp);
	*object = (float *)malloc(n_triangles*bytes_per_primitive);
	if (*object == NULL){
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...", filename);
		return -1;
	}

	fread(*object, bytes_per_primitive, n_triangles, fp);
	// fprintf(stdout, "Read %d primitives successfully.\n\n", n_triangles);
	fclose(fp);

	return n_triangles;
}

void set_material_tiger(int ambient, int diffuse, int specular) {
 
	if (ambient) 
		glUniform3f(loc_Material_Ka, 0.4f, 0.3f, 0.1f);
	else 
		glUniform3f(loc_Material_Ka, 0.0f, 0.0f, 0.0f);
	if (diffuse)
		glUniform3f(loc_Material_Kd, 0.95164f, 0.60648f, 0.22648f);
	else 
		glUniform3f(loc_Material_Kd, 0.0f, 0.0f, 0.0f);
	if (specular) {
		glUniform3f(loc_Material_Ks, 0.628281f, 0.555802f, 0.555802f);
		glUniform1f(loc_Material_Shininess, 21.2f); // [0.0, 128.0]
	}
	else {
		glUniform3f(loc_Material_Ks, 0.0f, 0.0f, 0.0f);
		glUniform1f(loc_Material_Shininess, 1.0f); // [0.0, 128.0]
	}
}

void prepare_tiger(void) { // vertices enumerated clockwise
	int i, n_bytes_per_vertex, n_bytes_per_triangle, tiger_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_TIGER_FRAMES; i++) {
		sprintf(filename, "Data/Tiger_%d%d_triangles_vnt.geom", i / 10, i % 10);
		tiger_n_triangles[i] = read_geometry(&tiger_vertices[i], n_bytes_per_triangle, filename);
		// assume all geometry files are effective
		tiger_n_total_triangles += tiger_n_triangles[i];

		if (i == 0)
			tiger_vertex_offset[i] = 0;
		else
			tiger_vertex_offset[i] = tiger_vertex_offset[i - 1] + 3 * tiger_n_triangles[i - 1];
	}

	// initialize vertex buffer object
	glGenBuffers(1, &tiger_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, tiger_VBO);
	glBufferData(GL_ARRAY_BUFFER, tiger_n_total_triangles*n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_TIGER_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, tiger_vertex_offset[i] * n_bytes_per_vertex,
		tiger_n_triangles[i] * n_bytes_per_triangle, tiger_vertices[i]);

	// as the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_TIGER_FRAMES; i++)
		free(tiger_vertices[i]);

	// initialize vertex array object
	glGenVertexArrays(1, &tiger_VAO);
	glBindVertexArray(tiger_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, tiger_VBO);
	glVertexAttribPointer(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_tiger(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(tiger_VAO);
	glDrawArrays(GL_TRIANGLES, tiger_vertex_offset[cur_frame_tiger], 3 * tiger_n_triangles[cur_frame_tiger]);
	glBindVertexArray(0);
}

// callbacks
void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(h_ShaderProgram_simple);
	ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(50.0f, 50.0f, 50.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_simple, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(2.0f);
	draw_axes();
	glLineWidth(1.0f);

	glUseProgram(h_ShaderProgram_PS);
  	set_material_floor();
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-250.0f, 0.0f, 250.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(500.0f, 500.0f, 500.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_floor();

    // Draw four tigers.
 	set_material_tiger(1, 0, 0);
 	ModelViewMatrix = glm::rotate(ViewMatrix, -rotation_angle_tiger*TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(200.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_tiger(); // ambient tiger

	int next_rotation_angle_tiger = (rotation_angle_tiger + 90) % 360;
	set_material_tiger(0, 1, 0);
	ModelViewMatrix = glm::rotate(ViewMatrix, -next_rotation_angle_tiger*TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(200.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_tiger(); // diffuse tiger

	next_rotation_angle_tiger = (rotation_angle_tiger + 180) % 360;
	set_material_tiger(0, 0, 1);
	ModelViewMatrix = glm::rotate(ViewMatrix, -next_rotation_angle_tiger * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(200.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_tiger(); // specular tiger

	next_rotation_angle_tiger = (rotation_angle_tiger + 270) % 360;
	set_material_tiger(1, 1, 1);
	ModelViewMatrix = glm::rotate(ViewMatrix, -next_rotation_angle_tiger * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(200.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_tiger(); //  tiger with full shading

	glUseProgram(0);

	glutSwapBuffers();
}

unsigned int _timestamp_scene = 0;
int animation_mode = 1;
void timer_scene(int value) {
	cur_frame_tiger = _timestamp_scene % N_TIGER_FRAMES;
	rotation_angle_tiger = _timestamp_scene % 360;
	glutPostRedisplay();
	if (animation_mode) {
		_timestamp_scene++;
		glutTimerFunc(100, timer_scene, 0);
	}
}

int use_halfway_vector = 0;
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups
		break;
	case 'a':
		animation_mode = 1 - animation_mode;
		if (animation_mode) {
			_timestamp_scene++;
			glutTimerFunc(100, timer_scene, 0);
		}
		break;
	case 'h':
		use_halfway_vector = 1 - use_halfway_vector;
		if (use_halfway_vector) {
			fprintf(stdout, "^^^ Using the halfway vector for specular reflection...\n");
			glUseProgram(h_ShaderProgram_PS);
			glUniform1i(loc_use_halfway_vector, 1);
			glUseProgram(0);
			glutPostRedisplay();
		}
		else {
			fprintf(stdout, "^^^ Using the reflection vector for specular reflection...\n");
			glUseProgram(h_ShaderProgram_PS);
			glUniform1i(loc_use_halfway_vector, 0);
			glUseProgram(0);
			glutPostRedisplay();
		}
		break;
	}
}

void reshape(int width, int height) {
	float aspect_ratio;
	glViewport(0, 0, width, height);
	
	aspect_ratio = (float) width / height;
	ProjectionMatrix = glm::perspective(45.0f*TO_RADIAN, aspect_ratio, 200.0f, 1000.0f);

	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &axes_VAO);
	glDeleteBuffers(1, &axes_VBO);

	glDeleteVertexArrays(1, &rectangle_VAO);
	glDeleteBuffers(1, &rectangle_VBO);

	glDeleteVertexArrays(1, &tiger_VAO);
	glDeleteBuffers(1, &tiger_VBO);
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutTimerFunc(100, timer_scene, 0);
	glutCloseFunc(cleanup);
}

void prepare_shader_program(void) {
	ShaderInfo shader_info_simple[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};
	ShaderInfo shader_info_PS[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Simple_Phong.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/Simple_Phong.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram_simple = LoadShaders(shader_info_simple);
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram_simple, "u_primitive_color");
	loc_ModelViewProjectionMatrix_simple = glGetUniformLocation(h_ShaderProgram_simple, 
		"u_ModelViewProjectionMatrix");

	h_ShaderProgram_PS = LoadShaders(shader_info_PS);
	loc_ModelViewProjectionMatrix_PS = glGetUniformLocation(h_ShaderProgram_PS, "MVP");
	loc_ModelViewMatrix_PS = glGetUniformLocation(h_ShaderProgram_PS, "ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_PS = glGetUniformLocation(h_ShaderProgram_PS, "NormalMatrix");

	loc_Light_Position = glGetUniformLocation(h_ShaderProgram_PS, "Light.Position");
	loc_Light_La = glGetUniformLocation(h_ShaderProgram_PS, "Light.La");
	loc_Light_L = glGetUniformLocation(h_ShaderProgram_PS, "Light.L");

	loc_Material_Ka = glGetUniformLocation(h_ShaderProgram_PS, "Material.Ka");
	loc_Material_Kd = glGetUniformLocation(h_ShaderProgram_PS, "Material.Kd");
	loc_Material_Ks = glGetUniformLocation(h_ShaderProgram_PS, "Material.Ks");
	loc_Material_Shininess = glGetUniformLocation(h_ShaderProgram_PS, "Material.Shininess");

	loc_use_halfway_vector = glGetUniformLocation(h_ShaderProgram_PS, "use_halfway_vector");
}

void initialize_OpenGL(void) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glm::vec4 Light_Position = { 0.0f, 500.0f, 0.0f, 1.0f };
	ViewMatrix = glm::lookAt(glm::vec3(350.0f, 350.0f, 350.0f), glm::vec3(0.0f, 0.0f, 0.0f), 
		glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec4 Light_Position_EC = ViewMatrix * Light_Position; // What is this line for?
	 
	glUseProgram(h_ShaderProgram_PS);
	glUniform4fv(loc_Light_Position, 1, &Light_Position_EC[0]);
	glUniform3f(loc_Light_La, 0.5f, 0.5f, 0.5f);
	glUniform3f(loc_Light_L, 1.0f, 1.0f, 1.0f);

	fprintf(stdout, "^^^ Using the reflection vector for specular reflection...\n");
	use_halfway_vector = 0;
	glUniform1i(loc_use_halfway_vector, 0);
	glUseProgram(0);
}

void prepare_scene(void) {
	prepare_axes();
	prepare_floor();
	prepare_tiger();
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
int main(int argc, char *argv[]) {
	// Phong Shading
	char program_name[64] = "Sogang CSE4170 5.3.1.Tiger_Simple_Shading_PS_GLSL";
	char messages[N_MESSAGE_LINES][256] = { "    - Keys used: 'a', 'h', 'ESC'"  };

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(800, 800);
	glutInitContextVersion(3, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
	return 1;
}