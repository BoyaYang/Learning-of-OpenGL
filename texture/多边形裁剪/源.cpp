/*
* Copyright (c) 1993-2003, Silicon Graphics, Inc.
* All Rights Reserved
*
* Permission to use, copy, modify, and distribute this software for any
* purpose and without fee is hereby granted, provided that the above
* copyright notice appear in all copies and that both the copyright
* notice and this permission notice appear in supporting documentation,
* and that the name of Silicon Graphics, Inc. not be used in
* advertising or publicity pertaining to distribution of the software
* without specific, written prior permission.
*
* THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS" AND
* WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE,
* INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR
* FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL SILICON
* GRAPHICS, INC.  BE LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT,
* SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
* OR ANY DAMAGES WHATSOEVER, INCLUDING WITHOUT LIMITATION, LOSS OF
* PROFIT, LOSS OF USE, SAVINGS OR REVENUE, OR THE CLAIMS OF THIRD
* PARTIES, WHETHER OR NOT SILICON GRAPHICS, INC.  HAS BEEN ADVISED OF
* THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE POSSESSION, USE
* OR PERFORMANCE OF THIS SOFTWARE.
*
* US Government Users Restricted Rights
* Use, duplication, or disclosure by the Government is subject to
* restrictions set forth in FAR 52.227.19(c)(2) or subparagraph
* (c)(1)(ii) of the Rights in Technical Data and Computer Software
* clause at DFARS 252.227-7013 and/or in similar or successor clauses
* in the FAR or the DOD or NASA FAR Supplement.  Unpublished - rights
* reserved under the copyright laws of the United States.
*
* Contractor/manufacturer is:
*	Silicon Graphics, Inc.
*	1500 Crittenden Lane
*	Mountain View, CA  94043
*	United State of America
*
* OpenGL(R) is a registered trademark of Silicon Graphics, Inc.
*/

/*  texbind.c
*  This program demonstrates using glBindTexture() by
*  creating and managing two textures.
*/
#include <stdlib.h>
#ifndef GLUT_DISABLE_ATEXIT_HACK
#define GLUT_DISABLE_ATEXIT_HACK
#endif
#include <glut.h>

#include <stdio.h>
#include <iostream>  

using namespace std;

#ifdef GL_VERSION_1_1
/*	Create checkerboard texture	*/

typedef struct
{
	GLubyte *imageData;//图像数据  
	GLuint bpp;//像素深度  
	GLuint width;//图像宽度  
	GLuint height;//图像高度  
	GLuint texID;//对应的纹理ID  
}TextureImage;

#define	checkImageWidth 64
#define	checkImageHeight 64
static GLubyte checkImage[checkImageHeight][checkImageWidth][4];
static GLubyte otherImage[checkImageHeight][checkImageWidth][4];

static GLuint texName[2];

TextureImage texture[1];



//加载TGA图像，生成纹理  
bool LoadTGA(TextureImage *texture, char *fileName);

bool LoadTGA(TextureImage *texture, char *filename)         // Loads A TGA File Into Memory  
{
	GLubyte     TGAheader[12] = { 0,0,2,0,0,0,0,0,0,0,0,0 };    // Uncompressed TGA Header  
	GLubyte     TGAcompare[12];                             // Used To Compare TGA Header  
	GLubyte     header[6];                                  // First 6 Useful Bytes From The Header  
	GLuint      bytesPerPixel;                              // Holds Number Of Bytes Per Pixel Used In The TGA File  
	GLuint      imageSize;                                  // Used To Store The Image Size When Setting Aside Ram  
	GLuint      temp;                                       // Temporary Variable  
	GLuint      type = GL_RGBA;                               // Set The Default GL Mode To RBGA (32 BPP)  

	FILE *file = fopen("haha.tga", "rb");                     // Open The TGA File  

	if (file == NULL ||                                       // Does File Even Exist?  
		fread(TGAcompare, 1, sizeof(TGAcompare), file) != sizeof(TGAcompare) ||  // Are There 12 Bytes To Read?  
		memcmp(TGAheader, TGAcompare, sizeof(TGAheader)) != 0 ||  // Does The Header Match What We Want?  
		fread(header, 1, sizeof(header), file) != sizeof(header))                // If So Read Next 6 Header Bytes  
	{
		if (file == NULL)                                   // Did The File Even Exist? *Added Jim Strong*  
			return false;                                   // Return False  
		else
		{
			fclose(file);                                   // If Anything Failed, Close The File  
			return false;                                   // Return False  
		}
	}

	texture->width = header[1] * 256 + header[0];           // Determine The TGA Width  (highbyte*256+lowbyte)  
	texture->height = header[3] * 256 + header[2];           // Determine The TGA Height (highbyte*256+lowbyte)  

															 //OpenGL中纹理只能使用24位或者32位的TGA图像  
	if (texture->width <= 0 ||                              // Is The Width Less Than Or Equal To Zero  
		texture->height <= 0 ||                              // Is The Height Less Than Or Equal To Zero  
		(header[4] != 24 && header[4] != 32))                   // Is The TGA 24 or 32 Bit?  
	{
		fclose(file);                                       // If Anything Failed, Close The File  
		return false;                                       // Return False  
	}

	texture->bpp = header[4];                            // Grab The TGA's Bits Per Pixel (24 or 32)  
	bytesPerPixel = texture->bpp / 8;                        // Divide By 8 To Get The Bytes Per Pixel  
	imageSize = texture->width*texture->height*bytesPerPixel;   // Calculate The Memory Required For The TGA Data  

	texture->imageData = (GLubyte *)malloc(imageSize);     // Reserve Memory To Hold The TGA Data  

	if (texture->imageData == NULL ||                          // Does The Storage Memory Exist?  
		fread(texture->imageData, 1, imageSize, file) != imageSize)    // Does The Image Size Match The Memory Reserved?  
	{
		if (texture->imageData != NULL)                     // Was Image Data Loaded  
			free(texture->imageData);                        // If So, Release The Image Data  

		fclose(file);                                       // Close The File  
		return false;                                       // Return False  
	}

	//RGB数据格式转换，便于在OpenGL中使用  
	for (GLuint i = 0; i<int(imageSize); i += bytesPerPixel)      // Loop Through The Image Data  
	{                                                       // Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)  
		temp = texture->imageData[i];                          // Temporarily Store The Value At Image Data 'i'  
		texture->imageData[i] = texture->imageData[i + 2];    // Set The 1st Byte To The Value Of The 3rd Byte  
		texture->imageData[i + 2] = temp;                    // Set The 3rd Byte To The Value In 'temp' (1st Byte Value)  
	}

	fclose(file);                                          // Close The File  

														   // Build A Texture From The Data  
	glGenTextures(1, &texture[0].texID);                    // Generate OpenGL texture IDs  

	glBindTexture(GL_TEXTURE_2D, texture[0].texID);         // Bind Our Texture  
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   // Linear Filtered  
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);   // Linear Filtered  

	if (texture[0].bpp == 24)                                 // Was The TGA 24 Bits  
	{
		type = GL_RGB;                                        // If So Set The 'type' To GL_RGB  
	}

	glTexImage2D(GL_TEXTURE_2D, 0, type, texture[0].width, texture[0].height, 0, type, GL_UNSIGNED_BYTE, texture[0].imageData);

	return true;                                            // Texture Building Went Ok, Return True  
}

void makeImage(void)
{
	int i, j, r, g, b;

	for (i = 0; i <checkImageWidth; i++)
	{
		for (j = 0; j < checkImageHeight; j++)
		{
			r = (i*j) % 255;
			g = (4 * i) % 255;
			b = (4 * j) % 255;
			checkImage[i][j][0] = (GLubyte)r;
			checkImage[i][j][1] = (GLubyte)g;
			checkImage[i][j][2] = (GLubyte)b;
			checkImage[i][j][3] = (GLubyte)255;
		}
	}
}

void makeCheckImages(void)
{
	int i, j, c;

	for (i = 0; i < checkImageHeight; i++) {
		for (j = 0; j < checkImageWidth; j++) {
			c = ((((i & 0x4) == 0) ^ ((j & 0x4)) == 0)) * 255;
			checkImage[i][j][0] = (GLubyte)c;
			checkImage[i][j][1] = (GLubyte)c;
			checkImage[i][j][2] = (GLubyte)c;
			checkImage[i][j][3] = (GLubyte)255;
			c = ((((i & 0x10) == 0) ^ ((j & 0x10)) == 0)) * 255;
			otherImage[i][j][0] = (GLubyte)c;
			otherImage[i][j][1] = (GLubyte)0;
			otherImage[i][j][2] = (GLubyte)0;
			otherImage[i][j][3] = (GLubyte)255;
		}
	}
}



void init(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);

	//makeCheckImages();
	makeImage();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(2, texName);
	glBindTexture(GL_TEXTURE_2D, texName[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, checkImageWidth,
		checkImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		checkImage);

	glBindTexture(GL_TEXTURE_2D, texName[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, checkImageWidth,
		checkImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		checkImage);
	glEnable(GL_TEXTURE_2D);
}

int initGraph()
{
	if (!LoadTGA(&texture[0], "haha.tga"))
		return GL_FALSE;
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	return GL_TRUE;
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, texture[0].texID);
	glRotatef(100, 0.0f, 0.0f, 0.0f);
	glRotatef(150, 1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-2.0, -1.0, 0.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-2.0, 1.0, 0.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(0.0, 1.0, 0.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(0.0, -1.0, 0.0);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, texture[0].texID);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(0.0, -1.0, 0.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(0.0, 1.0, 0.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(0, 1.0, -2);
	glTexCoord2f(1.0, 0.0); glVertex3f(0, -1.0, -2);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, texture[0].texID);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-2.0, 1.0, 0.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-2.0, 1.0, -2);
	glTexCoord2f(1.0, 1.0); glVertex3f(0, 1.0, -2);
	glTexCoord2f(1.0, 0.0); glVertex3f(0.0, 1.0, 0.0);
	glEnd();
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-2.0, -1.0, -2.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-2.0, 1.0, -2.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(0.0, 1.0, -2.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(0.0, -1.0, -2.0);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, texture[0].texID);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-2.0, -1.0, 0.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-2.0, 1.0, 0.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(-2.0, 1.0, -2);
	glTexCoord2f(1.0, 0.0); glVertex3f(-2.0, -1.0, -2);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, texture[0].texID);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-2.0, -1.0, 0.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-2.0, -1.0, -2);
	glTexCoord2f(1.0, 1.0); glVertex3f(0, -1.0, -2);
	glTexCoord2f(1.0, 0.0); glVertex3f(0.0, -1.0, 0.0);
	glEnd();
	glFlush();
}

void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	//glViewport(0, 0, 800, 800);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLfloat)w / (GLfloat)h, 1.0, 30.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -3.6);
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 27:
		exit(0);
		break;
	}
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(250, 250);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);
	//init();
	initGraph();
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMainLoop();
	return 0;
}
#else
int main(int argc, char** argv)
{
	fprintf(stderr, "This program demonstrates a feature which is not in OpenGL Version 1.0.\n");
	fprintf(stderr, "If your implementation of OpenGL Version 1.0 has the right extensions,\n");
	fprintf(stderr, "you may be able to modify this program to make it run.\n");
	return 0;
}
#endif

