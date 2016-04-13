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

#include "imageLoader.h"
#include "glm.h"

//Amount of models and model ids
#define MODEL_COUNT 10
#define PLAYER_MOD 0
#define ESPERMA_MOD 1
#define JEFE_MOD 2
#define CONDON_MOD 3
#define PASTILLA_MOD 4

using namespace std;

#define MENU 0
#define HISTORIA 1
#define JUEGO 2
#define PAUSA 3
#define GAMEOVER 4
#define WIN 5

// Variables generales del juego
int estado = MENU; // 0-Menu 1-Historia 2-Juego 3-Pausa
int nivel = 1; //Nivel del juego
int puntaje = 0;

// Variables para el estado de historia
int contHistoria = 0;
int duracionHistoria = 60;

// Variables del juego
int tiempoJuego = 600;
int enemigosActuales = 2;
int vidaIncr = 5;   // Incremento de vida por nivel
float velIncr = 0.1; //Incremento de velocidad por nivel
float limiteSuperior = 4;
float limiteInferior = -5;
float limiteIzquierda = -10;
float limiteDerecha = 10;
char msg[200] = "";

// Variables para jefes
int movFluido = 5;
int contMovFluido = 0;

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
int textura=1;
static GLuint texName[36];
const int TEXTURE_COUNT=7;
int z=1;

// Variables para modelos
GLMmodel models[MODEL_COUNT];

// Estructura para personajes que contienen sus coordenadas
typedef struct Personaje{
    float x;
    float y;
    int vida;
    float velocidad;
    int armaActual; // 1-Condon 2-Pastilla
    float radioVertical;
    float radioHorizontal;
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
    float radioVertical;
    float radioHorizontal;
    Arma() {x = 0; y = 0; tipo = 1; velocidad = 0.5; damage = 10; radioVertical = 0.5; radioHorizontal = 0.5;}
    Arma(float cx, float cy, int tip) {x = cx; y = cy; tipo = tip; velocidad = 0; damage = 0; radioHorizontal = 0; radioVertical = 0;}
};

//Estructura para variables de enemigos
typedef struct Enemigo{
    float x;
    float y;
    int vidaMax;
    int vida;
    float velocidad;
    bool vivo;
    float radioVertical;
    float radioHorizontal;
    Enemigo() {x = 0; y = 0; vidaMax = 5; vida = 5; velocidad = 0.2; vivo = false; radioHorizontal = 0.5; radioVertical = 0.5;}
};

//Estructura para variables de jefes
typedef struct Jefe{
    float x;
    float y;
    int vidaMax;
    int vida;
    float velocidad;
    bool vivo;
    int movHorizontal;
    int movVertical;
    float radioVertical;
    float radioHorizontal;
    Jefe() {x = 0; y = 0; vidaMax = 5; vida = 5; velocidad = 0.2;
                movHorizontal = 0; movVertical = 0; vivo = false;
                radioHorizontal = 0.5; radioVertical = 0.5;}
};

//Inicializacion del personaje principal
Personaje globulo = {-10, 0, 100, 0.5, 1, 0.5, 0.5, false,false,false,false};

//Inicializacion de los enemigos
Enemigo enemigos[20];

// Inicializacion de los jefes
Jefe jefes[2];

// Inicializacion de las armas
vector<Arma> disparos;


///////////////////////////////////////////////////////////////////
///////    Funciones para checar colisiones    ////////////////////
///////////////////////////////////////////////////////////////////
bool checaColision(Arma arma, Enemigo enemigo)
{
    return !(arma.x + arma.radioHorizontal <= enemigo.x - enemigo.radioHorizontal ||
             arma.x - arma.radioHorizontal >= enemigo.x + enemigo.radioHorizontal ||
             arma.y + arma.radioVertical <= enemigo.y - enemigo.radioVertical ||
             arma.y - arma.radioVertical >= enemigo.y + enemigo.radioVertical);
}

bool checaColision(Arma arma, Jefe jefe)
{
    return !(arma.x + arma.radioHorizontal <= jefe.x - jefe.radioHorizontal ||
             arma.x - arma.radioHorizontal >= jefe.x + jefe.radioHorizontal ||
             arma.y + arma.radioVertical <= jefe.y - jefe.radioVertical ||
             arma.y - arma.radioVertical >= jefe.y + jefe.radioVertical);
}

bool checaColision(Personaje personaje, Enemigo enemigo)
{
    return !(personaje.x + personaje.radioHorizontal <= enemigo.x - enemigo.radioHorizontal ||
             personaje.x - personaje.radioHorizontal >= enemigo.x + enemigo.radioHorizontal ||
             personaje.y + personaje.radioVertical <= enemigo.y - enemigo.radioVertical ||
             personaje.y - personaje.radioVertical >= enemigo.y + enemigo.radioVertical);
}

///////////////////////////////////////////////////////////////////
///////    Regenera Enemigo con sus datos originales    ///////////
/////////Asigna coordenadas segun el nivel de enemigo//////////////
void regeneraEnemigo(int i)
{
    enemigos[i].x = rand()% ((i+1)*10) + 13;
    enemigos[i].y = rand() % 9 - 5;
    enemigos[i].vida = enemigos[i].vidaMax;
}

///////////////////////////////////////////////////////////////////
///////   Funcion para inicializar a jefe    //////////////////////
///////////////////////////////////////////////////////////////////
void creaJefe(int i, int v, float vel, float rv, float rh)
{
    jefes[i].vivo = true;
    jefes[i].x = rand()% 40 + 13;
    jefes[i].y = rand() % 9 - 5;
    jefes[i].vidaMax = v;
    jefes[i].vida = jefes[i].vidaMax;
    jefes[i].velocidad = vel;
    jefes[i].radioVertical = rv;
    jefes[i].radioHorizontal = rh;
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
            textura++;
        if(contHistoria >= 60)
            estado = 2; //Juego
    }

    if(estado == JUEGO) {

        //Dificultad de niveles
        if(tiempoJuego > 0){
            if(nivel != 5 && nivel != 10)
                tiempoJuego--;
        }
        else {
            tiempoJuego = 600;
            nivel++;
            if(nivel == 5) creaJefe(0,150,0.3,1,1);
            if(nivel == 10) creaJefe(0,300,0.5,2,2);
            enemigosActuales += 2;
            if(enemigosActuales > 20) enemigosActuales = 20;
            for(int i=enemigosActuales-2; i<enemigosActuales; i++){
                enemigos[i].vivo = true;
                if(nivel%2 == 0)
                    enemigos[i].vidaMax += (nivel*vidaIncr);
                else
                    enemigos[i].velocidad += (nivel*velIncr);
                regeneraEnemigo(i);
            }
        }


        //Movimiento de los jefes
        for(int i=0; i<2; i++) {
            if(jefes[i].vivo)
            {
                if(jefes[i].x > 10)
                    jefes[i].x -= jefes[i].velocidad;
                else {
                    if(contMovFluido >= movFluido) {
                        jefes[i].movHorizontal = rand() % 3 - 1;
                        jefes[i].movVertical = rand() % 3 - 1;
                        contMovFluido = 0;
                    }
                    jefes[i].x += jefes[i].movHorizontal * jefes[i].velocidad;
                    jefes[i].y += jefes[i].movVertical * jefes[i].velocidad;
                    contMovFluido++;
                }

                if(jefes[i].x < 0) {jefes[i].x = 0; jefes[i].movHorizontal * -1; }
                if(jefes[i].y > 4) {jefes[i].y = 4; jefes[i].movVertical * -1; }
                if(jefes[i].y < -5) {jefes[i].y = -5; jefes[i].movVertical * -1; }

                //Checa colision con las armas
                for (vector<Arma>::iterator it = disparos.begin() ; it != disparos.end();) {
                    if(checaColision(*(it),jefes[i])) {
                        jefes[i].vida -= it->damage;
                        if(jefes[i].vida <= 0){
                            puntaje += jefes[i].vidaMax;
                            jefes[i].vivo = false;
                            nivel++;
                            if(i == 1) estado = WIN;
                        }
                        it = disparos.erase(it);
                    }
                    else
                        it++;
                }
            }
        }


        //Movimiento de enemigos
        for(int i=0; i<20; i++) {
            if(enemigos[i].vivo) {
                // Actualiza posicion del enemigo
                enemigos[i].x -= enemigos[i].velocidad;

                //Checa colision con el ovulo y personaje
                if(enemigos[i].x < -11 || checaColision(globulo,enemigos[i])) {
                    globulo.vida -= 10;
                    if(globulo.vida <= 0) globulo.vida = 0;
                    regeneraEnemigo(i);
                }

                //Checa colision con las armas
                for (vector<Arma>::iterator it = disparos.begin() ; it != disparos.end();) {
                    if(checaColision(*(it),enemigos[i])) {
                        enemigos[i].vida -= it->damage;
                        if(enemigos[i].vida <= 0){
                            puntaje += ((i+2)/2) * 5;
                            regeneraEnemigo(i);
                        }
                        it = disparos.erase(it);
                    }
                    else
                        it++;
                }
            }
        }

        // Movimiento del globulo
        if(globulo.movArriba) globulo.y += globulo.velocidad;
        if(globulo.movAbajo) globulo.y -= globulo.velocidad;
        if(globulo.movIzq) globulo.x -= globulo.velocidad;
        if(globulo.movDer) globulo.x += globulo.velocidad;

        if(globulo.y > limiteSuperior) globulo.y = limiteSuperior;
        if(globulo.y < limiteInferior) globulo.y = limiteInferior;
        if(globulo.x < limiteIzquierda) globulo.x = limiteIzquierda;
        if(globulo.x > limiteDerecha) globulo.x = limiteDerecha;


        // Movimiento de Disparos
        for (vector<Arma>::iterator it = disparos.begin() ; it != disparos.end();){
            //Checa si se sale de la pantalla
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


    // Callbacks
    glutPostRedisplay();
    glutTimerFunc(100,myTimer,0);
}

///////////////////////////////////////////////////////////////////
/////////////////    Funcion display    ///////////////////////////
///////////////////////////////////////////////////////////////////
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3ub(255,255,255);

    //Menu e Historia
    if(estado == MENU || estado == HISTORIA) {
        //Habilitar el uso de texturas
        glEnable(GL_TEXTURE_2D);

        //Elegir la textura del Quads: textura cambia con el timer
        if(estado == MENU)
            glBindTexture(GL_TEXTURE_2D, texName[0]);

        if(estado == HISTORIA)
            glBindTexture(GL_TEXTURE_2D, texName[textura]);

        glBegin(GL_QUADS);
        //Asignar la coordenada de textura 0,0 al vertice
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-12.5f, -6.5f, 0);
         //Asignar la coordenada de textura 1,0 al vertice
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(12.5f, -6.5f, 0);
         //Asignar la coordenada de textura 1,1 al vertice
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(12.5f, 6.5, 0);
         //Asignar la coordenada de textura 0,1 al vertice
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-12.5f, 6.5f, 0);
        glEnd();
    }

    // Juego
    if(estado == JUEGO) {
        //Barra de vida
        glColor3ub(1,115,20);
        glRectf(-12,5,-12+(globulo.vida/10),6);
        glColor3ub(160,38,0);
        glRectf(-12,5,-2,6);

        // Nivel
        sprintf(msg, "%s%d", "Nivel ", nivel);
        glColor3ub(0,0,0);
        glRasterPos2d(0,5.5);
        for(int k=0;msg[k]!='\0'; k++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, msg[k]);

        //Tiempo
        sprintf(msg, "%d", tiempoJuego/10);
        glColor3ub(0,0,0);
        glRasterPos2d(0.5,5);
        for(int k=0;msg[k]!='\0'; k++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, msg[k]);

        //Puntaje
        sprintf(msg, "%s%d", "Puntaje: " ,puntaje);
        glColor3ub(0,0,0);
        glRasterPos2d(5,5.25);
        for(int k=0;msg[k]!='\0'; k++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, msg[k]);

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
            // Dibuja al enemigo
            if(enemigos[i].vivo) {
                glPushMatrix();
                glTranslated(enemigos[i].x,enemigos[i].y,0);
                //glRotated(angulo, 0, 1, 0);
                glScalef(1,1,1);
                glColor3ub(70,200,130);
                //glutSolidSphere(0.5,20,20);
                glmDraw(&models[ESPERMA_MOD], GLM_COLOR | GLM_FLAT);
                glPopMatrix();
            }
        }

        //Jefes
        for(int i=0; i<2; i++) {
            // Dibuja al enemigo
            if(jefes[i].vivo) {
                glPushMatrix();
                glScalef(1,1,0.1);
                glTranslated(jefes[i].x,jefes[i].y,0);
                glColor3ub(90,150,50);
                glutSolidSphere(jefes[i].radioHorizontal,20,20);
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
        case 13:
            //Cambia del menu a la historia
            if(estado == MENU)
                estado = HISTORIA;
            break;
        case 32:
            {
                Arma disparo(globulo.x,globulo.y,globulo.armaActual);
                //Tecla de espacio (disparo)
                if(disparo.tipo == 1){
                    disparo.velocidad = 0.6;
                    disparo.damage = 15;
                    disparo.radioVertical = 0.1;
                    disparo.radioHorizontal = 0.1;
                }
                else if(globulo.armaActual == 2){
                    disparo.velocidad = 0.3;
                    disparo.damage = 20;
                    disparo.radioVertical = 0.2;
                    disparo.radioHorizontal = 0.2;
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
/////////////    Path padre    ////////////////////////////////////
//////Le borramos el exceso para solo obtener el Path padre////////
void getParentPath()
{
    for (int i = (int)fullPath.length()-1; i>=0 && fullPath[i] != '\\'; i--) {
        fullPath.erase(i,1);
    }
}

///////////////////////////////////////////////////////////////////
////    Makes the image into a texture,     ///////////////////////
///////////  and returns the id of the texture  ///////////////////
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

///////////////////////////////////////////////////////////////////
/////////    Inicializacion de Imagenes y Texturas    /////////////
///////////////////////////////////////////////////////////////////
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

    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/Instrucciones.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);

    delete image;
}

///////////////////////////////////////////////////////////////////
/////////////    Inicializacion de Modelos    /////////////////////
///////////////////////////////////////////////////////////////////
void initModels()
{
    //General settings
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    //player
    std::string ruta = fullPath + "objects/player.obj";
    std::cout << "Filepath: " << ruta << std::endl;
    models[PLAYER_MOD] = *glmReadOBJ(ruta.c_str());
    glmUnitize(&models[PLAYER_MOD]);
    glmVertexNormals(&models[PLAYER_MOD], 90.0, GL_TRUE);

    //espermatozoides
    ruta = fullPath + "objects/espermatozoide.obj";
    std::cout << "Filepath: " << ruta << std::endl;
    models[ESPERMA_MOD] = *glmReadOBJ(ruta.c_str());
    glmUnitize(&models[ESPERMA_MOD]);
    glmVertexNormals(&models[ESPERMA_MOD], 90.0, GL_TRUE);

}

///////////////////////////////////////////////////////////////////
/////////////    Inicializacion principal    //////////////////////
///////////////////////////////////////////////////////////////////
void init()
{
    glClearColor (1.0, 1.0, 1.0, 1.0);
    glColor3f(1.0, 1.0, 1.0);

    initModels();
    initRendering();

    // Inicializacion de enemigos
    srand (time(NULL));
    int randX, randY;
    for(int i=0; i<2; i++){
        enemigos[i].vivo = true;
        randX = rand()% ((i+1)*10) + 13;
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
    glutCreateWindow("El Guardian Globular");
    //Obtener el path de los archivos
    getParentPath();
    //States y callbacks
    init();
    glutReshapeFunc(reshape); //Funcion reshape
    glutDisplayFunc(display); //Dibujo
    glutMouseFunc(myMouse); //Funciones del mouse
    glutKeyboardFunc(myKeyboard); //Funciones del teclado
    glutKeyboardUpFunc(myKeyboardUp);
    glutSpecialFunc(mySpecialKeyboard); //Teclas especiales
    glutTimerFunc(1000,myTimer,0); //Timer

    glutMainLoop();
}
