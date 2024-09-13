#ifndef GRAPHICS
#define GRAPHICS

#include <gl/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TEXTURE_WIDTH 128
#define TEXTURE_HEIGHT 128

// screen
void screenSetup();
void screenClear();
void screenResize(float w, float h);

// drawing
unsigned int textureSetup(const char *fileName);
void drawTexture(float x, float y, float w, float h, unsigned int texture, float uvl, float uvr, float uvb, float uvt);

// screen functions

void screenSetup(){ // orthographic projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, 0, 10);
	glMatrixMode(GL_MODELVIEW);
	glClearColor(1.f, 1.f, 1.f, 1.f);
}

void screenClear(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void screenResize(float w, float h){
	glViewport(0, 0, w, h);
}

// drawing functions

unsigned int textureSetup(const char *fileName){
	
	// fetch data
	int width, height;
	unsigned char *data = stbi_load(fileName, &width, &height, NULL, 3);
	
	// generate texture
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	// assign data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
	return texture;
}

void drawTexture(float x, float y, float w, float h, unsigned int texture, float uvl, float uvr, float uvb, float uvt){
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glLoadIdentity();
	glTranslatef(x, y, 0.f);
	glScalef(w, h, 1.f);
	glColor3f(1.f, 1.f, 1.f);
	glBegin(GL_QUADS);
	glNormal3f(0.f, 0.f, 1.f);
	glTexCoord2d(uvr, uvt); glVertex2f(1.f, 1.f);
	glTexCoord2d(uvl, uvt); glVertex2f(-1.f, 1.f);
	glTexCoord2d(uvl, uvb); glVertex2f(-1.f, -1.f);
	glTexCoord2d(uvr, uvb); glVertex2f(1.f, -1.f);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

#endif