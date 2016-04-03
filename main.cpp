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
#include <vector>
#include <algorithm>
#include "imageloader.h"

using namespace std;

#define MENU 0
#define HISTORIA 1
#define JUEGO 2
#define PAUSA 3

// Variables generales del juego
int estado = MENU; // 0-Menu 1-Historia 2-Juego 3-Pausa
int nivel = 1; //Nivel del juego

// Variables para el estado de historia
int contHistoria = 0;
int duracionHistoria = 60;

// Variables del juego
int tiempoJuego = 0;
int enemigosActuales = 2;
int vidaIncr = 5;   // Incremento de vida por nivel
float velIncr = 0.1; //Incremento de velocidad por nivel
float limiteSuperior = 4;
float limiteInferior = -5;
float limiteIzquierda = -10;
float limiteDerecha = 10;

//Variables para disparos
int condonesFire = 0;
int pastillasFire = 0;
int condonesFireRate = 2;
int pastillasFireRate = 4;
bool condonShot = false;
bool pastillaShot = false;

// Variables para texturas
//__FILE__ is a preprocessor macro that expands to full path to the current file.
string fullPath = __FILE__;
int textura=0;
static GLuint texName[36];
const int TEXTURE_COUNT=7;
int z=1;

// Estructura para personajes que contienen sus coordenadas
typedef struct Personaje{
    float x;
    float y;
    int vida;
    float velocidad;
    int armaActual; // 1-Condon 2-Pastilla
    bool movArriba;
    bool movAbajo;
    bool movDer;
    bool movIzq;
};

typedef struct Arma {
    float x;
    float y;
    int tipo;
    float velocidad;
    int damage;
    Arma() {x = 0; y = 0; tipo = 1; velocidad = 0.5; damage = 10;}
    Arma(float cx, float cy, int tip) {x = cx; y = cy; tipo = tip; velocidad = 0; damage = 0;}
};

//Estructura para variables de enemigos
typedef struct Enemigo{
    float x;
    float y;
    int vida;
    float velocidad;
    bool vivo;
    Enemigo() {x = 0; y = 0; vida = 5; velocidad = 0.2; vivo = false;}
};

//Inicializacion del personaje principal
Personaje globulo = {-10, 0, 100, 0.5, 1,false,false,false,false};

//Inicializacion de los enemigos
Enemigo enemigos[20];

// Inicializacion de las armas
vector<Arma> disparos;

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
    /*
    // Texturas
    textura = textura +1;
    if (textura > TEXTURE_COUNT) textura = 1;
    */

    if(estado == HISTORIA) {
        contHistoria += 1;
        if(contHistoria%30 == 0)
            textura += 1;
        if(contHistoria >= 60)
            estado = 2; //Juego
    }

    if(estado == JUEGO) {
        // Movimiento del globulo
        if(globulo.movArriba) globulo.y += globulo.velocidad;
        if(globulo.movAbajo) globulo.y -= globulo.velocidad;
        if(globulo.movIzq) globulo.x -= globulo.velocidad;
        if(globulo.movDer) globulo.x += globulo.velocidad;

        if(globulo.y > limiteSuperior) globulo.y = limiteSuperior;
        if(globulo.y < limiteInferior) globulo.y = limiteInferior;
        if(globulo.x < limiteIzquierda) globulo.x = limiteIzquierda;
        if(globulo.x > limiteDerecha) globulo.x = limiteDerecha;

        //Dificultad de niveles
        if(tiempoJuego<600)
            tiempoJuego++;
        else {
            tiempoJuego = 0;
            nivel++;
            enemigosActuales += 2;
            if(enemigosActuales > 20) enemigosActuales = 20;
            for(int i=enemigosActuales-2; i<enemigosActuales; i++){
                enemigos[i].vivo = true;
                enemigos[i].x = rand() % 20 +13;
                enemigos[i].y = rand() % 9 - 5;
                if(nivel%2 == 0)
                    enemigos[i].vida += (nivel*vidaIncr);
                else
                    enemigos[i].velocidad += (nivel*velIncr);
            }
        }

        //Movimiento de enemigos
        for(int i=0; i<20; i++) {
            if(enemigos[i].vivo)
                enemigos[i].x -= enemigos[i].velocidad;
        }

        // Movimiento de Disparos
        for (vector<Arma>::iterator it = disparos.begin() ; it != disparos.end();){
            if (it->x > 13)
                it = disparos.erase(it);
            else{
                it->x += it->velocidad;
                ++it;
            }
        }

        // Fire rate de cada arma
        if(condonShot) {
            if(condonesFire<condonesFireRate)
                condonesFire++;
            else {
                condonesFire = 0;
                condonShot = false;
            }
        }

        if(pastillaShot) {
            if(pastillasFire<pastillasFireRate)
                pastillasFire++;
            else{
                pastillasFire = 0;
                pastillaShot = false;
            }
        }
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

    //Menu e Historia
    if(estado == MENU || estado == HISTORIA) {
        //Habilitar el uso de texturas
        glEnable(GL_TEXTURE_2D);

        //Elegir la textura del Quads: textura cambia con el timer
        if(estado == MENU)
            glBindTexture(GL_TEXTURE_2D, texName[1]);

        if(estado == HISTORIA)
            glBindTexture(GL_TEXTURE_2D, texName[textura]);

        glBegin(GL_QUADS);
        //Asignar la coordenada de textura 0,0 al vertice
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-10.0f, -5.0f, 0);
         //Asignar la coordenada de textura 1,0 al vertice
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(10.0f, -5.0f, 0);
         //Asignar la coordenada de textura 1,1 al vertice
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(10.0f, 5.0f, 0);
         //Asignar la coordenada de textura 0,1 al vertice
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-10.0f, 5.0f, 0);
        glEnd();
    }

    // Juego
    if(estado == JUEGO) {
        //Barra de vida
        glColor3ub(1,115,20);
        glRectf(-12,5,-12+(globulo.vida/10),6);
        glColor3ub(160,38,0);
        glRectf(-12,5,-2,6);

        //Globulo
        glPushMatrix();
        glScalef(1,1,0.1);
        glTranslated(globulo.x,globulo.y,0);
        glColor3ub(100,0,0);
        glutSolidSphere(0.5,20,20);
        glPopMatrix();

        // Disparos
        for (vector<Arma>::iterator it = disparos.begin() ; it != disparos.end(); ++it){
            // Condones
            if(it->tipo == 1){
                glPushMatrix();
                glScalef(1,1,0.1);
                glTranslated(it->x,it->y,0);
                glColor3ub(0,0,255);
                glutSolidSphere(0.1,20,20);
                glPopMatrix();
            }

            //Pastillas
            if(it->tipo == 2){
                glPushMatrix();
                glScalef(1,1,0.1);
                glTranslated(it->x,it->y,0);
                glColor3ub(0,255,255);
                glutSolidSphere(0.2,20,20);
                glPopMatrix();
            }

        }

        //Enemigos
        for(int i=0; i<20; i++) {
            //Checa colision con el ovulo
            if(enemigos[i].x < -11) {
                globulo.vida -= 10;
                if(globulo.vida < 0) globulo.vida = 0;
                enemigos[i].x = rand() % 20 + 13;
                enemigos[i].y = rand() % 9 - 5;
            }

            // Dibuja al enemigo
            if(enemigos[i].vivo) {
                glPushMatrix();
                glScalef(1,1,0.1);
                glTranslated(enemigos[i].x,enemigos[i].y,0);
                glColor3ub(70,200,130);
                glutSolidSphere(0.5,20,20);
                glPopMatrix();
            }
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
        case 'W':
            globulo.movArriba = true;
            break;
        case 's':
        case 'S':
            globulo.movAbajo = true;
            break;
        case 'a':
        case 'A':
            globulo.movIzq = true;
            break;
        case 'd':
        case 'D':
            globulo.movDer = true;
            break;
        case '1':
            globulo.armaActual = 1;
            break;
        case '2':
            globulo.armaActual = 2;
            break;
        case 32:
            {
                Arma disparo(globulo.x,globulo.y,globulo.armaActual);
                //Tecla de espacio (disparo)
                if(disparo.tipo == 1){
                    disparo.velocidad = 0.6;
                    disparo.damage = 15;
                }
                else if(globulo.armaActual == 2){
                    disparo.velocidad = 0.3;
                    disparo.damage = 20;
                }
                if(disparo.tipo == 1 && !condonShot) {
                    disparos.push_back(disparo);
                    condonShot = true;
                }
                else if(disparo.tipo == 2 && !pastillaShot) {
                    disparos.push_back(disparo);
                    pastillaShot = true;
                }
            }
            break;
        case 27:
            //Tecla de escape
            exit(0);
            break;
    }
    glutPostRedisplay();
}

///////////////////////////////////////////////////////////////////
/////////////    Funciones del teclado up    //////////////////////
///////////////////////////////////////////////////////////////////
void myKeyboardUp(unsigned char key, int x, int y)
{
    switch(key)
    {
        case 'w':
        case 'W':
            globulo.movArriba = false;
            break;
        case 's':
        case 'S':
            globulo.movAbajo = false;
            break;
        case 'a':
        case 'A':
            globulo.movIzq = false;
            break;
        case 'd':
        case 'D':
            globulo.movDer = false;
            break;
        case 32:
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
            //Cambia del menu a la historia
            if(estado == 0)
                estado++;
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
    for(int i=0; i<2; i++){
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
    glutKeyboardUpFunc(myKeyboardUp);
    glutSpecialFunc(mySpecialKeyboard); //Teclas especiales
    glutTimerFunc(1000,myTimer,0); //Timer

    glutMainLoop();
}
