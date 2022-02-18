#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

bool screen_flag = false;
int rightbuttonpressed = 0;
//float r = 0.0f, g = 0.0f, b = 0.0f; // Backgroud color = Black
float r = 250.0f / 255.0f, g = 128.0f / 255.0f, b = 114.0f / 255.0f; // Background color = Salmon
float line_x1 = -1.0f, line_x2 = 0.0f, line_y1 = 0.0f, line_y2 = 0.0f;	// end points of white line
int window_x = 500, window_y = 200;
int width = 500, height = 500;

void display(void) {
	glClearColor(r, g, b, 1.0f); 
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex2f(line_x1, line_y1); glVertex2f(line_x2, line_y2);
	glEnd();
	glFlush();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'r':
		r = 1.0f; g = b = 0.0f;
		fprintf(stdout, "$$$ The new window background color is (%5.3f, %5.3f, %5.3f).\n", r, g, b);
		glutPostRedisplay();
		break;
	case 'g':
		g = 1.0f; r = b = 0.0f;
		fprintf(stdout, "$$$ The new window background color is (%5.3f, %5.3f, %5.3f).\n", r, g, b);
		glutPostRedisplay();
		break;
	case 'b':
		b = 1.0f; r = g = 0.0f;
		fprintf(stdout, "$$$ The new window background color is (%5.3f, %5.3f, %5.3f).\n", r, g, b);
		glutPostRedisplay();
		break;
	case 's':
		r = 250.0f / 255.0f, g = 128.0f / 255.0f, b = 114.0f / 255.0f;
		fprintf(stdout, "$$$ The new window background color is (%5.3f, %5.3f, %5.3f).\n", r, g, b);
		glutPostRedisplay();
		break;
	case 'f':
		if (!screen_flag) {
			glutFullScreen();
			screen_flag = true;
		}

		else {
			glutInitWindowPosition(500, 200);
			glutPositionWindow(window_x, window_y);
			screen_flag = false;
		}

		break;
	case 'q':
		glutLeaveMainLoop(); 
		break;
	}
}

void special(int key, int x, int y) {
	int mode = glutGetModifiers();
	switch (key) {
	case GLUT_KEY_LEFT:	
		if (mode == GLUT_ACTIVE_CTRL)
			line_x1 = -1.0f, line_y1 = 0.0f;

		else if (mode == GLUT_ACTIVE_ALT) {
			if (window_x >= 10) window_x -= 10;
			else window_x = 0;
			glutPositionWindow(window_x, window_y);
		}

		else {
			r -= 0.1f;
			if (r < 0.0f) r = 0.0f;
			fprintf(stdout, "$$$ The new window background color is (%5.3f, %5.3f, %5.3f).\n", r, g, b);	
		}
		glutPostRedisplay();
		break;

	case GLUT_KEY_RIGHT:
		if (mode == GLUT_ACTIVE_CTRL)
			line_x1 = 1.0f, line_y1 = 0.0f;

		else if (mode == GLUT_ACTIVE_ALT) {
			if (window_x <= 1490 - width) window_x += 10;
			else window_x = 1500 - width;
			glutPositionWindow(window_x, window_y);
		}

		else {
			r += 0.1f;
			if (r > 1.0f) r = 1.0f;
			fprintf(stdout, "$$$ The new window background color is (%5.3f, %5.3f, %5.3f).\n", r, g, b);
		}
		glutPostRedisplay();
		break;

	case GLUT_KEY_DOWN:
		if (mode == GLUT_ACTIVE_CTRL) 
			line_x1 = 0.0f, line_y1 = -1.0f;

		else if (mode == GLUT_ACTIVE_ALT) {
			if (window_y <= 830 - height) window_y += 10;
			else window_y = 840 - height;
			glutPositionWindow(window_x, window_y);
		}

		else {
			g -= 0.1f;
			if (g < 0.0f) g = 0.0f;
			fprintf(stdout, "$$$ The new window background color is (%5.3f, %5.3f, %5.3f).\n", r, g, b);
		}
		glutPostRedisplay();
		break;
	case GLUT_KEY_UP:
		if (mode == GLUT_ACTIVE_CTRL)
			line_x1 = 0.0f, line_y1 = 1.0f;

		else if (mode == GLUT_ACTIVE_ALT) {
			if(window_y >= 10) window_y -= 10;
			glutPositionWindow(window_x, window_y);
		}

		else {
			g += 0.1f;
			if (g > 1.0f) g = 1.0f;
			fprintf(stdout, "$$$ The new window background color is (%5.3f, %5.3f, %5.3f).\n", r, g, b);
		}
		glutPostRedisplay();
		break;
	}
}

void mousepress(int button, int state, int x, int y) {
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN))
		fprintf(stdout, "*** The left mouse button was pressed at (%d, %d).\n", x, y);
	else if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN))
		rightbuttonpressed = 1;
	else if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_UP))
		rightbuttonpressed = 0;
}

void mousemove(int x, int y) {
	if (rightbuttonpressed) 
		fprintf(stdout, "$$$ The right mouse button is now at (%d, %d).\n", x, y);
}
	
void reshape(int width, int height) {
	fprintf(stdout, "### The new window size is %dx%d.\n", width, height);
}

void close(void) {
	fprintf(stdout, "\n^^^ The control is at the close callback function now.\n\n");
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutMouseFunc(mousepress);
	glutMotionFunc(mousemove);
	glutReshapeFunc(reshape);
 	glutCloseFunc(close);
}

void initialize_renderer(void) {
	register_callbacks();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = TRUE;
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

#define N_MESSAGE_LINES 4
void main(int argc, char *argv[]) {
	char program_name[64] = "Sogang CSE4170 SimplefreeGLUTcode";
	char messages[N_MESSAGE_LINES][256] = {
		"    - Keys used: 'r', 'g', 'b', 's', 'q'",
		"    - Special keys used: LEFT, RIGHT, UP, DOWN",
		"    - Mouse used: L-click, R-click and move",
		"    - Other operations: window size change"
	};

	glutInit(&argc, argv);
	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE); // <-- Be sure to use this profile for this example code!
 //	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_RGBA);

	glutInitWindowSize(width, height);
	glutInitWindowPosition(500, 200);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

 //   glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_EXIT); // default
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	
	glutMainLoop();
	fprintf(stdout, "^^^ The control is at the end of main function now.\n\n");
}