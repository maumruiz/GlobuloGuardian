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
#include "imageloader.h"

using namespace std;

//Estructura para variables de enemigos
typedef struct Enemigo{
    float x;
    float y;
    int vida;
    float velocidad;
    bool vivo;
    Enemigo() {x = 0; y = 0; vida = 100; velocidad = 0.2; vivo = false;}
};


// Estructura para personajes que contienen sus coordenadas
typedef struct Personaje{
    float x;
    float y;
    int vida;
    float velocidad;
};

//Inicializacion del personaje principal
Personaje globulo = {-10, 0, 100,0.5};

//Inicializacion de los enemigos
Enemigo enemigos[20];

// Variables para texturas
//__FILE__ is a preprocessor macro that expands to full path to the current file.
string fullPath = __FILE__;
using namespace std;
int textura=0;
static GLuint texName[36];
const int TEXTURE_COUNT=7;
int z=1;



//Le borramos el exceso para solo obtener el Path padre
void getParentPath()
{
    for (int i = (int)fullPath.length()-1; i>=0 && fullPath[i] != '\\'; i--) {
        fullPath.erase(i,1);
    }
}

//Makes the image into a texture, and returns the id of the texture
void loadTexture(Image* image,int k)
{

    glBindTexture(GL_TEXTURE_2D, texName[k]); //Tell OpenGL which texture to edit

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //Filtros de ampliacion y redución con cálculo mas cercano no es tan bueno pero es rápido
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_NEAREST);

    //Filtros de ampliacion y redución con cálculo lineal es mejo pero son más calculos
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);

    //Map the image to the texture
    glTexImage2D(GL_TEXTURE_2D,                //Always GL_TEXTURE_2D
                 0,                            //0 for now
                 GL_RGB,                       //Format OpenGL uses for image
                 image->width, image->height,  //Width and height
                 0,                            //The border of the image
                 GL_RGB, //GL_RGB, because pixels are stored in RGB format
                 GL_UNSIGNED_BYTE, //GL_UNSIGNED_BYTE, because pixels are stored
                 //as unsigned numbers
                 image->pixels);               //The actual pixel data
}

// Inicializacion de las imagenes y texturas
void initRendering()
{
    //Declaración del objeto Image
    Image* image;
    GLuint i=0;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glGenTextures(TEXTURE_COUNT, texName); //Make room for our texture


    char  ruta[200];
    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/Inicio.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);

    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/Historia.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);

    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/Nivel1.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);

    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/Inicio.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);

    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/Historia.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);

    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/Nivel1.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);


    delete image;
}


///////////////////////////////////////////////////////////////////
/////////////////    Funcion timer    /////////////////////////////
///////////////////////////////////////////////////////////////////
void myTimer(int i) {
    // Texturas
    textura = textura +1;
    if (textura > TEXTURE_COUNT) textura =0;

    //Movimiento de enemigos
    for(int i=0; i<20; i++) {
        if(enemigos[i].vivo)
            enemigos[i].x -= enemigos[i].velocidad;
    }

    glutPostRedisplay();
    glutTimerFunc(100,myTimer,0);
}

///////////////////////////////////////////////////////////////////
/////////////////    Funcion display    ///////////////////////////
///////////////////////////////////////////////////////////////////
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Barra de vida
    glColor3ub(0,255,0);
    glRectf(-12,5,-12+(globulo.vida/10),6);
    glColor3ub(255,0,0);
    glRectf(-12,5,-2,6);

    /*
    //Habilitar el uso de texturas
    glEnable(GL_TEXTURE_2D);

    //Elegir la textura del Quads: textura cambia con el timer
    glBindTexture(GL_TEXTURE_2D, texName[textura]);

    glBegin(GL_QUADS);
    //Asignar la coordenada de textura 0,0 al vertice
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-10.0f, -10.0f, 0);
     //Asignar la coordenada de textura 1,0 al vertice
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(10.0f, -10.0f, 0);
     //Asignar la coordenada de textura 1,1 al vertice
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(10.0f, 10.0f, 0);
     //Asignar la coordenada de textura 0,1 al vertice
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-10.0f, 10.0f, 0);
    glEnd();
    */

    //Globulo
    glPushMatrix();
    glScalef(1,1,0.1);
    glTranslated(globulo.x,globulo.y,0);
    glColor3ub(255,0,0);
    glutSolidSphere(0.5,20,20);
    glPopMatrix();

    //Enemigos
    for(int i=0; i<20; i++) {
        //Checa colision con el ovulo
        if(enemigos[i].x < -11) {
            globulo.vida -= 10;
            if(globulo.vida < 0) globulo.vida = 0;
            enemigos[i].x = rand() % 20 + 13;
            enemigos[i].y = rand() % 9 - 5;
        }

        // Actualiza la posicion del enemigo
        if(enemigos[i].vivo) {
            glPushMatrix();
            glScalef(1,1,0.1);
            glTranslated(enemigos[i].x,enemigos[i].y,0);
            glColor3ub(255,255,255);
            glutSolidSphere(0.5,20,20);
            glPopMatrix();
        }
    }

    glutSwapBuffers();
}

///////////////////////////////////////////////////////////////////
/////////////    Funciones del teclado    /////////////////////////
///////////////////////////////////////////////////////////////////
void myKeyboard(unsigned char key, int x, int y)
{
    switch(key)
    {
        case 'w':
            globulo.y += globulo.velocidad;
            if(globulo.y > 4) globulo.y = 4;
            break;
        case 's':
            globulo.y -= globulo.velocidad;
            if(globulo.y < -5) globulo.y = -5;
            break;
        case 'a':
            globulo.x -= globulo.velocidad;
            if(globulo.x < -10) globulo.x = -10;
            break;
        case 'd':
            globulo.x += globulo.velocidad;
            if(globulo.x > 10) globulo.x = 10;
            break;
        case 'v':
            globulo.vida -= 10;
            if(globulo.vida < 0) globulo.vida = 0;
        break;
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

///////////////////////////////////////////////////////////////////
/////////////    Funciones del teclado especial    ////////////////
///////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////
/////////////    Funciones del mouse    ///////////////////////////
///////////////////////////////////////////////////////////////////
void myMouse(int button, int state, int x, int y)
{
    glutPostRedisplay();
}

///////////////////////////////////////////////////////////////////
/////////////    Inicializacion principal    //////////////////////
///////////////////////////////////////////////////////////////////
void init()
{
    glClearColor (1.0, 1.0, 1.0, 1.0);
    glColor3f(0.0, 0.0, 0.0);
    // Para que las paredes se vean sólidas (no transparentes)

    // Inicializacion de enemigos
    srand (time(NULL));
    int randX, randY;
    for(int i=0; i<6; i++){
        enemigos[i].vivo = true;
        randX = rand() % 20 +13;
        randY = rand() % 9 - 5;
        enemigos[i].x = randX;
        enemigos[i].y = randY;
    }
}

///////////////////////////////////////////////////////////////////
/////////////    Funcion Reshape    ///////////////////////////////
///////////////////////////////////////////////////////////////////
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w / (float)h, 1.0, 20.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, 15, 0, 0, 0, 0, 1, 0);
}

///////////////////////////////////////////////////////////////////
/////////////    Main    //////////////////////////////////////////
///////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    //Initialize GLUT
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    //Window
    glutInitWindowSize(1000,500);
    glutInitWindowPosition(200,100);
    //Obtener el path de los archivos
    getParentPath();
    glutCreateWindow("Globulo Guardian");
    //States y callbacks
    init();
    initRendering();
    glutReshapeFunc(reshape); //Funcion reshape
    glutDisplayFunc(display); //Dibujo
    glutMouseFunc(myMouse); //Funciones del mouse
    glutKeyboardFunc(myKeyboard); //Funciones del teclado
    glutSpecialFunc(mySpecialKeyboard); //Teclas especiales
    glutTimerFunc(1000,myTimer,0); //Timer

    glutMainLoop();
}
