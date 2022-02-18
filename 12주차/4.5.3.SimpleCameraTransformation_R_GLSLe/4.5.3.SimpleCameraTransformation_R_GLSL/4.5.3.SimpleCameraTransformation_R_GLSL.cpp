#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

// Begin of shader setup
#include "Shaders/LoadShaders.h"
GLuint h_ShaderProgram; // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

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
// End of shader setup

// Begin of geometry setup
#include "4.5.3.GeometrySetup.h"
// End of geometry setup

// Begin of Callback function definitions

// include glm/*.hpp only if necessary
// #include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f

typedef struct {
	glm::vec3 prp, vrp, vup; // in this example code, make vup always equal to the v direction.
	float fov_y, aspect_ratio, near_clip, far_clip, zoom_factor;
} CAMERA;

typedef struct {
	int x, y, w, h;
} VIEWPORT;

typedef enum {
	VIEW_WORLD, VIEW_SQUARE, VIEW_TIGER, VIEW_COW
} VIEW_MODE;

#define NUMBER_OF_CAMERAS 2

CAMERA camera[NUMBER_OF_CAMERAS];
VIEWPORT viewport[NUMBER_OF_CAMERAS];

// ViewProjectionMatrix = ProjectionMatrix * ViewMatrix
glm::mat4 ViewProjectionMatrix[NUMBER_OF_CAMERAS], ViewMatrix[NUMBER_OF_CAMERAS], ProjectionMatrix[NUMBER_OF_CAMERAS];
// ModelViewProjectionMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix
glm::mat4 ModelViewProjectionMatrix; // This one is sent to vertex shader when it is ready.

void print_mat4(const char *string, glm::mat4 M) {
	fprintf(stdout, "\n***** %s ******\n", string);
	for (int i = 0; i < 4; i++)
		fprintf(stdout, "*** COL[%d] (%f, %f, %f, %f)\n", i, M[i].x, M[i].y, M[i].z, M[i].w);
	fprintf(stdout, "**************\n\n");
}

float rotation_angle_cow;
VIEW_MODE view_mode;

void display_camera(int camera_index) {
	// should optimize this dispaly function further to reduce the amount of floating-point operations.
	glm::mat4 Matrix_TIGER_tmp;

	glViewport(viewport[camera_index].x, viewport[camera_index].y, viewport[camera_index].w, viewport[camera_index].h);
	
	// At this point, the matrix ViewProjectionMatrix has been properly set up.
	switch (view_mode) {
	case VIEW_WORLD:
		ModelViewProjectionMatrix = glm::scale(ViewProjectionMatrix[camera_index], glm::vec3(5.0f, 5.0f, 5.0f));
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glLineWidth(5.0f);
		draw_axes(); // draw the WC axes.
		glLineWidth(1.0f);

		if (camera_index == 1) { // REMOVE THIS PART LATER...
			// THIS IS NOT THE RIGHT WAY OF CLEARING THE BACKGROUND OF THE SECOND VIEWPORT.
			// DEFINE A SUBWINDOW FOR THE SECOND VIEWPORT AND CLEAR IT USING the glClear(*) FUNCTION!!!
			ModelViewProjectionMatrix = glm::translate(ViewProjectionMatrix[camera_index], glm::vec3(-5000.0f, -10.0f, -5000.0f));
			ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(10000.0f, 10000.0f, 10000.0f));
			ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix,
				90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
			glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			draw_object(OBJECT_SQUARE16, 50 / 255.0f, 50 / 255.0f, 50 / 255.0f); //  draw the floor.
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		ModelViewProjectionMatrix = glm::translate(ViewProjectionMatrix[camera_index], glm::vec3(-6.0f, -0.01f, -6.0f));
		ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(12.0f, 12.0f, 12.0f));
		ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix,
			90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		draw_object(OBJECT_SQUARE16, 20 / 255.0f, 90 / 255.0f, 50 / 255.0f); //  draw the floor.
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		// set up this matrix only once outside this callback function if it is static.
		Matrix_TIGER_tmp = glm::rotate(glm::mat4(1.0f), -90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		Matrix_TIGER_tmp = glm::scale(Matrix_TIGER_tmp, glm::vec3(0.01f, 0.01f, 0.01f));

		ModelViewProjectionMatrix = glm::translate(ViewProjectionMatrix[camera_index], glm::vec3(-2.0f, 0.0f, -3.5f));
		ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix, 30.0f*TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
		ModelViewProjectionMatrix *= Matrix_TIGER_tmp;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_object(OBJECT_TIGER, 255 / 255.0f, 255 / 255.0f, 255 / 255.0f); // White

		ModelViewProjectionMatrix = glm::translate(ViewProjectionMatrix[camera_index], glm::vec3(0.0f, 0.0f, -3.5f));
		ModelViewProjectionMatrix *= Matrix_TIGER_tmp;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_object(OBJECT_TIGER, 0 / 255.0f, 0 / 255.0f, 255 / 255.0f); // Blue
 
		{
			float cyan_tiger_offset;
			cyan_tiger_offset = (float)((16 * (int)rotation_angle_cow) % 360);
			if (cyan_tiger_offset <= 180.0f)
				cyan_tiger_offset = (cyan_tiger_offset - 180.0f) / 180.0f;
			else
				cyan_tiger_offset = -(cyan_tiger_offset - 180.0f) / 180.0f;

			ModelViewProjectionMatrix = glm::translate(ViewProjectionMatrix[camera_index],
				glm::vec3(0.0f, 0.0f, 4.25f + cyan_tiger_offset));
			ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix, 180.0f*TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
			draw_axes();
			ModelViewProjectionMatrix *= Matrix_TIGER_tmp;
			glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
			draw_object(OBJECT_TIGER, 0 / 255.0f, 255 / 255.0f, 255 / 255.0f); // Cyan
		}

		ModelViewProjectionMatrix = glm::translate(ViewProjectionMatrix[camera_index], glm::vec3(-3.0f, 0.0f, 2.0f));
		ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix, 90.0f*TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
		ModelViewProjectionMatrix *= Matrix_TIGER_tmp;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_object(OBJECT_TIGER, 255 / 255.0f, 0 / 255.0f, 255 / 255.0f); // Magenta

		ModelViewProjectionMatrix = glm::translate(ViewProjectionMatrix[camera_index], glm::vec3(2.0f, 0.0f, -3.0f));
		ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix, -45.0f*TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
		ModelViewProjectionMatrix *= Matrix_TIGER_tmp;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_object(OBJECT_TIGER, 0 / 255.0f, 255 / 255.0f, 0 / 255.0f); // Green

		ModelViewProjectionMatrix = glm::translate(ViewProjectionMatrix[camera_index], glm::vec3(3.0f, 0.0f, 2.0f));
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glPointSize(5.0f);
		draw_points(255 / 255.0f, 0 / 255.0f, 0 / 255.0f);
		glPointSize(1.0f);

		ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix, -135.0f*TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
		ModelViewProjectionMatrix *= Matrix_TIGER_tmp;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_object(OBJECT_TIGER, 255 / 255.0f, 0 / 255.0f, 0 / 255.0f); // Red

		ModelViewProjectionMatrix = glm::translate(ViewProjectionMatrix[camera_index], glm::vec3(0.0f, 0.645f, 0.0f));
		ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix,
			rotation_angle_cow*TO_RADIAN, glm::vec3(0.0, 1.0, 0.0));
		ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(2.0f, 2.0f, 2.0f));
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_object(OBJECT_COW, 255 / 255.0f, 255 / 255.0f, 0 / 255.0f); //  Yellow
		break;
	case VIEW_SQUARE:
		ModelViewProjectionMatrix = ViewProjectionMatrix[camera_index]; // ModelMatrix = I
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_object(OBJECT_SQUARE16, 255 / 255.0f, 255 / 255.0f, 255 / 255.0f);
		glLineWidth(3.0f);
		draw_axes();
		glLineWidth(1.0f);
		break;
	case VIEW_TIGER:
		ModelViewProjectionMatrix = ViewProjectionMatrix[camera_index]; // ModelMatrix = I
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glLineWidth(3.0f);
		draw_axes();
		glLineWidth(1.0f);
		// the TIGER model is too big ...
		ModelViewProjectionMatrix = glm::scale(ViewProjectionMatrix[camera_index], glm::vec3(0.01f, 0.01f, 0.01f));
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_object(OBJECT_TIGER, 255 / 255.0f, 255 / 255.0f, 255 / 255.0f);
		break;
	case VIEW_COW:
		ModelViewProjectionMatrix = ViewProjectionMatrix[camera_index]; // ModelMatrix = I
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_object(OBJECT_COW, 255 / 255.0f, 255 / 255.0f, 255 / 255.0f);
		glLineWidth(3.0f);
		draw_axes();
		glLineWidth(1.0f);
		break;
	}
}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	display_camera(0);
 	display_camera(1);
	glutSwapBuffers();
}

unsigned int leftbutton_pressed = 0, rotation_mode_cow = 1, timestamp_cow = 0;
int prevx, prevy;

void timer_scene(int value) {
	rotation_angle_cow = (float)(timestamp_cow);
	glutPostRedisplay();
	timestamp_cow = (timestamp_cow + 1) % 360;
	if (rotation_mode_cow)
		glutTimerFunc(100, timer_scene, 0);
}

void mousepress(int button, int state, int x, int y)  {
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {
		prevx = x, prevy = y;
		leftbutton_pressed = 1;
		glutPostRedisplay();
	}
	else if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP)) {
		leftbutton_pressed = 0;
		glutPostRedisplay();
	}
}

#define CAM_ROT_SENSITIVITY 0.15f
void motion_1(int x, int y) {
	glm::mat4 mat4_tmp;
	glm::vec3 vec3_tmp;
	float delx, dely;

	if (leftbutton_pressed) {
		delx = (float)(x - prevx), dely = -(float)(y - prevy);
		prevx = x, prevy = y;

		mat4_tmp = glm::translate(glm::mat4(1.0f), camera[0].vrp);
		mat4_tmp = glm::rotate(mat4_tmp, CAM_ROT_SENSITIVITY*delx*TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
		mat4_tmp = glm::translate(mat4_tmp, -camera[0].vrp);

 		camera[0].prp = glm::vec3(mat4_tmp*glm::vec4(camera[0].prp, 1.0f));
 		camera[0].vup = glm::vec3(mat4_tmp*glm::vec4(camera[0].vup, 0.0f));

		vec3_tmp = glm::cross(camera[0].vup, camera[0].vrp - camera[0].prp);
		mat4_tmp = glm::translate(glm::mat4(1.0f), camera[0].vrp);
		mat4_tmp = glm::rotate(mat4_tmp, CAM_ROT_SENSITIVITY*dely*TO_RADIAN, vec3_tmp);
		mat4_tmp = glm::translate(mat4_tmp, -camera[0].vrp);

 		camera[0].prp = glm::vec3(mat4_tmp*glm::vec4(camera[0].prp, 1.0f));
	 	camera[0].vup = glm::vec3(mat4_tmp*glm::vec4(camera[0].vup, 0.0f));

		ViewMatrix[0] = glm::lookAt(camera[0].prp, camera[0].vrp, camera[0].vup);

		ViewProjectionMatrix[0] = ProjectionMatrix[0] * ViewMatrix[0];
		glutPostRedisplay();
	}
}

void motion_2(int x, int y) {
	glm::mat4 mat4_tmp;
	glm::vec3 vec3_tmp;
	float delx, dely;

	if (leftbutton_pressed) {
		delx = (float)(x - prevx), dely = -(float)(y - prevy);
		prevx = x, prevy = y;

		mat4_tmp = glm::translate(glm::mat4(1.0f), camera[0].vrp);
	 	mat4_tmp = glm::rotate(mat4_tmp, CAM_ROT_SENSITIVITY*delx*TO_RADIAN, camera[0].vup);
		mat4_tmp = glm::translate(mat4_tmp, -camera[0].vrp);

		camera[0].prp = glm::vec3(mat4_tmp*glm::vec4(camera[0].prp, 1.0f));
		camera[0].vup = glm::vec3(mat4_tmp*glm::vec4(camera[0].vup, 0.0f));

		vec3_tmp = glm::cross(camera[0].vup, camera[0].vrp - camera[0].prp);

		mat4_tmp = glm::translate(glm::mat4(1.0f), camera[0].vrp);
		mat4_tmp = glm::rotate(mat4_tmp, CAM_ROT_SENSITIVITY*dely*TO_RADIAN, vec3_tmp);
		mat4_tmp = glm::translate(mat4_tmp, -camera[0].vrp);

	 	camera[0].prp = glm::vec3(mat4_tmp*glm::vec4(camera[0].prp, 1.0f));
	 	camera[0].vup = glm::vec3(mat4_tmp*glm::vec4(camera[0].vup, 0.0f));

		ViewMatrix[0] = glm::lookAt(camera[0].prp, camera[0].vrp, camera[0].vup);

		ViewProjectionMatrix[0] = ProjectionMatrix[0] * ViewMatrix[0];
		glutPostRedisplay();
	}
}

void motion_3(int x, int y) {
	glm::mat4 mat4_tmp;
	glm::vec3 vec3_right, vec3_up, vec3_view, vec3_tmp;
	float delx, dely, length;

	if (leftbutton_pressed) {
		delx = (float)(x - prevx), dely = -(float)(y - prevy);
		prevx = x, prevy = y;
		length = sqrtf(delx*delx + dely*dely);
		if (length == 0.0f) return;
		
	 	vec3_view = glm::vec3(-ViewMatrix[0][0].z, -ViewMatrix[0][1].z, -ViewMatrix[0][2].z); // -n vector		
		vec3_up = camera[0].vup; // v vector
		vec3_right = glm::vec3(ViewMatrix[0][0].x, ViewMatrix[0][1].x, ViewMatrix[0][2].x); // u vector

		vec3_tmp = delx*vec3_right + dely*vec3_up;
		vec3_tmp = glm::cross(vec3_tmp, vec3_view); // the rotation axis
	
		mat4_tmp = glm::translate(glm::mat4(1.0f), camera[0].vrp);
		mat4_tmp = glm::rotate(mat4_tmp, CAM_ROT_SENSITIVITY*length*TO_RADIAN, vec3_tmp);
		mat4_tmp = glm::translate(mat4_tmp, -camera[0].vrp);

		camera[0].prp = glm::vec3(mat4_tmp*glm::vec4(camera[0].prp, 1.0f));
		camera[0].vup = glm::vec3(mat4_tmp*glm::vec4(camera[0].vup, 0.0f)); // vup is a vector.

		ViewMatrix[0] = glm::lookAt(camera[0].prp, camera[0].vrp, camera[0].vup);
 		camera[0].vup = glm::vec3(ViewMatrix[0][0].y, ViewMatrix[0][1].y, ViewMatrix[0][2].y); // vup may have changed slightly.
		ViewProjectionMatrix[0] = ProjectionMatrix[0] * ViewMatrix[0];
		glutPostRedisplay();
	}
}

void motion_4(int x, int y) {
	// Same as motion_3() except ViewMatrix[0] is computed directly without calling glm::lookAt();
	glm::mat4 mat4_tmp;
	glm::vec3 vec3_right, vec3_up, vec3_view, vec3_tmp;
	float delx, dely, length;

	if (leftbutton_pressed) {
		delx = (float)(x - prevx), dely = -(float)(y - prevy);
		prevx = x, prevy = y;
		length = sqrtf(delx*delx + dely*dely);
		if (length == 0.0f) return;

		vec3_view = glm::vec3(-ViewMatrix[0][0].z, -ViewMatrix[0][1].z, -ViewMatrix[0][2].z); // -n vector
		vec3_up = camera[0].vup; // v vector
		vec3_right = glm::vec3(ViewMatrix[0][0].x, ViewMatrix[0][1].x, ViewMatrix[0][2].x); // u vector

		vec3_tmp = delx*vec3_right + dely*vec3_up;
		vec3_tmp = glm::cross(vec3_tmp, vec3_view); // the rotation axis

		mat4_tmp = glm::translate(glm::mat4(1.0f), camera[0].vrp);
		mat4_tmp = glm::rotate(mat4_tmp, CAM_ROT_SENSITIVITY*length*TO_RADIAN, vec3_tmp);
		mat4_tmp = glm::translate(mat4_tmp, -camera[0].vrp); 

		camera[0].prp = glm::vec3(mat4_tmp*glm::vec4(camera[0].prp, 1.0f));
		camera[0].vup = glm::vec3(mat4_tmp*glm::vec4(camera[0].vup, 0.0f)); // vup is a vector.
		
		// set up the rotation part of the viewing transformation
		ViewMatrix[0] = glm::mat4(1.0f);
		ViewMatrix[0][0].y = camera[0].vup.x; ViewMatrix[0][1].y = camera[0].vup.y; ViewMatrix[0][2].y = camera[0].vup.z;

		vec3_tmp = glm::normalize(camera[0].prp - camera[0].vrp);
		ViewMatrix[0][0].z = vec3_tmp.x; ViewMatrix[0][1].z = vec3_tmp.y; ViewMatrix[0][2].z = vec3_tmp.z;

		vec3_tmp = glm::cross(camera[0].vup, vec3_tmp);
		ViewMatrix[0][0].x = vec3_tmp.x; ViewMatrix[0][1].x = vec3_tmp.y; ViewMatrix[0][2].x = vec3_tmp.z;

		// now finalize the veiwing transformation
		ViewMatrix[0]  = glm::translate(ViewMatrix[0], -camera[0].prp);

		ViewProjectionMatrix[0] = ProjectionMatrix[0] * ViewMatrix[0];
		glutPostRedisplay();
	}
}

void keyboard(unsigned char key, int x, int y) {
	if (key == 27) { // ESC key
		glutLeaveMainLoop(); // incur destuction callback for cleanups.
		return;
	}
	switch (key) {
	case 'w':
		view_mode = VIEW_WORLD;
		break;
	case 's':
		view_mode = VIEW_SQUARE;
		break;
	case 't':
		view_mode = VIEW_TIGER;
		break;
	case 'c':
		view_mode = VIEW_COW;
		break;
	case 'o': // loot at the origin.
		camera[0].vrp = glm::vec3(0.0f, 0.0f, 0.0f);  

		ViewMatrix[0] = glm::lookAt(camera[0].prp, camera[0].vrp, camera[0].vup);
		// in this example code, make vup always equal to the v direction.
		camera[0].vup = glm::vec3(ViewMatrix[0][0].y, ViewMatrix[0][1].y, ViewMatrix[0][2].y); 
		ViewProjectionMatrix[0] = ProjectionMatrix[0] * ViewMatrix[0];
		break;
	case 'r': // loot at the red TIGER.
		camera[0].vrp = glm::vec3(3.0f, 0.0f, 2.0f);  

		ViewMatrix[0] = glm::lookAt(camera[0].prp, camera[0].vrp, camera[0].vup);
		// in this example code, make vup always equal to the v direction.
		camera[0].vup = glm::vec3(ViewMatrix[0][0].y, ViewMatrix[0][1].y, ViewMatrix[0][2].y); 
		ViewProjectionMatrix[0] = ProjectionMatrix[0] * ViewMatrix[0];
		break;
	case '1':
		fprintf(stdout, "*** The current motion callback function is motion_1()...\n");
		glutMotionFunc(motion_1);
		break;
	case '2':
		fprintf(stdout, "*** The current motion callback function is motion_2()...\n");
		glutMotionFunc(motion_2);
		break;
	case '3':
		fprintf(stdout, "*** The current motion callback function is motion_3()...\n");
		glutMotionFunc(motion_3);
		break;
	case '4':
		fprintf(stdout, "*** The current motion callback function is motion_4()...\n");
		glutMotionFunc(motion_4);
		break;
	case 'm':
		rotation_mode_cow = 1 - rotation_mode_cow;
		if (rotation_mode_cow) 
			glutTimerFunc(100, timer_scene, 0);
		break;
	}
	glutPostRedisplay();
}

#define CAM_FAR_CLIP_DISTANCE_STEP	2.0f
#define CAM_MAX_FAR_CLIP_DISTANCE	100.0f
#define CAM_ZOOM_STEP			0.05f
#define CAM_MAX_ZOOM_IN_FACTOR	0.25f
#define CAM_MAX_ZOOM_OUT_FACTOR	2.50f

void special(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		camera[0].far_clip += CAM_FAR_CLIP_DISTANCE_STEP;
		if (camera[0].far_clip > CAM_MAX_FAR_CLIP_DISTANCE)
			camera[0].far_clip = CAM_MAX_FAR_CLIP_DISTANCE;
		fprintf(stdout, "*** New clipping distances: NEAR = %f, FAR = %f\n", camera[0].near_clip, camera[0].far_clip);
		ProjectionMatrix[0] = glm::perspective(camera[0].zoom_factor * camera[0].fov_y*TO_RADIAN, camera[0].aspect_ratio, camera[0].near_clip, camera[0].far_clip);
		ViewProjectionMatrix[0] = ProjectionMatrix[0] * ViewMatrix[0];
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN:
		camera[0].far_clip -= CAM_FAR_CLIP_DISTANCE_STEP;
		if (camera[0].near_clip > camera[0].far_clip)
			camera[0].far_clip = camera[0].near_clip + CAM_FAR_CLIP_DISTANCE_STEP;
		fprintf(stdout, "*** New clipping distances: NEAR = %f, FAR = %f\n", camera[0].near_clip, camera[0].far_clip);
		ProjectionMatrix[0] = glm::perspective(camera[0].zoom_factor * camera[0].fov_y*TO_RADIAN, camera[0].aspect_ratio, camera[0].near_clip, camera[0].far_clip);
		ViewProjectionMatrix[0] = ProjectionMatrix[0] * ViewMatrix[0];
		glutPostRedisplay();
		break;
	case GLUT_KEY_LEFT:
		camera[0].zoom_factor -= CAM_ZOOM_STEP;
		if (camera[0].zoom_factor < CAM_MAX_ZOOM_IN_FACTOR)
			camera[0].zoom_factor = CAM_MAX_ZOOM_IN_FACTOR;
		ProjectionMatrix[0] = glm::perspective(camera[0].zoom_factor * camera[0].fov_y*TO_RADIAN, camera[0].aspect_ratio, camera[0].near_clip, camera[0].far_clip);
		ViewProjectionMatrix[0] = ProjectionMatrix[0] * ViewMatrix[0];
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT:
		camera[0].zoom_factor += CAM_ZOOM_STEP;
		if (camera[0].zoom_factor > CAM_MAX_ZOOM_OUT_FACTOR)
			camera[0].zoom_factor = CAM_MAX_ZOOM_OUT_FACTOR;
		ProjectionMatrix[0] = glm::perspective(camera[0].zoom_factor * camera[0].fov_y*TO_RADIAN, camera[0].aspect_ratio, camera[0].near_clip, camera[0].far_clip);
		ViewProjectionMatrix[0] = ProjectionMatrix[0] * ViewMatrix[0];
		glutPostRedisplay();
		break;
	}
}

void reshape(int width, int height) {
	camera[0].aspect_ratio = (float)width / height;
	viewport[0].x = viewport[1].y = 0;
	viewport[0].w = (int) (0.75f*width); viewport[0].h = (int) (0.75f*height);
	ProjectionMatrix[0] = glm::perspective(camera[0].zoom_factor * camera[0].fov_y*TO_RADIAN, camera[0].aspect_ratio, 
		camera[0].near_clip, camera[0].far_clip);
	ViewProjectionMatrix[0] = ProjectionMatrix[0] * ViewMatrix[0];

	camera[1].aspect_ratio = camera[0].aspect_ratio; // for the time being ...
	viewport[1].x = (int) (0.75f*width); viewport[1].y = (int) (0.75f*height);
	viewport[1].w = (int) (0.25f*width); viewport[1].h = (int) (0.25*height);
	ProjectionMatrix[1] = glm::perspective(camera[1].fov_y*TO_RADIAN, camera[1].aspect_ratio, camera[1].near_clip, camera[1].far_clip);
	ViewProjectionMatrix[1] = ProjectionMatrix[1] * ViewMatrix[1];

	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &points_VAO);
	glDeleteBuffers(1, &points_VBO);

	glDeleteVertexArrays(1, &axes_VAO);
	glDeleteBuffers(1, &axes_VBO);

	glDeleteVertexArrays(N_OBJECTS, object_VAO);
	glDeleteBuffers(N_OBJECTS, object_VBO);
}
// End of callback function definitions

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutMouseFunc(mousepress);
	glutMotionFunc(motion_1);
	glutReshapeFunc(reshape);
	glutTimerFunc(100, timer_scene, 0);
	glutCloseFunc(cleanup);
}

#define PRINT_DEBUG_INFO  
void initialize_OpenGL(void) {
	// initialize the 0th camera.
	camera[0].prp = glm::vec3(0.0f, 0.0f, -20.0f);
	camera[0].vrp = glm::vec3(0.0f, 0.0f, 0.0f);
	camera[0].vup = glm::vec3(0.0f, 1.0f, 0.0f);
	ViewMatrix[0] = glm::lookAt(camera[0].prp, camera[0].vrp, camera[0].vup);
	camera[0].vup = glm::vec3(ViewMatrix[0][0].y, ViewMatrix[0][1].y, ViewMatrix[0][2].y); // in this example code, make vup always equal to the v direction.

#ifdef PRINT_DEBUG_INFO 
	print_mat4("Cam 0", ViewMatrix[0]);
#endif

	camera[0].fov_y = 15.0f;
	camera[0].aspect_ratio = 1.0f; // will be set when the viewing window popped up.
	camera[0].near_clip = 0.1f;
	camera[0].far_clip = 50.0f;
	camera[0].zoom_factor = 1.0f; // will be used for zoomming in and out.

	//initialize the 1st camera.
	camera[1].prp = glm::vec3(0.0f, 50.0f, 0.0f);
	camera[1].vrp = glm::vec3(0.0f, 0.0f, 0.0f);
	camera[1].vup = glm::vec3(0.0f, 0.0f, 1.0f);
	ViewMatrix[1] = glm::lookAt(camera[1].prp, camera[1].vrp, camera[1].vup);
	camera[1].vup = glm::vec3(ViewMatrix[1][0].y, ViewMatrix[1][1].y, ViewMatrix[1][2].y); // in this example code, make vup always equal to the v direction.

#ifdef PRINT_DEBUG_INFO 
	print_mat4("Cam 1", ViewMatrix[1]);
#endif

	camera[1].fov_y = 16.0f;
	camera[1].aspect_ratio = 1.0f; // will be set when the viewing window popped up.
	camera[1].near_clip = 0.1f;
	camera[1].far_clip = 100.0f;
	camera[1].zoom_factor = 1.0f; // will be used for zoomming in and out.

	rotation_angle_cow = 0.0f;
	view_mode = VIEW_WORLD;

	glClearColor(35 / 255.0f, 155 / 255.0f, 86 / 255.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);
}

void prepare_scene(void) {
	prepare_points();
	prepare_axes();
	prepare_square();
	prepare_tiger();
	prepare_cow();
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

void print_message(const char * m) {
	fprintf(stdout, "%s\n\n", m);
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

#define N_MESSAGE_LINES 3
void main(int argc, char *argv[]) {
	char program_name[64] = "Sogang CSE4170 (4.5.3) Simple Camera Transformation - ROTATION";
	char messages[N_MESSAGE_LINES][256] = { "    - Keys used: 'w', 's', 't', 'c', 'o', 'r', '1', '2', '3', '4', 'm', 'ESC'",
		"    - Special key used: KEY-UP/KEY-DOWN, KEY-LEFT/KEY-RIGHT",
		"    - Mouse used: Left Butten Click and Move"
	};

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(1200, 800);
	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}
