#include <windows.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <time.h>

using namespace std;

void myTimer(int i) {
    glutPostRedisplay();
    glutTimerFunc(100,myTimer,1);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    //Globulo
    glPushMatrix();
    glScalef(1,1,0.1);
    glTranslated(-9,0,0);
    //glRotated(90,1,0,0);
    glColor3ub(255,0,0);
    glutSolidSphere(0.5,20,20);
    glPopMatrix();

    //Enemigo
    glPushMatrix();
    glScalef(1,1,0.1);
    glTranslated(5,2,0);
    //glRotated(90,1,0,0);
    glColor3ub(255,255,255);
    glutSolidSphere(0.5,20,20);
    glPopMatrix();

    //Enemigo
    glPushMatrix();
    glScalef(1,1,0.1);
    glTranslated(6,-3,0);
    //glRotated(90,1,0,0);
    glColor3ub(255,255,255);
    glutSolidSphere(0.5,20,20);
    glPopMatrix();

    //Enemigo
    glPushMatrix();
    glScalef(1,1,0.1);
    glTranslated(0,-1,0);
    //glRotated(90,1,0,0);
    glColor3ub(255,255,255);
    glutSolidSphere(0.5,20,20);
    glPopMatrix();

    glutSwapBuffers();
}

void myKeyboard(unsigned char key, int x, int y)
{
    switch(key)
    {
        case 32:
            //Tecla de espacio
            break;
        case 27:
            //Tecla de escape
            exit(0);
            break;
    }
    glutPostRedisplay();
}

void mySpecialKeyboard(int key, int x, int y)
{
    switch(key)
    {
        case GLUT_KEY_UP:

            break;
        case GLUT_KEY_DOWN:

            break;
    }
    glutPostRedisplay();
}

void myMouse(int button, int state, int x, int y)
{
    glutPostRedisplay();
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-10.0, 10.0, -5.0, 5.0, 1.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, 1, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void init()
{
    glClearColor (0.0, 0.0, 0.0, 1.0);
    glColor3f(0.0, 0.0, 0.0);
    // Para que las paredes se vean sólidas (no transparentes)

}

int main(int argc, char** argv)
{
    //Initialize GLUT
    glutInit(&argc,argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA);
    //Window
    glutInitWindowSize(1000,500);
    glutInitWindowPosition(200,100);
    glutCreateWindow("Globulo Guardian");
    //States y callbacks
    init();
    glutReshapeFunc(reshape); //Funcion reshape
    glutDisplayFunc(display); //Dibujo
    glutMouseFunc(myMouse); //Funciones del mouse
    glutKeyboardFunc(myKeyboard); //Funciones del teclado
    glutSpecialFunc(mySpecialKeyboard); //Teclas especiales
    glutTimerFunc(100,myTimer,1); //Timer

    glutMainLoop();
}
