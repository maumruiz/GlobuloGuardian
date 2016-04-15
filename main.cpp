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
#define MODEL_COUNT 20
#define PLAYER_MOD 0
#define ESPERMA_MOD 1
#define JEFE1_MOD 2
#define JEFE2_MOD 3
#define CONDON_MOD 4
#define PASTILLA_MOD 5
#define UPVIDA_MOD 6
#define UPVELOCIDAD_MOD 7
#define UPCONDON_MOD 8
#define UPPASTILLA_MOD 9
#define UPINYECCION_MOD 10

//Estados del juego
#define MENU 0
#define HISTORIA 1
#define JUEGO 2
#define PAUSA 3
#define GAMEOVER 4
#define WIN 5
#define INSTRUCCIONES 6

//Armas
#define CONDON 1
#define PASTILLA 2

//Power Ups
#define UPVIDA 1
#define UPVELOCIDAD 2
#define UPCONDON 3
#define UPPASTILLA 4
#define UPINYECCION 5

using namespace std;

// Variables generales del juego
int estado = MENU; // 0-Menu 1-Historia 2-Juego 3-Pausa
int nivel = 4; //Nivel del juego
int puntaje = 0;

// Variables para el estado de historia
int contHistoria = 0;
int duracionHistoria = 60;

// Variables del juego
int tiempoJuego = 1200;
int enemigosActuales = 2;
int vidaIncr = 5;   // Incremento de vida por nivel
float velIncr = 0.02; //Incremento de velocidad por nivel
float limiteSuperior = 4;
float limiteInferior = -5;
float limiteIzquierda = -10;
float limiteDerecha = 10;
char msg[200] = "";

// Variables para jefes
int movFluido = 20;
int contMovFluido = 0;

//Variables para disparos
int condonesFire = 0;
int pastillasFire = 0;
int condonesFireRate = 7;
int pastillasFireRate = 12;
bool condonShot = false;
bool pastillaShot = false;
int extraDamageCondon = 0;
int extraDamagePastilla = 0;
int extraDamageInyeccion = 0;

// Variables para texturas
//__FILE__ is a preprocessor macro that expands to full path to the current file.
string fullPath = __FILE__;
int textura=6;
static GLuint texName[36]; //0-Menu 1-Instrucciones 2-Pausa 3-Win 4-Gameover 5..-Historia
const int TEXTURE_COUNT=17;
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
    int tipo; //1-Condon 2-Pastilla
    float velocidad;
    int damage;
    bool explosion;
    int explosionRate;
    int contExplosion;
    float radioVertical;
    float radioHorizontal;
    Arma() {x = 0; y = 0; tipo = 1; velocidad = 0.5; damage = 10; explosion = false; explosionRate = 5;
            contExplosion = 0; radioVertical = 0.5; radioHorizontal = 0.5;}
    Arma(float cx, float cy, int tip) {x = cx; y = cy; tipo = tip; velocidad = 0; damage = 0;
            explosion = false; explosionRate = 5; contExplosion = 0; radioHorizontal = 0; radioVertical = 0;}
};

typedef struct PowerUp {
    float x;
    float y;
    int tipo; //1-vida 2-velocidad 3-damage
    float velocidad;
    float radioVertical;
    float radioHorizontal;
    PowerUp() {x = 0; y = 0; tipo = 0; velocidad = 0.1; radioVertical = 0.5; radioHorizontal = 0.5;}
    PowerUp(float cx, float cy, int tip) {x = cx; y = cy; tipo = tip; velocidad = 0.5; radioHorizontal = 0.5; radioVertical = 0.5;}
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
    Enemigo() {x = 0; y = 0; vidaMax = 5; vida = 5; velocidad = 0.07; vivo = false; radioHorizontal = 1.0; radioVertical = 0.3;}
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
Personaje globulo = {-10, 0, 100, 0.2, 1, 0.7, 0.7, false,false,false,false};

//Inicializacion de los enemigos
Enemigo enemigos[20];

// Inicializacion de los jefes
Jefe jefes[2];

// Inicializacion de las armas
vector<Arma> disparos;


//Inicializacion de PowerUps
PowerUp powerups[6];


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

bool checaColision(Personaje personaje, PowerUp powerup)
{
    return !(personaje.x + personaje.radioHorizontal <= powerup.x - powerup.radioHorizontal ||
             personaje.x - personaje.radioHorizontal >= powerup.x + powerup.radioHorizontal ||
             personaje.y + personaje.radioVertical <= powerup.y - powerup.radioVertical ||
             personaje.y - personaje.radioVertical >= powerup.y + powerup.radioVertical);
}

bool checaExplosion(Arma arma, Enemigo enemigo)
{
    return !(arma.x + arma.radioHorizontal + (arma.explosionRate/10.0) <= enemigo.x - enemigo.radioHorizontal ||
             arma.x - arma.radioHorizontal - (arma.explosionRate/10.0) >= enemigo.x + enemigo.radioHorizontal ||
             arma.y + arma.radioVertical + (arma.explosionRate/10.0) <= enemigo.y - enemigo.radioVertical ||
             arma.y - arma.radioVertical - (arma.explosionRate/10.0) >= enemigo.y + enemigo.radioVertical);
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
///////    Regenera Power Ups    //////////////////////////////////
///////////////////////////////////////////////////////////////////
void regeneraPowerUps(int i)
{
    powerups[i].x = rand()% 300 + 80;
    powerups[i].y = rand() % 9 - 5;
}

///////////////////////////////////////////////////////////////////
///////   Funcion para inicializar a jefe    //////////////////////
///////////////////////////////////////////////////////////////////
void creaJefe(int i, int v, float vel, float rv, float rh)
{
    jefes[i].vivo = true;
    jefes[i].x = rand()% 50 + 13;
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

    if(estado == JUEGO) {

        //Dificultad de niveles
        if(tiempoJuego > 0){
            if(nivel != 5 && nivel != 10)
                tiempoJuego--;
        }
        else {
            estado = HISTORIA;
            tiempoJuego = 1200;
            nivel++;
            if(nivel == 5) creaJefe(0,400,0.2,1.1,1.1);
            if(nivel == 10) creaJefe(1,800,0.4,2,2);
            enemigosActuales += 2;
            if(enemigosActuales > 20) enemigosActuales = 20;
            for(int i=enemigosActuales-2; i<enemigosActuales; i++){
                enemigos[i].vivo = true;
                if(nivel%2 == 0)
                    enemigos[i].vidaMax += (nivel*vidaIncr);
                else
                    enemigos[i].velocidad += (nivel*velIncr);
            }
            for(int i=0; i<enemigosActuales; i++)
                regeneraEnemigo(i);
            for(int i=0; i<6; i++)
                regeneraPowerUps(i);
        }


        //Actualiza los jefes
        for(int i=0; i<2; i++) {
            if(jefes[i].vivo)
            {
                // Movimiento del jefe
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
                            if(i == 0) {
                                estado = HISTORIA;
                                enemigosActuales += 2;
                                if(enemigosActuales > 20) enemigosActuales = 20;
                                for(int i=enemigosActuales-2; i<enemigosActuales; i++){
                                    enemigos[i].vivo = true;
                                    if(nivel%2 == 0)
                                        enemigos[i].vidaMax += (nivel*vidaIncr);
                                    else
                                        enemigos[i].velocidad += (nivel*velIncr);
                                }
                                for(int i=0; i<enemigosActuales; i++)
                                    regeneraEnemigo(i);
                                for(int i=0; i<6; i++)
                                    regeneraPowerUps(i);
                            }
                            if(i == 1) estado = WIN;
                        }

                        if(it->tipo != PASTILLA)
                            it = disparos.erase(it);
                        else {
                            it->explosion = true;
                            for(int j=0; j<enemigosActuales; j++)
                                if(j != i && checaExplosion(*(it),enemigos[j])){
                                    enemigos[j].vida -= it->damage/2;
                                    if(enemigos[j].vida <= 0){
                                        puntaje += ((j+2)/2) * 5;
                                        regeneraEnemigo(j);
                                    }
                                }
                            it++;
                        }
                    }
                    else
                        it++;
                }
            }
        }


        //Actualiza enemigos
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
                        if(it->tipo != PASTILLA)
                            it = disparos.erase(it);
                        else {
                            it->explosion = true;
                            for(int j=0; j<enemigosActuales; j++)
                                if(j != i && checaExplosion(*(it),enemigos[j])){
                                    enemigos[j].vida -= it->damage/2;
                                    if(enemigos[j].vida <= 0){
                                        puntaje += ((j+2)/2) * 5;
                                        regeneraEnemigo(j);
                                    }
                                }
                            it++;
                        }
                    }
                    else
                        it++;
                }
            }
        }

        //Actualiza power ups
        for(int i=0; i<6; i++) {
                // Actualiza posicion del power up
                powerups[i].x -= powerups[i].velocidad;

                // Checa colision con el ovulo
                if(powerups[i].x < -11) {
                    regeneraPowerUps(i);
                }

                //Checa colision con el personaje
                if(checaColision(globulo,powerups[i])) {
                    if(powerups[i].tipo == UPVIDA) {
                        globulo.vida += 30;
                        if(globulo.vida > 100) globulo.vida = 100;
                    } else
                    if(powerups[i].tipo == UPVELOCIDAD) {
                        globulo.velocidad += 0.1;
                    } else
                    if(powerups[i].tipo == UPCONDON) {
                        extraDamageCondon += 5;
                    } else
                    if(powerups[i].tipo == UPPASTILLA) {
                        extraDamagePastilla += 5;
                    } else
                    if(powerups[i].tipo == UPINYECCION) {
                        extraDamageInyeccion += 5;
                    }
                    regeneraPowerUps(i);
                }
        }

        // Actualiza el globulo
        if(globulo.movArriba) globulo.y += globulo.velocidad;
        if(globulo.movAbajo) globulo.y -= globulo.velocidad;
        if(globulo.movIzq) globulo.x -= globulo.velocidad;
        if(globulo.movDer) globulo.x += globulo.velocidad;

        if(globulo.y > limiteSuperior) globulo.y = limiteSuperior;
        if(globulo.y < limiteInferior) globulo.y = limiteInferior;
        if(globulo.x < limiteIzquierda) globulo.x = limiteIzquierda;
        if(globulo.x > limiteDerecha) globulo.x = limiteDerecha;


        // Actualiza los Disparos
        for (vector<Arma>::iterator it = disparos.begin() ; it != disparos.end();){
            // Checa si algun arma exploto
            if(it->explosion) {
                if(it->contExplosion <= it->explosionRate) {
                    it->contExplosion++;
                    it++;
                }
                else
                    it = disparos.erase(it);
            }
            else
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
    glutTimerFunc(50,myTimer,0);
}

///////////////////////////////////////////////////////////////////
/////////////////    Funcion display    ///////////////////////////
///////////////////////////////////////////////////////////////////
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3ub(255,255,255);

    //Menu e Historia
    if(estado == MENU || estado == HISTORIA || estado == INSTRUCCIONES ||
       estado == PAUSA || estado == WIN || estado == GAMEOVER) {
        //Habilitar el uso de texturas
        glEnable(GL_TEXTURE_2D);

        //Elegir la textura del Quads:
        if(estado == MENU)
            glBindTexture(GL_TEXTURE_2D, texName[0]);

        if(estado == INSTRUCCIONES)
            glBindTexture(GL_TEXTURE_2D, texName[1]);

        if(estado == PAUSA)
            glBindTexture(GL_TEXTURE_2D, texName[2]);

        if(estado == WIN)
            glBindTexture(GL_TEXTURE_2D, texName[3]);

        if(estado == GAMEOVER)
            glBindTexture(GL_TEXTURE_2D, texName[4]);

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
    else
    // Juego
    if(estado == JUEGO) {

        // Suelo
            //Habilitar el uso de texturas
        glEnable(GL_TEXTURE_2D);
            //Elegir la textura del Quads:
        glBindTexture(GL_TEXTURE_2D, texName[5]);
        glBegin(GL_QUADS);
            //Asignar la coordenada de textura 0,0 al vertice
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-20.0f, -9.0f, -5);
            //Asignar la coordenada de textura 1,0 al vertice
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(20.0f, -9.0f, -5);
            //Asignar la coordenada de textura 1,1 al vertice
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(20.0f, 9.0, -5);
            //Asignar la coordenada de textura 0,1 al vertice
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-20.0f, 9.0f, -5);
        glEnd();

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
        sprintf(msg, "%d", tiempoJuego/20);
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
        glTranslated(globulo.x,globulo.y,0);
        glRotated(90, 1, 0, 0);
        glScalef(0.7,0.7,0.7);
        glColor3ub(100,0,0);
        glmDraw(&models[PLAYER_MOD], GLM_COLOR | GLM_FLAT);
        glPopMatrix();

        // Disparos
        for (vector<Arma>::iterator it = disparos.begin() ; it != disparos.end(); ++it){
            // Condones
            if(it->tipo == CONDON){
                glPushMatrix();
                glTranslated(it->x,it->y,0);
                glRotated(-90, 0, 0, 1);
                glScalef(0.5,0.5,0.5);
                glColor3ub(0,0,255);
                //glutSolidSphere(0.1,20,20);
                glmDraw(&models[CONDON_MOD], GLM_COLOR | GLM_FLAT);
                glPopMatrix();
            }

            //Pastillas
            if(it->tipo == PASTILLA){
                glPushMatrix();
                glTranslated(it->x,it->y,0);
                glRotated(90, 1, 0, 0);
                glScalef(0.3+it->contExplosion/10.0,0.3+it->contExplosion/10.0,0.3+it->contExplosion/10.0);
                glColor3ub(0,255,255);
                //glutSolidSphere(0.2+it->contExplosion/10.0,20,20);
                glmDraw(&models[PASTILLA_MOD], GLM_COLOR | GLM_FLAT);
                glPopMatrix();
            }

            //Box
                glLineWidth(1);
                glBegin(GL_LINES);
                glColor3ub(0, 0, 0);
                glVertex2f(it->x-it->radioHorizontal, it->y+it->radioVertical);
                glVertex2f(it->x+it->radioHorizontal, it->y+it->radioVertical);
                glVertex2f(it->x-it->radioHorizontal, it->y-it->radioVertical);
                glVertex2f(it->x+it->radioHorizontal, it->y-it->radioVertical);
                glVertex2f(it->x-it->radioHorizontal, it->y+it->radioVertical);
                glVertex2f(it->x-it->radioHorizontal, it->y-it->radioVertical);
                glVertex2f(it->x+it->radioHorizontal, it->y+it->radioVertical);
                glVertex2f(it->x+it->radioHorizontal, it->y-it->radioVertical);
                glEnd();

        }

        //PowerUps
        for(int i=0; i<6; i++) {
            // Dibuja al enemigo
            glPushMatrix();
            //Box
            glLineWidth(1);
            glBegin(GL_LINES);
            glColor3ub(0, 0, 0);
            glVertex2f(powerups[i].x-powerups[i].radioHorizontal, powerups[i].y+powerups[i].radioVertical);
            glVertex2f(powerups[i].x+powerups[i].radioHorizontal, powerups[i].y+powerups[i].radioVertical);
            glVertex2f(powerups[i].x-powerups[i].radioHorizontal, powerups[i].y-powerups[i].radioVertical);
            glVertex2f(powerups[i].x+powerups[i].radioHorizontal, powerups[i].y-powerups[i].radioVertical);
            glVertex2f(powerups[i].x-powerups[i].radioHorizontal, powerups[i].y+powerups[i].radioVertical);
            glVertex2f(powerups[i].x-powerups[i].radioHorizontal, powerups[i].y-powerups[i].radioVertical);
            glVertex2f(powerups[i].x+powerups[i].radioHorizontal, powerups[i].y+powerups[i].radioVertical);
            glVertex2f(powerups[i].x+powerups[i].radioHorizontal, powerups[i].y-powerups[i].radioVertical);
            glEnd();

            //PowerUp
            glTranslated(powerups[i].x,powerups[i].y,0);
            glScalef(0.5,0.5,0.5);
            glColor3ub(70,100,130);
            //glutSolidSphere(0.5,20,20);
            if(powerups[i].tipo == UPVIDA) glmDraw(&models[UPVIDA_MOD], GLM_COLOR | GLM_FLAT);
            else if(powerups[i].tipo == UPVELOCIDAD) glmDraw(&models[UPVELOCIDAD_MOD], GLM_COLOR | GLM_FLAT);
            else if(powerups[i].tipo == UPCONDON) glmDraw(&models[UPCONDON_MOD], GLM_COLOR | GLM_FLAT);
            else if(powerups[i].tipo == UPPASTILLA) glmDraw(&models[UPPASTILLA_MOD], GLM_COLOR | GLM_FLAT);
            else if(powerups[i].tipo == UPINYECCION) glmDraw(&models[UPINYECCION_MOD], GLM_COLOR | GLM_FLAT);

            glPopMatrix();
        }

        //Enemigos
        for(int i=0; i<20; i++) {
            // Dibuja al enemigo
            if(enemigos[i].vivo) {
                glPushMatrix();


                //Box
                /*
                glLineWidth(1);
                glBegin(GL_LINES);
                glColor3ub(0, 0, 0);
                glVertex2f(enemigos[i].x-enemigos[i].radioHorizontal, enemigos[i].y+enemigos[i].radioVertical);
                glVertex2f(enemigos[i].x+enemigos[i].radioHorizontal, enemigos[i].y+enemigos[i].radioVertical);
                glVertex2f(enemigos[i].x-enemigos[i].radioHorizontal, enemigos[i].y-enemigos[i].radioVertical);
                glVertex2f(enemigos[i].x+enemigos[i].radioHorizontal, enemigos[i].y-enemigos[i].radioVertical);
                glVertex2f(enemigos[i].x-enemigos[i].radioHorizontal, enemigos[i].y+enemigos[i].radioVertical);
                glVertex2f(enemigos[i].x-enemigos[i].radioHorizontal, enemigos[i].y-enemigos[i].radioVertical);
                glVertex2f(enemigos[i].x+enemigos[i].radioHorizontal, enemigos[i].y+enemigos[i].radioVertical);
                glVertex2f(enemigos[i].x+enemigos[i].radioHorizontal, enemigos[i].y-enemigos[i].radioVertical);
                glEnd();
                */


                //Barra de vida del enemigo
                glColor3ub(1,115,20);
                glRectf(enemigos[i].x-enemigos[i].radioHorizontal,enemigos[i].y+enemigos[i].radioVertical,
                        (enemigos[i].x-enemigos[i].radioHorizontal)+(enemigos[i].vida*(2*enemigos[i].radioHorizontal)/enemigos[i].vidaMax),
                         enemigos[i].y+enemigos[i].radioVertical+0.2);
                glColor3ub(160,38,0);
                glRectf(enemigos[i].x-enemigos[i].radioHorizontal,enemigos[i].y+enemigos[i].radioVertical,
                        enemigos[i].x+enemigos[i].radioHorizontal,enemigos[i].y+enemigos[i].radioVertical+0.2);


                //Enemigo
                glTranslated(enemigos[i].x,enemigos[i].y,0);
                glRotated(90, 0, 1, 0);
                glRotated(90,0,0,1);
                glScalef(1,1,1);
                glColor3ub(70,200,130);
                //glutSolidSphere(0.5,20,20);
                //glmDraw(&models[ESPERMA_MOD], GLM_COLOR | GLM_FLAT);
                glmDraw(&models[ESPERMA_MOD], GLM_COLOR | GLM_SMOOTH);
                glPopMatrix();
            }
        }

        //Jefes
        for(int i=0; i<2; i++) {
            // Dibuja al enemigo
            if(jefes[i].vivo) {
                glPushMatrix();


                glLineWidth(1);
                glBegin(GL_LINES);
                glColor3ub(0, 0, 0);
                glVertex2f(jefes[i].x-jefes[i].radioHorizontal, jefes[i].y+jefes[i].radioVertical);
                glVertex2f(jefes[i].x+jefes[i].radioHorizontal, jefes[i].y+jefes[i].radioVertical);
                glVertex2f(jefes[i].x-jefes[i].radioHorizontal, jefes[i].y-jefes[i].radioVertical);
                glVertex2f(jefes[i].x+jefes[i].radioHorizontal, jefes[i].y-jefes[i].radioVertical);
                glVertex2f(jefes[i].x-jefes[i].radioHorizontal, jefes[i].y+jefes[i].radioVertical);
                glVertex2f(jefes[i].x-jefes[i].radioHorizontal, jefes[i].y-jefes[i].radioVertical);
                glVertex2f(jefes[i].x+jefes[i].radioHorizontal, jefes[i].y+jefes[i].radioVertical);
                glVertex2f(jefes[i].x+jefes[i].radioHorizontal, jefes[i].y-jefes[i].radioVertical);
                glEnd();

                //Barra de vida del enemigo
                glColor3ub(1,115,20);
                glRectf(jefes[i].x-jefes[i].radioHorizontal,jefes[i].y+jefes[i].radioVertical,
                        (jefes[i].x-jefes[i].radioHorizontal)+(jefes[i].vida*(2*jefes[i].radioHorizontal)/jefes[i].vidaMax),
                         jefes[i].y+jefes[i].radioVertical+0.2);
                glColor3ub(160,38,0);
                glRectf(jefes[i].x-jefes[i].radioHorizontal,jefes[i].y+jefes[i].radioVertical,
                        jefes[i].x+jefes[i].radioHorizontal,jefes[i].y+jefes[i].radioVertical+0.2);

                // Dibuja al enemigo
                glTranslated(jefes[i].x,jefes[i].y,0);
                //glRotated(90,0,0,1);

                if(i == 0){
                    glScalef(5,5,5);
                    glColor3ub(90,150,50);
                    glmDraw(&models[JEFE1_MOD], GLM_COLOR | GLM_FLAT);
                }
                else {
                    glScalef(1,1,1);
                    glColor3ub(90,150,50);
                    glutSolidSphere(jefes[i].radioHorizontal,20,20);
                    //glmDraw(&models[JEFE1_MOD], GLM_COLOR | GLM_FLAT);
                }
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
            else
            // Cambia las texturas de la historia
            if(estado == HISTORIA){
                if(textura == 6)
                    estado = INSTRUCCIONES;
                else if(textura > 6)
                    estado = JUEGO;
                textura++;
            }
            else
            //Cambia las instrucciones
            if(estado == INSTRUCCIONES) {
                estado = JUEGO;
            }
            break;
        case 32:
            {
                Arma disparo(globulo.x,globulo.y,globulo.armaActual);
                //Tecla de espacio (disparo)
                if(disparo.tipo == CONDON){
                    disparo.velocidad = 0.5;
                    disparo.damage = 10 + extraDamageCondon;
                    disparo.radioVertical = 0.2;
                    disparo.radioHorizontal = 0.5;
                }
                else if(disparo.tipo == PASTILLA){
                    disparo.velocidad = 0.3;
                    disparo.damage = 15 + extraDamagePastilla;
                    disparo.radioVertical = 0.3;
                    disparo.radioHorizontal = 0.3;
                    disparo.explosionRate = 10;
                }

                if(disparo.tipo == CONDON && !condonShot) {
                    disparos.push_back(disparo);
                    condonShot = true;
                }
                else if(disparo.tipo == PASTILLA && !pastillaShot) {
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

    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/Instrucciones.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);

    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/Pausa.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);

    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/Win.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);

    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/GameOver.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);

    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/Juego.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);

    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/Historia.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);

    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/TipNivel1.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);

    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/TipNivel2.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);

    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/TipNivel3.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);

    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/TipNivel4.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);

    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/TipNivel5.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);

    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/TipNivel6.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);

    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/TipNivel7.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);

    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/TipNivel8.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);

    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/TipNivel9.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);

    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/TipNivel10.bmp");
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
    std::string ruta = fullPath + "objects/Globulo.obj";
    models[PLAYER_MOD] = *glmReadOBJ(ruta.c_str());
    glmUnitize(&models[PLAYER_MOD]);
    glmVertexNormals(&models[PLAYER_MOD], 90.0, GL_TRUE);

    //espermatozoides
    ruta = fullPath + "objects/esperma.obj";
    models[ESPERMA_MOD] = *glmReadOBJ(ruta.c_str());
    glmUnitize(&models[ESPERMA_MOD]);
    glmVertexNormals(&models[ESPERMA_MOD], 90.0, GL_TRUE);

    //Jefe 1
    ruta = fullPath + "objects/papiloma.obj";
    models[JEFE1_MOD] = *glmReadOBJ(ruta.c_str());
    glmUnitize(&models[JEFE1_MOD]);
    glmVertexNormals(&models[JEFE1_MOD], 90.0, GL_TRUE);

    //condon
    ruta = fullPath + "objects/condon.obj";
    models[CONDON_MOD] = *glmReadOBJ(ruta.c_str());
    glmUnitize(&models[CONDON_MOD]);
    glmVertexNormals(&models[CONDON_MOD], 90.0, GL_TRUE);

    //pastilla
    ruta = fullPath + "objects/pastilla.obj";
    models[PASTILLA_MOD] = *glmReadOBJ(ruta.c_str());
    glmUnitize(&models[PASTILLA_MOD]);
    glmVertexNormals(&models[PASTILLA_MOD], 90.0, GL_TRUE);

    //power up de vida
    ruta = fullPath + "objects/cruz.obj";
    models[UPVIDA_MOD] = *glmReadOBJ(ruta.c_str());
    glmUnitize(&models[UPVIDA_MOD]);
    glmVertexNormals(&models[UPVIDA_MOD], 90.0, GL_TRUE);

    //power up de velocidad
    ruta = fullPath + "objects/rayito.obj";
    models[UPVELOCIDAD_MOD] = *glmReadOBJ(ruta.c_str());
    glmUnitize(&models[UPVELOCIDAD_MOD]);
    glmVertexNormals(&models[UPVELOCIDAD_MOD], 90.0, GL_TRUE);

    //power up de condon
    ruta = fullPath + "objects/PowerCondon.obj";
    models[UPCONDON_MOD] = *glmReadOBJ(ruta.c_str());
    glmUnitize(&models[UPCONDON_MOD]);
    glmVertexNormals(&models[UPCONDON_MOD], 90.0, GL_TRUE);

    //power up de pastillas
    ruta = fullPath + "objects/powerinyeccion.obj";
    models[UPPASTILLA_MOD] = *glmReadOBJ(ruta.c_str());
    glmUnitize(&models[UPPASTILLA_MOD]);
    glmVertexNormals(&models[UPPASTILLA_MOD], 90.0, GL_TRUE);

    //power up de inyecciones
    ruta = fullPath + "objects/powerinyeccion.obj";
    models[UPINYECCION_MOD] = *glmReadOBJ(ruta.c_str());
    glmUnitize(&models[UPINYECCION_MOD]);
    glmVertexNormals(&models[UPINYECCION_MOD], 90.0, GL_TRUE);

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

    //Inicializacion de power ups
    powerups[0].tipo = UPVIDA;
    powerups[1].tipo = UPVIDA;
    powerups[2].tipo = UPVELOCIDAD;
    powerups[3].tipo = UPCONDON;
    powerups[4].tipo = UPPASTILLA;
    powerups[5].tipo = UPINYECCION;
    for(int i=0; i<6; i++){
        regeneraPowerUps(i);
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
    gluPerspective(45.0, (float)w / (float)h, 1.0, 60.0);
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
    glutTimerFunc(200,myTimer,0); //Timer

    glutMainLoop();
}
