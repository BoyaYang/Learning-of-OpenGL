// DrawVertex.cpp : Defines the entry point for the console application.
//

//#include "StdAfx.h"
#include <stdlib.h>
#define GLUT_DISABLE_ATEXIT_HACK
#include "glut.h"//GL/

#define FALSE   0
#define TRUE    1
unsigned int RC, RC0, RC1;


#define LEFT 1//代表:0001
#define RIGHT 2//代表:0010
#define BOTTOM 4//代表:0100
#define TOP 8//代表:1000

double wxr = 0.5
, wxl = -0.5
, wyt = 0.5
, wyb = -0.5;
double Pointx[2] = { 0.3,-0.3 },
Pointy[2] = { 0,0.6 };

void display(void)
{
	glClearColor(0.0f, 1.0f, 0.75f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//绘制图形

	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex2f(wxl, wyt);
	glVertex2f(wxl, wyb);//1
	glVertex2f(wxl, wyt);
	glVertex2f(wxr, wyt);//2
	glVertex2f(wxr, wyt);
	glVertex2f(wxr, wyb);//3
	glVertex2f(wxl, wyb);
	glVertex2f(wxr, wyb);//4
	glVertex2f(Pointx[0], Pointy[0]);
	glVertex2f(Pointx[1], Pointy[1]);
	glEnd();


	glFlush();
}

unsigned int EnCode(double LinePx, double LinePy)//端点编码函数
{//顺序左右下上
	RC = 0;
	if (LinePx<wxl)
	{
		RC = RC | LEFT;
	}
	if (LinePx>wxr)
	{
		RC = RC | RIGHT;
	}
	if (LinePy<wyb)
	{
		RC = RC | BOTTOM;
	}
	if (LinePy>wyt)
	{
		RC = RC | TOP;
	}
	return RC;
}

void Cohen()//Cohen－Sutherland算法
{
	bool Change;
	double x, y;
	RC0 = EnCode(Pointx[0], Pointy[0]);
	RC1 = EnCode(Pointx[1], Pointy[1]);
	while (TRUE)
	{
		Change = FALSE;
		if (0 == (RC0 | RC1))
		{//简取之
			return;
		}
		else if (0 != (RC0 & RC1))
		{//简弃之
			return;
		}
		else
		{
			if (0 == RC0)//如果P0点在窗口内，交换P0和P1,保证p0点在窗口外
			{
				//交换点的坐标值
				double TPointx, TPointy;
				TPointx = Pointx[0];TPointy = Pointy[0];
				Pointx[0] = Pointx[1];Pointy[0] = Pointy[1];
				Pointx[1] = TPointx;Pointy[1] = TPointy;
				//交换点的编码值
				unsigned int TRC;
				TRC = RC0;RC0 = RC1;RC1 = TRC;
			}
			//按左、右、下、上的顺序裁剪
			if (RC0 & LEFT)//P0点位于窗口的左侧
			{
				x = wxl;//求交点y
				y = Pointy[0] + (Pointy[1] - Pointy[0])*(x - Pointx[0]) / (Pointx[1] - Pointx[0]);
				Pointx[0] = x;Pointy[0] = y;
				Change = TRUE;
				RC0 = EnCode(Pointx[0], Pointy[0]);RC1 = EnCode(Pointx[1], Pointy[1]);
			}
			if (RC0 & RIGHT)//P0点位于窗口的右侧
			{
				x = wxr;//求交点y
				y = Pointy[0] + (Pointy[1] - Pointy[0])*(x - Pointx[0]) / (Pointx[1] - Pointx[0]);
				Pointx[0] = x;Pointy[0] = y;
				Change = TRUE;
				RC0 = EnCode(Pointx[0], Pointy[0]);RC1 = EnCode(Pointx[1], Pointy[1]);
			}
			if (RC0 & BOTTOM)//P0点位于窗口的下侧
			{
				y = wyb;//求交点x
				x = Pointx[0] + (Pointx[1] - Pointx[0])*(y - Pointy[0]) / (Pointy[1] - Pointy[0]);
				Pointx[0] = x;Pointy[0] = y;
				Change = TRUE;
				RC0 = EnCode(Pointx[0], Pointy[0]);RC1 = EnCode(Pointx[1], Pointy[1]);
			}
			if (RC0 & TOP)//P0点位于窗口的上侧
			{
				y = wyt;//求交点x
				x = Pointx[0] + (Pointx[1] - Pointx[0])*(y - Pointy[0]) / (Pointy[1] - Pointy[0]);
				Pointx[0] = x;Pointy[0] = y;
				Change = TRUE;
				RC0 = EnCode(Pointx[0], Pointy[0]);RC1 = EnCode(Pointx[1], Pointy[1]);
			}
			if (0 != RC0)
				Pointx[0] = 0, Pointx[1] = 0, Pointy[0] = 0, Pointy[1] = 0;
			if (FALSE == Change)
			{
				return;
			}
		}
	}
}


void mouse(int button, int state, int x, int y)
{
	switch (button) {
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN) {
			Cohen();
			display();

		}
		break;

	default:
		break;
	}
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(500, 500);
	glutCreateWindow(argv[0]);
	glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutMainLoop();
	return 0;
}

