#include <stdio.h>
#include<string.h>
#include <GL/glew.h>        
#include <GL/freeglut.h>    
#include "ogldev_math_3d.h"    //����OpenGL��3d��ѧ��
#include "ogldev_util.h"	//���ڶ�ȡ�ı��ļ�

GLuint VBO;   //ȫ��GLuint���ñ��������������㻺��������			  
GLuint gWorldLocation;	// ƽ�Ʊ任һ�±����ľ������

// ����Ҫ��ȡ�Ķ�����ɫ���ű���Ƭ����ɫ���ű����ļ�������Ϊ�ļ���ȡ·��
const char* pVSFileName = "shader.vs";
const char* pFSFileName = "shader.fs";

static void RenderCallBack()
{
	glClear(GL_COLOR_BUFFER_BIT);  //�����ɫ����

	static float Scale = 0.0f;	 //ά��һ��������������ľ�̬������
	Scale += 0.001f;	//���ͼ��仯̫�����̫�����ɵ��ڴ���ֵ

	// 4x4��ƽ�Ʊ任����
	Matrix4f World;

	World.m[0][0] = 1.0f; World.m[0][1] = 0.0f; World.m[0][2] = 0.0f; World.m[0][3] = sinf(Scale);
	World.m[1][0] = 0.0f; World.m[1][1] = 1.0f; World.m[1][2] = 0.0f; World.m[1][3] = 0.0f;
	World.m[2][0] = 0.0f; World.m[2][1] = 0.0f; World.m[2][2] = 1.0f; World.m[2][3] = 0.0f;
	World.m[3][0] = 0.0f; World.m[3][1] = 0.0f; World.m[3][2] = 0.0f; World.m[3][3] = 1.0f;

	// ���������ݼ��ص�shader��
	glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, &World.m[0][0]);

	glEnableVertexAttribArray(0);   //������������
	glBindBuffer(GL_ARRAY_BUFFER, VBO);   //��GL_ARRAY_BUFFER������
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);   //���߽���bufer�е�����

	glDrawArrays(GL_TRIANGLES, 0, 3);   //�������Σ�3������
	glDisableVertexAttribArray(0);   //���ö�������
	glutSwapBuffers();   //����ǰ�󻺴�
}

static void InitializeGlutCallbacks()
{
	glutDisplayFunc(RenderCallBack);
	glutIdleFunc(RenderCallBack);	//����Ⱦ�ص�ע��Ϊȫ�����ûص�
}

static void CreateVertexBuffer()
{
	Vector3f Vertices[3];   //��������3������Ķ�������
	Vertices[0] = Vector3f(-1.0f, -1.0f, 0.0f); 
	Vertices[1] = Vector3f(1.0f, -1.0f, 0.0f); 
	Vertices[2] = Vector3f(0.0f, 1.0f, 0.0f); 

	glGenBuffers(1, &VBO);   //����������
	glBindBuffer(GL_ARRAY_BUFFER, VBO);   //��GL_ARRAY_BUFFER������

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);   //�󶨶�������
}

//ʹ��shader�ı�����shader���󣬲���shader����ɫ��������
static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// ����shader���Ͳ�����������shader����
	GLuint ShaderObj = glCreateShader(ShaderType);
	// ����Ƿ���ɹ�
	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(0);
	}

	// ����shader�Ĵ���Դ
	const GLchar* p[1];
	p[0] = pShaderText;
	GLint Lengths[1];
	Lengths[0]= strlen(pShaderText);
	glShaderSource(ShaderObj, 1, p, Lengths);
	glCompileShader(ShaderObj);	   // ����shader����

								   // ����shader��صĴ���
	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		exit(1);
	}

	// ������õ�shader����󶨵�program object���������
	glAttachShader(ShaderProgram, ShaderObj);
}

// ������ɫ������
static void CompileShaders()
{
	// ������ɫ������
	GLuint ShaderProgram = glCreateProgram();
	// ����Ƿ񴴽��ɹ�
	if (ShaderProgram == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	// �洢��ɫ���ı����ַ�������
	string vs, fs;

	// �ֱ��ȡ��ɫ���ļ��е��ı����ַ���������
	if (!ReadFile(pVSFileName, vs)) {
		exit(1);
	};
	if (!ReadFile(pFSFileName, fs)) {
		exit(1);
	};

	// ���Ӷ�����ɫ����Ƭ����ɫ��
	AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);
	AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

	// ����shader��ɫ�����򣬲���������ش���
	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };
	glLinkProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	// �����֤�ڵ�ǰ�Ĺ���״̬�����Ƿ���Ա�ִ��
	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);

	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	// ���õ�������������ʹ������ɹ�������shader����
	glUseProgram(ShaderProgram);

	// ��ѯ��ȡһ�±�����λ��
	gWorldLocation = glGetUniformLocation(ShaderProgram, "gWorld");	   
	assert(gWorldLocation != 0xFFFFFFFF);	// ������
}

int main(int argc, char ** argv) 
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); 

	glutInitWindowSize(400, 400);     
	glutInitWindowPosition(100, 100); 
	glutCreateWindow("ƽ�Ʊ任");  

	glutDisplayFunc(RenderCallBack);  //��ʼ��Ⱦ

	InitializeGlutCallbacks();

	// ���GLEW�Ƿ����������Ҫ��GLUT��ʼ��֮��
	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); //������պ����ɫֵ
	CreateVertexBuffer(); //�������㻺����

	CompileShaders();	// ������ɫ��

	glutMainLoop();   //֪ͨ��ʼGLUT���ڲ�ѭ��

	return 0;
}