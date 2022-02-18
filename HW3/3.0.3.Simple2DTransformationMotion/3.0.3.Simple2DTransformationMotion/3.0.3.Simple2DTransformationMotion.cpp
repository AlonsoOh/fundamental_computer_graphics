#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Shaders/LoadShaders.h"
GLuint h_ShaderProgram; // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

// include glm/*.hpp only if necessary
//#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, ortho, etc.
glm::mat4 ModelViewProjectionMatrix;
glm::mat4 ViewMatrix, ProjectionMatrix, ViewProjectionMatrix;

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))
#define CAKE_ROTATION_RADIUS win_width / 8

#define LOC_VERTEX 0

int win_width = 0, win_height = 0; 
float centerx = 0.0f, centery = 0.0f;
float sword_angle = 0.0f;
float change_angle = 1.0f;
int cake_start = 0;
int cake_clock = 0;

//variables for car movement
float car_scale_ratio = 1.0f;
float car1_x = -(win_width / 3), car1_y = 50;
float car2_x = win_width / 3, car2_y = 50;
float car1_width, car2_width;

int hit = 0;
int away = 0;
int order = 0;
int fly_rotate = 0;
float gradient;

// 2D 물체 정의 부분은 objects.h 파일로 분리
// 새로운 물체 추가 시 prepare_scene() 함수에서 해당 물체에 대한 prepare_***() 함수를 수행함.
// (필수는 아니나 올바른 코딩을 위하여) cleanup() 함수에서 해당 resource를 free 시킴.
#include "objects.h"

int collision_detect(float distX, float distY, float xRadius, float yRadius) {
	float dist = sqrt(distX * distX + distY + distY);
	if (dist <= xRadius + yRadius)
		return 1;
	return 0;
}

int outside_window(float x, float y) {
	if (y > win_height / 2.0f - 20) return 1;
	if (x > win_width / 2.0f - 20) return 2;
	if (y < -(win_height / 2.0f) + 20) return 3;
	if (x < -(win_width / 2.0f) + 20) return 4;

	return 0;
}

unsigned int timestamp = 0;
float tmpX = car1_x, tmpY = car1_y;
int reflect_angle = 0;

void timer(int value) {
	timestamp = (timestamp + 1) % UINT_MAX;
	cake_clock = (cake_clock + 1) % 270;

	if (!away) {
		if (collision_detect(car1_x - car2_x, 0, car1_width, car2_width)) {
			car_scale_ratio -= ((1.0f / 3.0f) / ((car1_width / 3.0f) / 10));
			car2_x -= 10;
		}
		else car2_x -= 20;

		if ((car2_x - car2_width / 2.0f) - (car1_x + car1_width / 2.0f) <= car1_width / 3.0f) {
			away = 1;
			order = 0;
			tmpX = car1_x;
			tmpY = car1_y;

			reflect_angle = rand() % 60;
			reflect_angle = rand() % 2 ? 180 - reflect_angle : 180 + reflect_angle;
			gradient = tan(reflect_angle * TO_RADIAN);
		}
	}

	else {
		fly_rotate = (fly_rotate + 50) % 360;
		switch (order) {
		case 0:
			car1_x += 30.0f * cos(reflect_angle * TO_RADIAN);
			car1_y = gradient * (car1_x - tmpX) + tmpY;

			if ((hit = outside_window(car1_x, car1_y))) {
				order = (order + 1) % 3;
				tmpX = car1_x;
				tmpY = car1_y;

				reflect_angle = rand() % 60;

				if (hit == 1) reflect_angle = 180 + reflect_angle;

				if (hit == 3) reflect_angle = 360 - reflect_angle;

				else if (hit == 4 && gradient > 0) reflect_angle = 360 - reflect_angle;

				gradient = tan(reflect_angle * TO_RADIAN);
			}
			break;

		case 1:
			car1_x += 30.0f * cos(reflect_angle * TO_RADIAN);

			car1_y = gradient * (car1_x - tmpX) + tmpY;
			if ((hit = outside_window(car1_x, car1_y)) > 0) {
				order = (order + 1) % 3;
				tmpX = car1_x;
				tmpY = car1_y;
				reflect_angle = rand() % 60;

				switch (hit) {
				case 1:
					if (gradient > 0) reflect_angle = 360 - reflect_angle;
					else reflect_angle += 180;
					break;

				case 2:
					if (gradient > 0) reflect_angle = 180 - reflect_angle;
					else reflect_angle += 180;
					break;

				case 3:
					if (gradient > 0) reflect_angle = 180 - reflect_angle;
					else reflect_angle = reflect_angle;
					break;

				case 4:
					if (gradient < 0) reflect_angle = reflect_angle;
					else reflect_angle = 360 - reflect_angle;
					break;
				}
				gradient = tan(reflect_angle * TO_RADIAN);
			}
			break;

		case 2:
			car1_x += 30.0f * cos(reflect_angle * TO_RADIAN);
			car1_y = gradient * (car1_x - tmpX) + tmpY;
			if (outside_window(car1_x, car1_y) > 0) {
				order = (order + 1) % 3;
				car1_x = -(win_width / 3.0f);
				car1_y = 50.0f;
				car2_x = win_width / 3.0f;
				car_scale_ratio = 1.0f;
				fly_rotate = 0;
				away = 0;
			}
			break;
		}
	}

	glutPostRedisplay();
	glutTimerFunc(10, timer, 0);
}

void display(void) {
	glm::mat4 ModelMatrix;
	
	glClear(GL_COLOR_BUFFER_BIT);
	
	ModelMatrix = glm::mat4(1.0f);
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_axes();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-400.0f, -800.0f + timestamp % win_height, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(5.0f, 5.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_hat();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -400.0f + timestamp % win_height, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(5.0f, 5.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_bear2();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(400.0f, 0.0f + timestamp % win_height, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(5.0f, 5.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_house();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(win_width / 4, 60.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_bear3();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-win_width / 4, -0.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_hat();

	//display swords
	float radius = win_width < win_height ? win_width / 7.0f : win_height / 7.0f;
	float sword_timer_pos = radius;
	glm::vec3 sword_co;
	for (float i = sword_angle; i < 360; i += 45) {
		sword_co = glm::vec3(radius * cos((float)i * TO_RADIAN), radius * sin((float)i * TO_RADIAN), 0.0f);
		ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));

		ModelMatrix = glm::translate(ModelMatrix, sword_co);
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.0f, 3.0f, 1.0f));
		ModelMatrix = glm::rotate(ModelMatrix, (-90 + i) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_sword();
	}

    //draw_house(); // in MC
	ModelMatrix = glm::mat4(1.0f);
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 100.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(5.0f, 5.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_bear1();

	//set of cake rotation
	for (int i = 0; i < 100; i++) {
		if (i % 6 == 1) {
			ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-CAKE_ROTATION_RADIUS, 0.0f, 0.0f));
			ModelMatrix = glm::rotate(ModelMatrix, cake_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-CAKE_ROTATION_RADIUS, 0.0f, 0.0f));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.0f, 3.0f, 1.0f));
		}

		else if (i % 6 == 2) {
			ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-2 * CAKE_ROTATION_RADIUS, 0.0f, 0.0f));
			ModelMatrix = glm::rotate(ModelMatrix, cake_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.0f, 3.0f, 1.0f));
		}

		else if (i % 6 == 3) {
			ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-3 * CAKE_ROTATION_RADIUS, 0.0f, 0.0f));
			ModelMatrix = glm::rotate(ModelMatrix, cake_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(CAKE_ROTATION_RADIUS, 0.0f, 0.0f));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.0f, 3.0f, 1.0f));
		}

		else if (i % 6 == 4) {
			ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(3 * CAKE_ROTATION_RADIUS, 0.0f, 0.0f));
			ModelMatrix = glm::rotate(ModelMatrix, -cake_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-CAKE_ROTATION_RADIUS, 0.0f, 0.0f));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.0f, 3.0f, 1.0f));
		}

		else if (i % 6 == 5) {
			ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(2 * CAKE_ROTATION_RADIUS, 0.0f, 0.0f));
			ModelMatrix = glm::rotate(ModelMatrix, -cake_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0, 0.0f, 0.0f));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.0f, 3.0f, 1.0f));
		}

		else {
			ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(CAKE_ROTATION_RADIUS, 0.0f, 0.0f));
			ModelMatrix = glm::rotate(ModelMatrix, -cake_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(CAKE_ROTATION_RADIUS, 0.0f, 0.0f));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.0f, 3.0f, 1.0f));
		}

		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_cake();
	}

	for (int i = 0; i < 100; i++) {
		if (i % 6 == 1) {
			ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-CAKE_ROTATION_RADIUS, 0.0f, 0.0f));
			ModelMatrix = glm::rotate(ModelMatrix, -cake_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-CAKE_ROTATION_RADIUS, 0.0f, 0.0f));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.0f, 3.0f, 1.0f));
		}

		else if (i % 6 == 2) {
			ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-2 * CAKE_ROTATION_RADIUS, 0.0f, 0.0f));
			ModelMatrix = glm::rotate(ModelMatrix, -cake_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.0f, 3.0f, 1.0f));
		}

		else if (i % 6 == 3) {
			ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-3 * CAKE_ROTATION_RADIUS, 0.0f, 0.0f));
			ModelMatrix = glm::rotate(ModelMatrix, -cake_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(CAKE_ROTATION_RADIUS, 0.0f, 0.0f));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.0f, 3.0f, 1.0f));
		}

		else if (i % 6 == 4) {
			ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(3 * CAKE_ROTATION_RADIUS, 0.0f, 0.0f));
			ModelMatrix = glm::rotate(ModelMatrix, cake_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-CAKE_ROTATION_RADIUS, 0.0f, 0.0f));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.0f, 3.0f, 1.0f));
		}

		else if (i % 6 == 5) {
			ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(2 * CAKE_ROTATION_RADIUS, 0.0f, 0.0f));
			ModelMatrix = glm::rotate(ModelMatrix, cake_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0, 0.0f, 0.0f));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.0f, 3.0f, 1.0f));
		}

		else {
			ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(CAKE_ROTATION_RADIUS, 0.0f, 0.0f));
			ModelMatrix = glm::rotate(ModelMatrix, cake_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(CAKE_ROTATION_RADIUS, 0.0f, 0.0f));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.0f, 3.0f, 1.0f));
		}

		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_bear2();
	}

	//car collision
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, win_height / 4, 0.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(car1_x, car1_y, 0.0f));

	ModelMatrix = glm::rotate(ModelMatrix, fly_rotate * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(car_scale_ratio, 1.0f, 1.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(car1_width / 16.0f, car1_width / 16.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_car();
	//
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, win_height / 4, 0.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(car2_x, car2_y, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(car2_width / 18.0f, car2_width / 18.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_car2();

	glFlush();	
}   

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	}
}

void special(int key, int x, int y) {
	switch (key)
	{
	case GLUT_KEY_LEFT:
		sword_angle += change_angle;
		if (sword_angle > 45.0f) sword_angle -= 45.0f;
		glutPostRedisplay();
		break;

	case GLUT_KEY_UP:
		if (change_angle < 10) change_angle += 1;
		printf("Rotation speed : %f\n", change_angle);
		break;
	
	case GLUT_KEY_RIGHT:
		sword_angle -= change_angle;
		if (sword_angle < 0.0f) sword_angle += 45.0f;
		glutPostRedisplay();
		break;

	case GLUT_KEY_DOWN:
		if (change_angle > 0) change_angle -= 1;
		printf("Rotation speed : %f\n", change_angle);
		break;
	}	
}

int leftbuttonpressed = 0;
void mouse(int button, int state, int x, int y) {
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN))
		leftbuttonpressed = 1;
	else if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP))
		leftbuttonpressed = 0;
}

void motion(int x, int y) {
	if (leftbuttonpressed) {
		centerx =  x - win_width/2.0f, centery = (win_height - y) - win_height/2.0f;
		glutPostRedisplay();
	}
} 
	
void reshape(int width, int height) {
	win_width = width, win_height = height;
	
	car1_width = win_width / 250.0f < 4 ? win_width / 250.0f : 4.0f;
	car2_width = win_width / 250.0f < 4 ? win_width / 250.0f : 4.0f;
	car1_width *= 10.0f;
	car2_width *= 12.0f;

  	glViewport(0, 0, win_width, win_height);
	ProjectionMatrix = glm::ortho(-win_width / 2.0, win_width / 2.0, 
		-win_height / 2.0, win_height / 2.0, -1000.0, 1000.0);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	update_axes();

	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &VAO_axes);
	glDeleteBuffers(1, &VBO_axes);

	glDeleteVertexArrays(1, &VAO_airplane);
	glDeleteBuffers(1, &VBO_airplane);

	glDeleteVertexArrays(1, &VAO_house);
	glDeleteBuffers(1, &VBO_house);
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutReshapeFunc(reshape);
	glutTimerFunc(10, timer, 0);
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
	glEnable(GL_MULTISAMPLE); 
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glClearColor(153 / 255.0f, 153 / 255.0f, 204 / 255.0f, 1.0f);
	ViewMatrix = glm::mat4(1.0f);
}

void prepare_scene(void) {
	prepare_axes();
	prepare_airplane();
	prepare_house();
	prepare_bear();
	prepare_car();
	prepare_car2();
	prepare_cake();
	prepare_sword();
	prepare_hat();
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
	fprintf(stdout, "    This program is made by Sae arm Oh(20141547) for HW#3\n");
	fprintf(stdout, "      Student of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 2
int main(int argc, char *argv[]) {
	char program_name[64] = "CSE4170 - Basic Computer Graphics 2020(by 20141547)";
	char messages[N_MESSAGE_LINES][256] = {
		"    - Keys used: 'ESC', direction key"
		"    - Mouse used: L-click and move"
	};

	glutInit (&argc, argv);
 	glutInitDisplayMode(GLUT_RGBA | GLUT_MULTISAMPLE);
	glutInitWindowSize (1200, 800);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}


