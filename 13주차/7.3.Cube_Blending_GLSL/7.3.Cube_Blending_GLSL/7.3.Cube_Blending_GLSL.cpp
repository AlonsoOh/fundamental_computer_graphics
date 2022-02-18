#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Shaders/LoadShaders.h"
#include "My_Shading.h"
GLuint h_ShaderProgram_PS; // handle to shader program

// for Phone Shading shader
#define NUMBER_OF_LIGHT_SUPPORTED 4 
GLint loc_global_ambient_color;
loc_light_Parameters loc_light[NUMBER_OF_LIGHT_SUPPORTED];
loc_Material_Parameters loc_material;
GLint loc_ModelViewProjectionMatrix_PS, loc_ModelViewMatrix_PS, loc_ModelViewMatrixInvTrans_PS;
GLint loc_u_flag_blending, loc_u_fragment_alpha;

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

#define LOC_VERTEX 0
#define LOC_NORMAL 1

// lights in scene
Light_Parameters light[NUMBER_OF_LIGHT_SUPPORTED];

// rectangle object
GLuint rectangle_VBO, rectangle_VAO;
GLfloat rectangle_vertices[12][3] = {  // vertices enumerated counterclockwise
	 { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f },
	 { 1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f },
	 { 1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }
};

#define NUMBER_OF_RECTANGLES 3
Material_Parameters material_rectangle[NUMBER_OF_RECTANGLES];
float rectangle_alpha[NUMBER_OF_RECTANGLES];

 void prepare_rectangle(void) { // Draw coordinate axes.
	 // Initialize vertex buffer object.
	 glGenBuffers(1, &rectangle_VBO);

	 glBindBuffer(GL_ARRAY_BUFFER, rectangle_VBO);
	 glBufferData(GL_ARRAY_BUFFER, sizeof(rectangle_vertices), &rectangle_vertices[0][0], GL_STATIC_DRAW);

	 // Initialize vertex array object.
	 glGenVertexArrays(1, &rectangle_VAO);
	 glBindVertexArray(rectangle_VAO);

	 glBindBuffer(GL_ARRAY_BUFFER, rectangle_VBO);
	 glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), BUFFER_OFFSET(0));
	 glEnableVertexAttribArray(0);
	 glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float)));
 	 glEnableVertexAttribArray(1);

	 glBindBuffer(GL_ARRAY_BUFFER, 0);
	 glBindVertexArray(0);

	 // rectangle 0: Yellow Rubber
	 material_rectangle[0].ambient_color[0] = 0.05f;
	 material_rectangle[0].ambient_color[1] = 0.05f;
	 material_rectangle[0].ambient_color[2] = 0.0f;
	 material_rectangle[0].ambient_color[3] = 1.0f;

	 material_rectangle[0].diffuse_color[0] = 0.5f;
	 material_rectangle[0].diffuse_color[1] = 0.5f;
	 material_rectangle[0].diffuse_color[2] = 0.4f;
	 material_rectangle[0].diffuse_color[3] = 1.0f;

	 material_rectangle[0].specular_color[0] = 0.7f;
	 material_rectangle[0].specular_color[1] = 0.7f;
	 material_rectangle[0].specular_color[2] = 0.04f;
	 material_rectangle[0].specular_color[3] = 1.0f;

	 material_rectangle[0].specular_exponent = 10.0f;

	 material_rectangle[0].emissive_color[0] = 0.0f;
	 material_rectangle[0].emissive_color[1] = 0.0f;
	 material_rectangle[0].emissive_color[2] = 0.0f;
	 material_rectangle[0].emissive_color[3] = 1.0f;

	 // rectangle 1: Emerald
	 material_rectangle[1].ambient_color[0] = 0.0215f;
	 material_rectangle[1].ambient_color[1] = 0.1745f;
	 material_rectangle[1].ambient_color[2] = 0.0215f;
	 material_rectangle[1].ambient_color[3] = 1.0f;

	 material_rectangle[1].diffuse_color[0] = 0.07568f;
	 material_rectangle[1].diffuse_color[1] = 0.61424f;
	 material_rectangle[1].diffuse_color[2] = 0.07568f;
	 material_rectangle[1].diffuse_color[3] = 1.0f;

	 material_rectangle[1].specular_color[0] = 0.633f;
	 material_rectangle[1].specular_color[1] = 0.727811f;
	 material_rectangle[1].specular_color[2] = 0.633f;
	 material_rectangle[1].specular_color[3] = 1.0f;

	 material_rectangle[1].specular_exponent = 76.8f;

	 material_rectangle[1].emissive_color[0] = 0.0f;
	 material_rectangle[1].emissive_color[1] = 0.0f;
	 material_rectangle[1].emissive_color[2] = 0.0f;
	 material_rectangle[1].emissive_color[3] = 1.0f;

	 // rectangle 2: Some background color
	 material_rectangle[2].ambient_color[0] = 0.10588f;
	 material_rectangle[2].ambient_color[1] = 0.058824f;
	 material_rectangle[2].ambient_color[2] = 0.113725f;
	 material_rectangle[2].ambient_color[3] = 1.0f;

	 material_rectangle[2].diffuse_color[0] = 0.18207451f;
	 material_rectangle[2].diffuse_color[1] = 0.21250588f;
	 material_rectangle[2].diffuse_color[2] = 0.27311176f;
	 material_rectangle[2].diffuse_color[3] = 1.0f;

	 material_rectangle[2].specular_color[0] = 0.23333f;
	 material_rectangle[2].specular_color[1] = 0.23333f;
	 material_rectangle[2].specular_color[2] = 0.4521569f;
	 material_rectangle[2].specular_color[3] = 1.0f;

	 material_rectangle[2].specular_exponent = 9.8f;

	 material_rectangle[2].emissive_color[0] = 0.0f;
	 material_rectangle[2].emissive_color[1] = 0.0f;
	 material_rectangle[2].emissive_color[2] = 0.0f;
	 material_rectangle[2].emissive_color[3] = 1.0f;

	 rectangle_alpha[0] = 0.25f;
	 rectangle_alpha[1] = 0.8f;
	 rectangle_alpha[2] = 1.0f;
 }

 void set_material_rectangle(int i) {
	 if ((i >= 0) && (i < NUMBER_OF_RECTANGLES)) {
		 glUniform4fv(loc_material.ambient_color, 1, material_rectangle[i].ambient_color);
	 	 glUniform4fv(loc_material.diffuse_color, 1, material_rectangle[i].diffuse_color);
		 glUniform4fv(loc_material.specular_color, 1, material_rectangle[i].specular_color);
		 glUniform1f(loc_material.specular_exponent, material_rectangle[i].specular_exponent);
		 glUniform4fv(loc_material.emissive_color, 1, material_rectangle[i].emissive_color);
		 return;
	 }
 }

 void draw_rectangle(void) {
	 glFrontFace(GL_CCW);

	 glBindVertexArray(rectangle_VAO);
	 glDrawArrays(GL_TRIANGLES, 0, 6);
	 glBindVertexArray(0);
 }

// cube object
GLuint cube_VBO, cube_VAO;
GLfloat cube_vertices[72][3] = { // vertices enumerated counterclockwise
	{ -1.0f, -1.0f, -1.0f }, { -1.0f, 0.0f, 0.0f }, { -1.0f, -1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f }, 
	{ -1.0f, 1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, 
	{ -1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f },
	{ 1.0f, -1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f }, { -1.0f, -1.0f, -1.0f }, { 0.0f, -1.0f, 0.0f }, 
	{ 1.0f, -1.0f, -1.0f }, { 0.0f, -1.0f, 0.0f },  
	{ 1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, 
	{ -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f },
	{ -1.0f, -1.0f, -1.0f }, { -1.0f, 0.0f, 0.0f }, { -1.0f, 1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f },
	{ -1.0f, 1.0f, -1.0f }, { -1.0f, 0.0f, 0.0f },
	{ 1.0f, -1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f }, { -1.0f, -1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f },
	{ -1.0f, -1.0f, -1.0f }, { 0.0f, -1.0f, 0.0f },
	{ -1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { -1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f },
	{ 1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f },
	{ 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, -1.0f, -1.0f }, { 1.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, -1.0f }, { 1.0f, 0.0f, 0.0f },
	{ 1.0f, -1.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f },
	{ 1.0f, -1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, -1.0f }, { 0.0f, 1.0f, 0.0f },
	{ -1.0f, 1.0f, -1.0f }, { 0.0f, 1.0f, 0.0f },
	{ 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { -1.0f, 1.0f, -1.0f }, { 0.0f, 1.0f, 0.0f },
	{ -1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f },
	{ 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { -1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f },
	{ 1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }
};

Material_Parameters material_cube;
float cube_alpha;

void prepare_cube(void) {  
	// Initialize vertex buffer object.
	glGenBuffers(1, &cube_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, cube_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), &cube_vertices[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &cube_VAO);
	glBindVertexArray(cube_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, cube_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_cube.ambient_color[0] = 0.1745f;
	material_cube.ambient_color[1] = 0.01175f;
	material_cube.ambient_color[2] = 0.01175f;
	material_cube.ambient_color[3] = 1.0f;

	material_cube.diffuse_color[0] = 0.61424f;
	material_cube.diffuse_color[1] = 0.04136f;
	material_cube.diffuse_color[2] = 0.04136f;
	material_cube.diffuse_color[3] = 1.0f;

	material_cube.specular_color[0] = 0.727811f;
	material_cube.specular_color[1] = 0.626959f;
	material_cube.specular_color[2] = 0.626959f;
	material_cube.specular_color[3] = 1.0f;

	material_cube.specular_exponent = 76.8f;

	material_cube.emissive_color[0] = 0.0f;
	material_cube.emissive_color[1] = 0.0f;
	material_cube.emissive_color[2] = 0.0f;
	material_cube.emissive_color[3] = 1.0f;

	cube_alpha = 0.5f;
}

void set_material_cube(void) {
	glUniform4fv(loc_material.ambient_color, 1, material_cube.ambient_color);
	glUniform4fv(loc_material.diffuse_color, 1, material_cube.diffuse_color);
	glUniform4fv(loc_material.specular_color, 1, material_cube.specular_color);
	glUniform1f(loc_material.specular_exponent, material_cube.specular_exponent);
	glUniform4fv(loc_material.emissive_color, 1, material_cube.emissive_color);
}

void draw_cube(void) {
	glFrontFace(GL_CCW);

	glBindVertexArray(cube_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6*6);
	glBindVertexArray(0);
}

int flag_blend_mode = 0; // for blending
int flag_back_to_front = 1; // for drawing order
float rotation_angle_cube = 0.0f;  // for cube rotation

void draw_objects_Back_to_Front(void) {
	glUseProgram(h_ShaderProgram_PS);
	if (flag_blend_mode) {
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glUniform1i(loc_u_flag_blending, 1); // Back_to_Front
	}
	else
		glUniform1i(loc_u_flag_blending, 0);

	// draw rectangle 2
	set_material_rectangle(2);
	glUniform1f(loc_u_fragment_alpha, rectangle_alpha[2]);
	ModelViewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-50.0f, -50.0f, -50.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(100.0f, 100.0f, 100.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_rectangle();

	// draw rectangle 1
	set_material_rectangle(1);
	glUniform1f(loc_u_fragment_alpha, rectangle_alpha[1]);
	ModelViewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-6.0f, -5.0f, -40.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(20.0f, 20.0f, 20.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_rectangle();  

	// draw cube
	glEnable(GL_CULL_FACE);

	set_material_cube();
	glUniform1f(loc_u_fragment_alpha, cube_alpha);

	ModelViewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 1.0f, -30.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(4.0f, 4.0f, 4.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, rotation_angle_cube, glm::vec3(1.0f, 1.0f, 1.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	
	glCullFace(GL_FRONT); 
	draw_cube();
	glCullFace(GL_BACK);
	draw_cube();

	glDisable(GL_CULL_FACE);

	// draw rectangle 0
	set_material_rectangle(0);
	glUniform1f(loc_u_fragment_alpha, rectangle_alpha[0]);
	ModelViewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-6.8f, -7.5f, -20.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(9.0f, 9.0f, 9.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_rectangle();  

	glUseProgram(0);
}

void draw_objects_Front_to_Back(void) {
	glUseProgram(h_ShaderProgram_PS);
	if (flag_blend_mode) {
		glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
		glUniform1i(loc_u_flag_blending, 2); // Front_to_Back
	}
	else  
		glUniform1i(loc_u_flag_blending, 0);

	// draw rectangle 0
	set_material_rectangle(0);
	glUniform1f(loc_u_fragment_alpha, rectangle_alpha[0]);
	ModelViewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-6.8f, -7.5f, -20.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(9.0f, 9.0f, 9.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_rectangle();  

	// draw cube
	glEnable(GL_CULL_FACE);

	set_material_cube();
	glUniform1f(loc_u_fragment_alpha, cube_alpha);

	ModelViewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 1.0f, -30.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(4.0f, 4.0f, 4.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, rotation_angle_cube, glm::vec3(1.0f, 1.0f, 1.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	glCullFace(GL_BACK);
 	draw_cube();

	glCullFace(GL_FRONT); 
 	draw_cube();

	glDisable(GL_CULL_FACE);

	// draw rectangle 1
	set_material_rectangle(1);
	glUniform1f(loc_u_fragment_alpha, rectangle_alpha[1]);
	ModelViewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-6.0f, -5.0f, -40.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(20.0f, 20.0f, 20.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
 	draw_rectangle();

	// draw rectangle 2
	set_material_rectangle(2);
	glUniform1f(loc_u_fragment_alpha, rectangle_alpha[2]);
	ModelViewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-50.0f, -50.0f, -50.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(100.0f, 100.0f, 100.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
 	draw_rectangle();

	glUseProgram(0);
}

// callbacks
void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (flag_blend_mode) {
		glEnable(GL_BLEND);
 		glDisable(GL_DEPTH_TEST);
	}

	if (flag_back_to_front)
		draw_objects_Back_to_Front();
	else 
		draw_objects_Front_to_Back();

 	glutSwapBuffers();

	if (flag_blend_mode) {
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups
		break;
	case 'b':
		flag_blend_mode = 1 - flag_blend_mode;
		if (flag_blend_mode) {
			fprintf(stdout, "* Blend mode is on...\n");
		}
		else {
			fprintf(stdout, "* Blend mode is off...\n");
		}
		glutPostRedisplay();
		break;
	case 'o':
		flag_back_to_front = 1 - flag_back_to_front;
		if (flag_back_to_front) {
			fprintf(stdout, "** Back-to-front mode is on...\n");
		}
		else {
			fprintf(stdout, "** Front-to-back mode is on...\n");
		}
		glutPostRedisplay();
		break;
	case 'r':
		if (flag_blend_mode) {
			cube_alpha += 0.05f;
			if (cube_alpha > 1.0f)
				cube_alpha = 1.0f;
			glutPostRedisplay();
		}
		break;
	case 'R':
		if (flag_blend_mode) {
			cube_alpha -= 0.05f;
			if (cube_alpha < 0.0f)
				cube_alpha = 0.0f;
			glutPostRedisplay();
		}
		break;
	case 'y':
		if (flag_blend_mode) {
			rectangle_alpha[0] += 0.05f;
			if (rectangle_alpha[0] > 1.0f)
				rectangle_alpha[0] = 1.0f;
			glutPostRedisplay();
		}
		break;
	case 'Y':
		if (flag_blend_mode) {
			rectangle_alpha[0] -= 0.05f;
			if (rectangle_alpha[0] < 0.0f)
				rectangle_alpha[0] = 0.0f;
			glutPostRedisplay();
		}
		break;
	case 'g':
		if (flag_blend_mode) {
			rectangle_alpha[1] += 0.05f;
			if (rectangle_alpha[1] > 1.0f)
				rectangle_alpha[1] = 1.0f;
			glutPostRedisplay();
		}
		break;
	case 'G':
		if (flag_blend_mode) {
			rectangle_alpha[1] -= 0.05f;
			if (rectangle_alpha[1] < 0.0f)
				rectangle_alpha[1] = 0.0f;
			glutPostRedisplay();
			break;
		}
	}
}

void reshape(int width, int height) {
	float aspect_ratio;
	glViewport(0, 0, width, height);
	
	aspect_ratio = (float) width / height;
	ProjectionMatrix = glm::perspective(45.0f*TO_RADIAN, aspect_ratio, 1.0f, 10000.0f);

	glutPostRedisplay();
}

void timer_scene(int timestamp_scene) {
	rotation_angle_cube = (timestamp_scene % 360)*TO_RADIAN;
	glutPostRedisplay();
	glutTimerFunc(100, timer_scene, (timestamp_scene + 1) % INT_MAX);
}

void cleanup(void) {
 	glDeleteVertexArrays(1, &rectangle_VAO);
 	glDeleteBuffers(1, &rectangle_VBO);

	glDeleteVertexArrays(1, &cube_VAO);
	glDeleteBuffers(1, &cube_VBO);
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutTimerFunc(100, timer_scene, 0);
	glutCloseFunc(cleanup);
}

void prepare_shader_program(void) {
	int i;
	char string[256];

	ShaderInfo shader_info_PS[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Phong_alpha_blending.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/Phong_alpha_blending.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram_PS = LoadShaders(shader_info_PS);
	loc_ModelViewProjectionMatrix_PS = glGetUniformLocation(h_ShaderProgram_PS, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_PS = glGetUniformLocation(h_ShaderProgram_PS, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_PS = glGetUniformLocation(h_ShaderProgram_PS, "u_ModelViewMatrixInvTrans");

	loc_global_ambient_color = glGetUniformLocation(h_ShaderProgram_PS, "u_global_ambient_color");
	for (i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].light_on", i);
		loc_light[i].light_on = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].position", i);
		loc_light[i].position = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].ambient_color", i);
		loc_light[i].ambient_color = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].diffuse_color", i);
		loc_light[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].specular_color", i);
		loc_light[i].specular_color = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].spot_direction", i);
		loc_light[i].spot_direction = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].spot_exponent", i);
		loc_light[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].spot_cutoff_angle", i);
		loc_light[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].light_attenuation_factors", i);
		loc_light[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_PS, string);
	}

	loc_material.ambient_color = glGetUniformLocation(h_ShaderProgram_PS, "u_material.ambient_color");
	loc_material.diffuse_color = glGetUniformLocation(h_ShaderProgram_PS, "u_material.diffuse_color");
	loc_material.specular_color = glGetUniformLocation(h_ShaderProgram_PS, "u_material.specular_color");
	loc_material.emissive_color = glGetUniformLocation(h_ShaderProgram_PS, "u_material.emissive_color");
	loc_material.specular_exponent = glGetUniformLocation(h_ShaderProgram_PS, "u_material.specular_exponent");

	loc_u_flag_blending = glGetUniformLocation(h_ShaderProgram_PS, "u_flag_blending");
	loc_u_fragment_alpha = glGetUniformLocation(h_ShaderProgram_PS, "u_fragment_alpha");
}

void initialize_lights_and_material(void) { // follow OpenGL conventions for initialization
	int i;

	glUseProgram(h_ShaderProgram_PS);

	glUniform4f(loc_global_ambient_color, 0.2f, 0.2f, 0.2f, 1.0f);
	for (i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		glUniform1i(loc_light[i].light_on, 0); // turn off all lights initially
		glUniform4f(loc_light[i].position, 0.0f, 0.0f, 1.0f, 0.0f);
		glUniform4f(loc_light[i].ambient_color, 0.0f, 0.0f, 0.0f, 1.0f);
		if (i == 0) {
			glUniform4f(loc_light[i].diffuse_color, 1.0f, 1.0f, 1.0f, 1.0f);
			glUniform4f(loc_light[i].specular_color, 1.0f, 1.0f, 1.0f, 1.0f);
		}
		else {
			glUniform4f(loc_light[i].diffuse_color, 0.0f, 0.0f, 0.0f, 1.0f);
			glUniform4f(loc_light[i].specular_color, 0.0f, 0.0f, 0.0f, 1.0f);
		}
		glUniform3f(loc_light[i].spot_direction, 0.0f, 0.0f, -1.0f);
		glUniform1f(loc_light[i].spot_exponent, 0.0f); // [0.0, 128.0]
		glUniform1f(loc_light[i].spot_cutoff_angle, 180.0f); // [0.0, 90.0] or 180.0 (180.0 for no spot light effect)
		glUniform4f(loc_light[i].light_attenuation_factors, 1.0f, 0.0f, 0.0f, 0.0f); // .w != 0.0f for no ligth attenuation
	}

	glUniform4f(loc_material.ambient_color, 0.2f, 0.2f, 0.2f, 1.0f);
	glUniform4f(loc_material.diffuse_color, 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform4f(loc_material.specular_color, 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform4f(loc_material.emissive_color, 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform1f(loc_material.specular_exponent, 0.0f); // [0.0, 128.0]

	glUseProgram(0);
}

void initialize_OpenGL(void) {
	glEnable(GL_DEPTH_TEST);
  	glEnable(GL_MULTISAMPLE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	ViewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), 
		glm::vec3(0.0f, 1.0f, 0.0f));

	initialize_lights_and_material();
}

void set_up_scene_lights(void) {
	// point_light_EC: use light 0
	light[0].light_on = 1;
	light[0].position[0] = 30.0f; light[0].position[1] = 100.0f; 	 
	light[0].position[2] = 100.0f; light[0].position[3] = 1.0f;

	light[0].ambient_color[0] = 0.3f; light[0].ambient_color[1] = 0.3f;
	light[0].ambient_color[2] = 0.3f; light[0].ambient_color[3] = 1.0f;

	light[0].diffuse_color[0] = 0.7f; light[0].diffuse_color[1] = 0.7f;
	light[0].diffuse_color[2] = 0.7f; light[0].diffuse_color[3] = 1.0f;

	light[0].specular_color[0] = 0.9f; light[0].specular_color[1] = 0.9f;
	light[0].specular_color[2] = 0.9f; light[0].specular_color[3] = 1.0f;

	// point_light_EC: use light 1
	light[1].light_on = 1;
	light[1].position[0] = -50.0f; light[1].position[1] = -100.0f;  
	light[1].position[2] = 100.0f; light[1].position[3] = 1.0f;

	light[1].ambient_color[0] = 0.3f; light[1].ambient_color[1] = 0.3f;
	light[1].ambient_color[2] = 0.3f; light[1].ambient_color[3] = 1.0f;

	light[1].diffuse_color[0] = 0.82f; light[1].diffuse_color[1] = 0.82f;
	light[1].diffuse_color[2] = 0.82f; light[1].diffuse_color[3] = 1.0f;

	light[1].specular_color[0] = 0.82f; light[1].specular_color[1] = 0.82f;
	light[1].specular_color[2] = 0.82f; light[1].specular_color[3] = 1.0f;

	glUseProgram(h_ShaderProgram_PS);
	glUniform4f(loc_global_ambient_color, 0.3f, 0.3f, 0.3f, 1.0f);

	glUniform1i(loc_light[0].light_on, light[0].light_on);
	glUniform4fv(loc_light[0].position, 1, light[0].position);
	glUniform4fv(loc_light[0].ambient_color, 1, light[0].ambient_color);
	glUniform4fv(loc_light[0].diffuse_color, 1, light[0].diffuse_color);
	glUniform4fv(loc_light[0].specular_color, 1, light[0].specular_color);

	glUniform1i(loc_light[1].light_on, light[1].light_on);
	glUniform4fv(loc_light[1].position, 1, light[1].position); 
	glUniform4fv(loc_light[1].ambient_color, 1, light[1].ambient_color);
	glUniform4fv(loc_light[1].diffuse_color, 1, light[1].diffuse_color);
	glUniform4fv(loc_light[1].specular_color, 1, light[1].specular_color);
	
	glUseProgram(0);
}

void prepare_scene(void) {
	prepare_rectangle();
	prepare_cube();
	set_up_scene_lights();
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
	GLint r, g, b, a; 
	
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	glutInitContextVersion(4, 0);

	glGetIntegerv(GL_RED_BITS, &r);
	glGetIntegerv(GL_GREEN_BITS, &g);
	glGetIntegerv(GL_BLUE_BITS, &b);
	glGetIntegerv(GL_ALPHA_BITS, &a);
	printf("*** Color Buffer Bit Planes: (R, G, B, A) = (%u, %u, %u, %u)\n\n", r, g, b, a);
}

#define N_MESSAGE_LINES 1
void main(int argc, char *argv[]) {
	// OpenGL blending example
	char program_name[64] = "Sogang CSE4170 7.3.Blending_Cube_PS_GLSL";
	char messages[N_MESSAGE_LINES][256] = { "    - Keys used: 'b', 'o', 'r', 'R', 'y', 'Y', 'g', 'G', 'ESC'" };

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE | GLUT_ALPHA);

	glutInitWindowSize(700, 700);

	glutInitContextProfile(GLUT_CORE_PROFILE); 
	glutCreateWindow(program_name);
	initialize_glew();
	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}