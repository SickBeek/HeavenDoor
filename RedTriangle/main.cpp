/*
*        Computer Graphics Course - Shenzhen University
*      Week 6 - Camera Position and Control Skeleton Code
* ============================================================
*
* - 本代码仅仅是参考代码，具体要求请参考作业说明，按照顺序逐步完成。
* - 关于配置OpenGL开发环境、编译运行，请参考第一周实验课程相关文档。
*/

#include "Angel.h"
#include "TriMesh.h"

#pragma comment(lib, "glew32.lib")

#include <cstdlib>
#include <iostream>

using namespace std;

GLuint programID;
GLuint vertexArrayID;
GLuint vertexBufferID;
GLuint vertexIndexBuffer;

GLuint vPositionID;
GLuint modelViewMatrixID;

TriMesh* mesh = new TriMesh();

namespace Camera
{
	mat4 modelMatrix;
	mat4 viewMatrix;

	mat4 lookAt(const vec4& eye, const vec4& at, const vec4& up)
	{
		vec4 n = normalize(eye - at);
		vec3 u3 = normalize(cross(up, n));
		vec4 u = vec4(u3.x, u3.y, u3.z, 0.0);
		vec3 v3 = normalize(cross(n, u));
		vec4 v = vec4(v3.x, v3.y, v3.z, 0.0);
		vec4 t = vec4(0.0, 0.0, 0.0, 1.0);
		return mat4(u, v, n, t) * Translate(-eye);
	}
}

GLint mainWindow;

std::string const type[3] = {
	"SCALE","ROTATE","TRANSLATE"
};
const double DELTA_DELTA = 0.1;    // Delta的变化率
const double DEFAULT_DELTA = 0.3;    // 默认的Delta值

const int X_AXIS = 0;
const int Y_AXIS = 1;
const int Z_AXIS = 2;

const int TRANSFORM_SCALE = 0;
const int TRANSFORM_ROTATE = 1;
const int TRANSFORM_TRANSLATE = 2;

int currentTransform = TRANSFORM_TRANSLATE;    // 设置当前变换
int draw_mode = GL_FILL;

double scaleDelta = DEFAULT_DELTA;
double rotateDelta = DEFAULT_DELTA;
double translateDelta = DEFAULT_DELTA;

float R = 1;
int longAngle = 0;
int latAngle = 0;
int currentWindowSizeW;
int currentWindowSizeH;
float eyeRate = 5;

vec4 eye = vec4(0,0,0,1);;
vec4 at;
vec4 up;

vec3 scaleTheta(1.0, 1.0, 1.0);    // 缩放控制变量
vec3 rotateTheta(0.0, 0.0, 0.0);    // 旋转控制变量
vec3 translateTheta(0.0, 0.0, 0.0);    // 平移控制变量

//////////////////////////////////////////////////////////////////////////
// OpenGL 初始化

void init()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// 加载shader并且获取变量的位置
	programID = InitShader("vshader.glsl", "fshader.glsl");
	vPositionID = glGetAttribLocation(programID, "vPosition");
	//从顶点着色器中获取模-视变换矩阵位置
	modelViewMatrixID = glGetUniformLocation(programID, "modelViewMatrix");

	// 从外部读取三维模型文件
	mesh->read_off("cube.off");

	vector<vec3f> vs = mesh->v();
	vector<vec3i> fs = mesh->f();

	// 生成VAO
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	// 生成VBO，并绑定顶点坐标
	glGenBuffers(1, &vertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, vs.size() * sizeof(vec3f), vs.data(), GL_STATIC_DRAW);

	// 生成VBO，并绑定顶点索引
	glGenBuffers(1, &vertexIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, fs.size() * sizeof(vec3i), fs.data(), GL_STATIC_DRAW);

	// OpenGL相应状态设置
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	
}

//////////////////////////////////////////////////////////////////////////
// 渲染

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glUseProgram(programID);

	mat4 m(
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	);
	//	modelMatrix
	mat4 modelMatrix(
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	);
	modelMatrix *= Scale(scaleTheta[0], scaleTheta[1], scaleTheta[2]);
	modelMatrix *= RotateX(rotateTheta[0])*RotateY(rotateTheta[1])*RotateZ(rotateTheta[2]);
	modelMatrix *= Translate(translateTheta[0], translateTheta[1], translateTheta[2]);

	latAngle %= 360;
	longAngle %= 360;

	///glFrustum(-10, 10, -10, 10, -1, -10);
	//	viewMatrix
	
	glMatrixMode(GL_MODELVIEW);
	mat4 viewMatrix = 1;
	float x = 0, y = 0, z = 0;
	float latRad = latAngle * DegreesToRadians;
	float longRad = longAngle * DegreesToRadians;
	y = R * sin(latRad);
	x = R * cos(latRad)*sin(longRad);
	z = R * cos(latRad)*cos(longRad);

	//eye = vec4(y, x, z, 0);
	
	at = vec4(0, 0, -5, 1);
	up = vec4(0.0, 1.0, 0.0, 0);
	viewMatrix = LookAt(eye, at, up);

	//	modelViewMatrix
	mat4 modelViewMatrix = viewMatrix * modelMatrix;

	/*
	cout << "viewMatrix[" << viewMatrix << "]" << endl;
	cout << "modelMatrix[" << modelMatrix << "]" << endl;
	cout << "modelViewMatrix[" << modelViewMatrix << "]" << endl;
	*/
	cout << "eye=" << eye << endl;
	cout << "at=" << at << endl;
	cout << "up=" << up << endl;
	cout << "R=" << R << endl;
	cout << "angle=(" << longAngle << ',' << latAngle << ')' << endl << endl;

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, &modelViewMatrix[0][0]);

	glEnableVertexAttribArray(vPositionID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glVertexAttribPointer(
		vPositionID,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexIndexBuffer);

	glDrawElements(
		GL_TRIANGLES,
		int(mesh->f().size() * 3),
		GL_UNSIGNED_INT,
		(void*)0
	);

	glDisableVertexAttribArray(vPositionID);
	glUseProgram(0);

	glutSwapBuffers();
}

//////////////////////////////////////////////////////////////////////////
// 重新设置窗口

void reshape(GLsizei w, GLsizei h)
{
	glViewport(0, 0, w, h);
}

// 通过Delta值更新Theta
void updateTheta(int axis, int sign) {
	switch (currentTransform) {
	case TRANSFORM_SCALE:
		scaleTheta[axis] += sign * scaleDelta;
		break;
	case TRANSFORM_ROTATE:
		rotateTheta[axis] += sign * rotateDelta;
		break;
	case TRANSFORM_TRANSLATE:
		translateTheta[axis] += sign * translateDelta;
		break;
	}
}

// 复原Theta和Delta
void resetTheta()
{
	R = 1.0;
	longAngle = 0;
	latAngle = 0;
	eye = vec4(0, 0, 1, 1);

	scaleTheta = vec3(1.0, 1.0, 1.0);
	rotateTheta = vec3(0.0, 0.0, 0.0);
	translateTheta = vec3(0.0, 0.0, 0.0);
	scaleDelta = DEFAULT_DELTA;
	rotateDelta = DEFAULT_DELTA;
	translateDelta = DEFAULT_DELTA;
}

// 更新变化Delta值
void updateDelta(int sign)
{
	switch (currentTransform) {
	case TRANSFORM_SCALE:
		scaleDelta += sign * DELTA_DELTA;
		break;
	case TRANSFORM_ROTATE:
		rotateDelta += sign * DELTA_DELTA;
		break;
	case TRANSFORM_TRANSLATE:
		translateDelta += sign * DELTA_DELTA;
		break;
	}
}

// 键盘响应函数

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'q':
		updateTheta(X_AXIS, 1);
		break;
	case 'a':
		updateTheta(X_AXIS, -1);
		break;
	case 'w':
		updateTheta(Y_AXIS, 1);
		break;
	case 's':
		updateTheta(Y_AXIS, -1);
		break;
	case 'e':
		updateTheta(Z_AXIS, 1);
		break;
	case 'd':
		updateTheta(Z_AXIS, -1);
		break;
	case 'r':
		updateDelta(1);
		break;
	case 'f':
		updateDelta(-1);
		break;
	case 't':
		resetTheta();
		break;
	case 'j':
		latAngle -= eyeRate;
		break;
	case 'l':
		latAngle += eyeRate;
		break;
	case 'k':
		longAngle -= eyeRate;
		break;
	case 'i':
		longAngle += eyeRate;
		break;
	case 033:
		// Esc按键
		exit(EXIT_SUCCESS);
		break;
	case 32:// 空格键，切换模式
		currentTransform = (currentTransform + 1) % 3;
		std::cout << type[currentTransform] << std::endl;
		break;
	case 'g':
		if (draw_mode == GL_FILL)
			draw_mode = GL_LINE;
		else
			draw_mode = GL_FILL;
		break;
	case 'x':
		R += 0.2;
		break;
	case 'z':
		R -= 0.2;
		break;
	case '4':
		eye.x += 0.2;
		break;
	case '1':
		eye.x -= 0.2;
		break;
	case '5':
		eye.y += 0.2;
		break;
	case '2':
		eye.y -= 0.2;
		break;
	case '6':
		eye.z += 0.2;
		break;
	case '3':
		eye.z -= 0.2;
		break;
	}

	glutPostWindowRedisplay(mainWindow);
}

//////////////////////////////////////////////////////////////////////////

void idle(void)
{
	//glutPostRedisplay();
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// 鼠标响应函数
int trackX;
int trackY;
void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		trackX = x;
		trackY = y;
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		trackX = trackY = 0;
	}
}

/* 鼠标拖动事件 */
void mouseMove(int x, int y)
{
	int rate = 360;
	vec2 p = vec2(x, y);
	int dx = trackX - p.x;
	int dy = trackY - p.y;
	vec2 d = vec2(dx*rate / currentWindowSizeW, dy*rate / currentWindowSizeH);
	if (d.x == 0 && d.y == 0)
		return;

	trackX = x;
	trackY = y;
	cout << "track" << trackX << " " << trackY << endl;
	longAngle = longAngle - ceil(d.y);
	latAngle = latAngle + ceil(d.x);

	cout.setf(ios::showpos);
	cout << "d" << d << " angle=(" << longAngle << ',' << latAngle << ')' << endl;
	cout.unsetf(ios::showpos);
	
	glutPostWindowRedisplay(mainWindow);
}

void clean()
{
	glDeleteBuffers(1, &vertexBufferID);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &vertexArrayID);

	if (mesh) {
		delete mesh;
		mesh = NULL;
	}
}

void onReshape(int w, int h)
{
	currentWindowSizeW = w;
	currentWindowSizeH = h;
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(500, 500);
	mainWindow = glutCreateWindow("OpenGL-Tutorial");
	currentWindowSizeW = currentWindowSizeH = 500;

	glewInit();
	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMove);	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);

	glutMainLoop();

	clean();

	return 0;
}