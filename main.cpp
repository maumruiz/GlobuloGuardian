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
#define ESPERMA_COUNT 2

#define ESPERMA_MOD1 0
#define ESPERMA_MOD2 1

#define PLAYER_MOD 0
#define ESPERMA_MOD 1
#define JEFE1_MOD 2
#define JEFE2_MOD 3
#define CONDON_MOD 4
#define PASTILLA_MOD 5
#define INYECCION_MOD 6
#define UPVIDA_MOD 7
#define UPVELOCIDAD_MOD 8
#define UPCONDON_MOD 9
#define UPPASTILLA_MOD 10
#define UPINYECCION_MOD 11

//Estados del juego
#define MENU 0
#define INSTRUCCIONES 1
#define CREDITOS 2
#define PAUSA 3
#define WIN 4
#define GAMEOVER 5
#define JUEGO 6
#define HISTORIA 7

//Armas
#define CONDON 1
#define PASTILLA 2
#define INYECCION 3

//Power Ups
#define UPVIDA 1
#define UPVELOCIDAD 2
#define UPCONDON 3
#define UPPASTILLA 4
#define UPINYECCION 5

using namespace std;

// Variables generales del juego
int estado = MENU; // 0-Menu 1-Historia 2-Juego 3-Pausa
int nivel = 1; //Nivel del juego
int puntaje = 0;

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
int anguloPowerUp = 0;

// Variables para jefes
int movFluido = 20;
int contMovFluido = 0;

//Variables para disparos
int condonesFire = 0;
int pastillasFire = 0;
int inyeccionesFire = 0;
int condonesFireRate = 7;
int pastillasFireRate = 12;
int inyeccionesFireRate = 5;
bool condonShot = false;
bool pastillaShot = false;
bool inyeccionShot = false;
int extraDamageCondon = 0;
int extraDamagePastilla = 0;
int extraDamageInyeccion = 0;

//Variables para la camara
bool camaraFP = false;
bool moviendoCamara = false;
float lookAtX = 0;
float lookAtZ = 15;
float lookAtUpX = 0;
float lookAtUpY = 1;

//Variables para animacion de espermas
int espermaAnimacionModMax = 1;
int espermaAnimacion = 0;
int espermaAnimacionTimer = 4;
int espermaAnimacionCounter = 1;

//Variables para iluminacion
/*
GLfloat light_ambient[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
*/
GLfloat light_position[] = { 0.0, -8.0, 5.0, 0.0 };


float ambiente[][4]={
  {0.19225, 0.19225, 0.19225, 1.0}//plata
};

float difuso[][4]={
  {0.50754, 0.50754, 0.50754, 1.0},//plata
  {0.098, 0.098, 0.439,1},
  {0-117, 0.5647,1,1},
  {0, 0, 0, 1}
};

float especular[][4]={
  {0.508273, 0.508273, 0.508273,1.0}//plata
};

float brillo[]={
  0.4f
};

// Variables para texturas
//__FILE__ is a preprocessor macro that expands to full path to the current file.
string fullPath = __FILE__;
int textura=HISTORIA;
static GLuint texName[36]; //0-Menu 1-Instrucciones 2-Pausa 3-Win 4-Gameover 5..-Historia
const int TEXTURE_COUNT=17;
string rutaMusica;

// Variables para modelos
GLMmodel models[MODEL_COUNT];
GLMmodel espermas[ESPERMA_COUNT];

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
/////////////    Iluminacion de materiales    /////////////////////
///////////////////////////////////////////////////////////////////
void materialLightning(int i)
{
  //Asigna los apropiados materiales a las superficies
  glMaterialfv(GL_FRONT,GL_AMBIENT,ambiente[i]);
  glMaterialfv(GL_FRONT,GL_DIFFUSE,difuso[i]);
  glMaterialfv(GL_FRONT,GL_SPECULAR,especular[i]);
  glMaterialf(GL_FRONT,GL_SHININESS,brillo[i]*128.0);
  // asigna la apropiada fuente de luz
  GLfloat lightIntensity[] = {1.0f, 1.0f, 1.0f, 1.0f};
  GLfloat light_position[] = { 0.0, -8.0, 5.0, 0.0 };
  glLightfv(GL_LIGHT0, GL_POSITION,light_position);
  glLightfv(GL_LIGHT0, GL_DIFFUSE,lightIntensity);
  //asigna la c√°mara
  //comienza el dibujo

}

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
    enemigos[i].y = (rand() % 90 - 50) / 10.0;
    enemigos[i].vida = enemigos[i].vidaMax;
}

///////////////////////////////////////////////////////////////////
///////    Regenera Power Ups    //////////////////////////////////
///////////////////////////////////////////////////////////////////
void regeneraPowerUps(int i)
{
    powerups[i].x = rand()% 300 + 80;
    powerups[i].y = rand() % (90 - 50) / 10.0;
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
///////   Funcion para reiniciar el juego    //////////////////////
////// Vuelve a inicializar los valores por default ///////////////
void reiniciaJuego()
{
    nivel = 1; //Nivel del juego
    puntaje = 0;

    // Variables del juego
    tiempoJuego = 1200;
    enemigosActuales = 2;
    anguloPowerUp = 0;

    // Variables para jefes
    contMovFluido = 0;

    //Variables para disparos
    condonesFire = 0;
    pastillasFire = 0;
    inyeccionesFire = 0;
    condonShot = false;
    pastillaShot = false;
    inyeccionShot = false;
    extraDamageCondon = 0;
    extraDamagePastilla = 0;
    extraDamageInyeccion = 0;

    //Variables para la camara
    camaraFP = false;
    moviendoCamara = false;
    lookAtX = 0;
    lookAtZ = 15;
    lookAtUpX = 0;
    lookAtUpY = 1;

    textura=HISTORIA;

    //Inicializacion del personaje principal
    globulo = {-10, 0, 100, 0.2, 1, 0.7, 0.7, false,false,false,false};
    //Inicializacion de los enemigos
    Enemigo auxEnemigo;
    for (int i=0; i<20; i++) {
        enemigos[i] = auxEnemigo;
    }
    for(int i=0; i<2; i++){
        enemigos[i].vivo = true;
        enemigos[i].x = rand()% ((i+1)*10) + 13;
        enemigos[i].y = (rand() % 90 - 50) / 10.0;
    }

    // Inicializacion de los jefes
    Jefe auxJefe;
    for (int i=0; i<2; i++) {
        jefes[i] = auxJefe;
    }

    // Inicializacion de las armas
    disparos.clear();

    //Inicializacion de PowerUps
    for(int i=0; i<6; i++){
        regeneraPowerUps(i);
    }
}

///////////////////////////////////////////////////////////////////
/////////////////    Funcion timer    /////////////////////////////
///////////////////////////////////////////////////////////////////
void myTimer(int i) {

    if(estado == JUEGO) {

        // Movimiento de camara
        if(moviendoCamara) {
            if(camaraFP) {
                if(lookAtX > -16) lookAtX -= 0.5; else lookAtX = -16;
                if(lookAtZ > 2) lookAtZ -= 0.5; else lookAtZ = 2;
                if(lookAtUpX < 1) lookAtUpX += 0.1; else lookAtUpX = 1;
                if(lookAtUpY > 0) lookAtUpY -= 0.1; else lookAtUpY = 0;
                if(lookAtX == -16 && lookAtZ == 2 && lookAtUpX == 1 && lookAtUpY == 0)
                    moviendoCamara = false;
            }
            else {
                if(lookAtX < 0) lookAtX += 0.5; else lookAtX = 0;
                if(lookAtZ < 15) lookAtZ += 0.5; else lookAtZ = 15;
                if(lookAtUpX > 0) lookAtUpX -= 0.1; else lookAtUpX = 0;
                if(lookAtUpY < 1) lookAtUpY += 0.1; else lookAtUpY = 1;
                if(lookAtX == 0 && lookAtZ == 15 && lookAtUpX == 0 && lookAtUpY == 1)
                    moviendoCamara = false;
            }
        }

        //Dificultad de niveles
        if(tiempoJuego > 0){
            if(nivel != 5 && nivel != 10)
                tiempoJuego--;
        }
        else {
            rutaMusica = fullPath + "Sounds/Menu.wav";
            PlaySound(TEXT(rutaMusica.c_str()),NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
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
            disparos.clear();
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
                    if(checaColision(*(it),jefes[i]) && !it->explosion) {
                        jefes[i].vida -= it->damage;
                        // Si se mato al jefe cambia el nivel
                        if(jefes[i].vida <= 0){
                            puntaje += jefes[i].vidaMax;
                            jefes[i].vivo = false;
                            nivel++;
                            //Si era el primer jefe cambia a historia y aumenta enemigos
                            if(i == 0) {
                                estado = HISTORIA;
                                enemigosActuales += 2;
                                if(enemigosActuales > 20) enemigosActuales = 20;
                                //Los enemigos se vuelven mas resistentes o mas rapidos
                                for(int i=enemigosActuales-2; i<enemigosActuales; i++){
                                    enemigos[i].vivo = true;
                                    if(nivel%2 == 0)
                                        enemigos[i].vidaMax += (nivel*vidaIncr);
                                    else
                                        enemigos[i].velocidad += (nivel*velIncr);
                                }
                                // Regenera enemigos, power ups y disparos
                                for(int i=0; i<enemigosActuales; i++)
                                    regeneraEnemigo(i);
                                for(int i=0; i<6; i++)
                                    regeneraPowerUps(i);
                                // disparos.clear();
                                rutaMusica = fullPath + "Sounds/Menu.wav";
                                PlaySound(TEXT(rutaMusica.c_str()),NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
                            }

                            //Si era el ultimo jefe, se gana el juego
                            if(i == 1){
                                estado = WIN;
                                rutaMusica = fullPath + "Sounds/Win.wav";
                                PlaySound(TEXT(rutaMusica.c_str()),NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
                            }
                        }

                        // Si el arma no era pastilla se elimina el disparo
                        if(it->tipo != PASTILLA)
                            it = disparos.erase(it);
                        // Si el arma era una pastilla hace explosion y le pega a los demas
                        else if(it->tipo == PASTILLA){
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
                    if(globulo.vida <= 0) {
                        globulo.vida = 0;
                        estado = GAMEOVER;
                        rutaMusica = fullPath + "Sounds/GameOver.wav";
                        PlaySound(TEXT(rutaMusica.c_str()),NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
                    }
                    regeneraEnemigo(i);
                }

                //Checa colision con las armas
                for (vector<Arma>::iterator it = disparos.begin() ; it != disparos.end();) {
                    if(checaColision(*(it),enemigos[i]) && !it->explosion) {
                        enemigos[i].vida -= it->damage;
                        if(enemigos[i].vida <= 0){
                            puntaje += ((i+2)/2) * 5;
                            regeneraEnemigo(i);
                        }
                        if(it->tipo != PASTILLA)
                            it = disparos.erase(it);
                        else if(it->tipo == PASTILLA){
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
        anguloPowerUp = (anguloPowerUp + 5) % 360;

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

        // Actualiza animaciones
        if(espermaAnimacionCounter < espermaAnimacionTimer){
            espermaAnimacionCounter++;
        }
        else {
            if(espermaAnimacion < espermaAnimacionModMax)
                espermaAnimacion++;
            else
                espermaAnimacion = 0;

            espermaAnimacionCounter = 0;
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

        if(inyeccionShot) {
            if(inyeccionesFire<inyeccionesFireRate)
                inyeccionesFire++;
            else{
                inyeccionesFire = 0;
                inyeccionShot = false;
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
    glColor3ub(255,255,255); //R: 243 G: 138 B: 168

    glPushMatrix();
    gluLookAt(lookAtX, 0, lookAtZ, 0, 0, 0, lookAtUpX, lookAtUpY, 1);

    //Habilitar la luz
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    //Habilitar el uso de texturas
    glEnable(GL_TEXTURE_2D);

    //Menu e Historia
    if(estado == MENU || estado == HISTORIA || estado == INSTRUCCIONES ||
       estado == PAUSA || estado == WIN || estado == GAMEOVER || estado == CREDITOS) {

        //Elegir la textura del Quads:
        if(estado == MENU)
            glBindTexture(GL_TEXTURE_2D, texName[MENU]);

        if(estado == INSTRUCCIONES)
            glBindTexture(GL_TEXTURE_2D, texName[INSTRUCCIONES]);

        if(estado == CREDITOS)
            glBindTexture(GL_TEXTURE_2D, texName[CREDITOS]);

        if(estado == PAUSA)
            glBindTexture(GL_TEXTURE_2D, texName[PAUSA]);

        if(estado == WIN)
            glBindTexture(GL_TEXTURE_2D, texName[WIN]);

        if(estado == GAMEOVER)
            glBindTexture(GL_TEXTURE_2D, texName[GAMEOVER]);

        if(estado == HISTORIA)
            glBindTexture(GL_TEXTURE_2D, texName[textura]);

        if(!camaraFP) {
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
        else {
            glBegin(GL_QUADS);
            //Asignar la coordenada de textura 0,0 al vertice
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(-2, 12.5f, -6.5f);
             //Asignar la coordenada de textura 1,0 al vertice
            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(-2, -12.5f, -6.5f);

             //Asignar la coordenada de textura 1,1 al vertice
            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(-1, -12.5f, 6.5f);
             //Asignar la coordenada de textura 0,1 al vertice
            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(-1, 12.5f, 6.5f);

            glEnd();
        }

    }
    else
    // Juego
    if(estado == JUEGO) {

        if(camaraFP || moviendoCamara) {
            // Suelo
            //Elegir la textura del Quads:
            glBindTexture(GL_TEXTURE_2D, texName[JUEGO]);
            glBegin(GL_QUADS);
                //Asignar la coordenada de textura 0,0 al vertice
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(-20.0f, -20.0f, -5);
                //Asignar la coordenada de textura 1,0 al vertice
            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(100.0f, -20.0f, -5);
                //Asignar la coordenada de textura 1,1 al vertice
            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(60.0f, 20.0, -5);
                //Asignar la coordenada de textura 0,1 al vertice
            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(-60.0f, 20.0f, -5);
            glEnd();
        }
        else if(!camaraFP && !moviendoCamara) {
            // Suelo
            //Elegir la textura del Quads:
            glBindTexture(GL_TEXTURE_2D, texName[JUEGO]);
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
        }

            // Techo
            //Elegir la textura del Quads:
            glBindTexture(GL_TEXTURE_2D, texName[JUEGO]);
            glBegin(GL_QUADS);
                //Asignar la coordenada de textura 0,0 al vertice
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(-20.0f, -20.0f, 15);
                //Asignar la coordenada de textura 1,0 al vertice
            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(100.0f, -20.0f, 15);
                //Asignar la coordenada de textura 1,1 al vertice
            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(60.0f, 20.0, 15);
                //Asignar la coordenada de textura 0,1 al vertice
            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(-60.0f, 20.0f, 15);
            glEnd();

            //Pared Izquierda
            glBindTexture(GL_TEXTURE_2D, texName[JUEGO]);
            glBegin(GL_QUADS);
                //Asignar la coordenada de textura 0,0 al vertice
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(-20.0f, 20.0f, -5);
                //Asignar la coordenada de textura 1,0 al vertice
            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(100.0f, 20.0f, -5);
                //Asignar la coordenada de textura 1,1 al vertice
            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(100.0f, 20.0, 15);
                //Asignar la coordenada de textura 0,1 al vertice
            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(-20.0f, 20.0f, 15);
            glEnd();

            //Pared Derecha
            glBindTexture(GL_TEXTURE_2D, texName[JUEGO]);
            glBegin(GL_QUADS);
                //Asignar la coordenada de textura 0,0 al vertice
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(-20.0f, -20.0f, -5);
                //Asignar la coordenada de textura 1,0 al vertice
            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(100.0f, -20.0f, -5);
                //Asignar la coordenada de textura 1,1 al vertice
            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(100.0f, -20.0, 15);
                //Asignar la coordenada de textura 0,1 al vertice
            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(-20.0f, -20.0f, 15);
            glEnd();

        // Barra de vida
        if(camaraFP) {
            glBegin(GL_QUADS);
            glColor3ub(1,115,20);
            glVertex3f(0, 12, 5);
            glVertex3f(0, 12-(globulo.vida/10), 5);
            glVertex3f(0, 12-(globulo.vida/10), 6);
            glVertex3f(0, 12, 6);
            glEnd();
            glBegin(GL_QUADS);
            glColor3ub(160,38,0);
            glVertex3f(0, 12, 5);
            glVertex3f(0, 2, 5);
            glVertex3f(0, 2, 6);
            glVertex3f(0, 12, 6);
            glEnd();
        }
        else {
            glBegin(GL_QUADS);
            glColor3ub(1,115,20);
            glVertex3f(-12, 5, 0);
            glVertex3f(-12+(globulo.vida/10), 5, 0);
            glVertex3f(-12+(globulo.vida/10), 6, 0);
            glVertex3f(-12, 6, 0);
            glEnd();
            glBegin(GL_QUADS);
            glColor3ub(160,38,0);
            glVertex3f(-12, 5, 0);
            glVertex3f(-2, 5, 0);
            glVertex3f(-2, 6, 0);
            glVertex3f(-12, 6, 0);
            glEnd();
        }

        // Nivel
        sprintf(msg, "%s%d", "Nivel ", nivel);
        glColor3ub(0,0,0);
        if(camaraFP) glRasterPos3d(0,0,5.5); else glRasterPos2d(0,5.5);
        for(int k=0;msg[k]!='\0'; k++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, msg[k]);

        //Tiempo
        sprintf(msg, "%d", tiempoJuego/20);
        glColor3ub(0,0,0);
        if(camaraFP) glRasterPos3d(0, -0.5, 5); else glRasterPos2d(0.5, 5);
        for(int k=0;msg[k]!='\0'; k++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, msg[k]);

        //Puntaje
        sprintf(msg, "%s%d", "Puntaje: " ,puntaje);
        glColor3ub(0,0,0);
        if(camaraFP) glRasterPos3d(0, -5, 5.25); else glRasterPos2d(5,5.25);
        for(int k=0;msg[k]!='\0'; k++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, msg[k]);

        //Globulo
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        //materialLightning(0);
        glPushMatrix();
        glTranslated(globulo.x,globulo.y,0);
        glRotated(90, 1, 0, 0);
        glRotated(90, 0, 1, 0);
        glScalef(0.7,0.7,0.7);
        glColor3ub(100,0,0);
        glmDraw(&models[PLAYER_MOD], GLM_COLOR | GLM_SMOOTH);
        //Dibuja el arma actual del globulo
        glPushMatrix();
        if(globulo.armaActual == CONDON) {
            glTranslated(0,0,1);
            glRotated(90, 1, 0, 0);
            glScalef(0.5,0.5,0.5);
            glmDraw(&models[CONDON_MOD], GLM_COLOR | GLM_SMOOTH);
        }
        else if(globulo.armaActual == PASTILLA) {
            glTranslated(0,0,1);
            glScalef(0.5,0.5,0.5);
            glmDraw(&models[PASTILLA_MOD], GLM_COLOR | GLM_SMOOTH);
        }
        else if(globulo.armaActual == INYECCION) {
            glTranslated(0,0,1.5);
            glRotated(90, 1, 0, 0);
            //glScalef(0.5,0.5,0.5);
            glmDraw(&models[INYECCION_MOD], GLM_COLOR | GLM_SMOOTH);
        }

        glPopMatrix();
        glPopMatrix();
        glDisable(GL_LIGHT0);
        glDisable(GL_LIGHTING);

        // Disparos
        for (vector<Arma>::iterator it = disparos.begin() ; it != disparos.end(); ++it){
            // Condones
            if(it->tipo == CONDON){
                glEnable(GL_LIGHTING);
                glEnable(GL_LIGHT0);
                glPushMatrix();
                glTranslated(it->x,it->y,0);
                glRotated(-90, 0, 0, 1);
                glScalef(0.5,0.5,0.5);
                glColor3ub(0,0,255);
                //glutSolidSphere(0.1,20,20);
                glmDraw(&models[CONDON_MOD], GLM_COLOR | GLM_SMOOTH);
                glPopMatrix();
                glDisable(GL_LIGHT0);
                glDisable(GL_LIGHTING);
            }

            //Pastillas
            if(it->tipo == PASTILLA){
                glEnable(GL_LIGHTING);
                glEnable(GL_LIGHT0);
                glPushMatrix();
                glTranslated(it->x,it->y,0);
                glRotated(90, 1, 0, 0);
                glScalef(0.5+it->contExplosion/10.0,0.5+it->contExplosion/10.0,0.5+it->contExplosion/10.0);
                glColor3ub(0,255,255);
                //glutSolidSphere(0.2+it->contExplosion/10.0,20,20);
                glmDraw(&models[PASTILLA_MOD], GLM_COLOR | GLM_SMOOTH);
                glPopMatrix();
                glDisable(GL_LIGHT0);
                glDisable(GL_LIGHTING);
            }

            //Inyecciones
            if(it->tipo == INYECCION){
                glEnable(GL_LIGHTING);
                glEnable(GL_LIGHT0);
                glPushMatrix();
                glTranslated(it->x,it->y,0);
                glRotated(-90, 0, 0, 1);
                glScalef(1,1,1);
                glColor3ub(0,255,255);
                //glutSolidSphere(0.2+it->contExplosion/10.0,20,20);
                glmDraw(&models[INYECCION_MOD], GLM_COLOR | GLM_SMOOTH);
                glPopMatrix();
                glDisable(GL_LIGHT0);
                glDisable(GL_LIGHTING);
            }

            //Box
            /*
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
            */

        }

        //PowerUps
        for(int i=0; i<6; i++) {
            // Dibuja el power up
            glPushMatrix();
            //Box
            /*
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
            */

            //PowerUp
            glEnable(GL_LIGHTING);
            glEnable(GL_LIGHT0);
            glTranslated(powerups[i].x,powerups[i].y,0);
            glRotated(anguloPowerUp, 0, 1, 0);
            glScalef(0.5,0.5,0.5);
            glColor3ub(70,100,130);
            //glutSolidSphere(0.5,20,20);
            if(powerups[i].tipo == UPVIDA) glmDraw(&models[UPVIDA_MOD], GLM_COLOR | GLM_SMOOTH);
            else if(powerups[i].tipo == UPVELOCIDAD) glmDraw(&models[UPVELOCIDAD_MOD], GLM_COLOR | GLM_SMOOTH);
            else if(powerups[i].tipo == UPCONDON) glmDraw(&models[UPCONDON_MOD], GLM_COLOR | GLM_SMOOTH);
            else if(powerups[i].tipo == UPPASTILLA) glmDraw(&models[UPPASTILLA_MOD], GLM_COLOR | GLM_SMOOTH);
            else if(powerups[i].tipo == UPINYECCION) glmDraw(&models[UPINYECCION_MOD], GLM_COLOR | GLM_SMOOTH);

            glPopMatrix();
            glDisable(GL_LIGHT0);
            glDisable(GL_LIGHTING);
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

                // Barra de vida del enemigo
                if(camaraFP) {
                    glBegin(GL_QUADS);
                    glColor3ub(1,115,20);
                    glVertex3f(enemigos[i].x,enemigos[i].y+enemigos[i].radioHorizontal,enemigos[i].radioVertical);
                    glVertex3f(enemigos[i].x, (enemigos[i].y+enemigos[i].radioHorizontal)
                               -(enemigos[i].vida*(2*enemigos[i].radioHorizontal)/enemigos[i].vidaMax),
                               enemigos[i].radioVertical);
                    glVertex3f(enemigos[i].x,(enemigos[i].y+enemigos[i].radioHorizontal)
                               -(enemigos[i].vida*(2*enemigos[i].radioHorizontal)/enemigos[i].vidaMax),
                                enemigos[i].radioVertical+0.2);
                    glVertex3f(enemigos[i].x,enemigos[i].y+enemigos[i].radioHorizontal, enemigos[i].radioVertical+0.2);
                    glEnd();

                    glBegin(GL_QUADS);
                    glColor3ub(160,38,0);
                    glVertex3f(enemigos[i].x,enemigos[i].y+enemigos[i].radioHorizontal,enemigos[i].radioVertical);
                    glVertex3f(enemigos[i].x,enemigos[i].y-enemigos[i].radioHorizontal,enemigos[i].radioVertical);
                    glVertex3f(enemigos[i].x,enemigos[i].y-enemigos[i].radioHorizontal,enemigos[i].radioVertical+0.2);
                    glVertex3f(enemigos[i].x,enemigos[i].y+enemigos[i].radioHorizontal,enemigos[i].radioVertical+0.2);
                    glEnd();
                }
                else {
                    glBegin(GL_QUADS);
                    glColor3ub(1,115,20);
                    glVertex3f(enemigos[i].x-enemigos[i].radioHorizontal,enemigos[i].y+enemigos[i].radioVertical, 0);
                    glVertex3f((enemigos[i].x-enemigos[i].radioHorizontal)+(enemigos[i].vida*(2*enemigos[i].radioHorizontal)/enemigos[i].vidaMax),
                               enemigos[i].y+enemigos[i].radioVertical, 0);
                    glVertex3f((enemigos[i].x-enemigos[i].radioHorizontal)+(enemigos[i].vida*(2*enemigos[i].radioHorizontal)/enemigos[i].vidaMax),
                                enemigos[i].y+enemigos[i].radioVertical+0.2, 0);
                    glVertex3f(enemigos[i].x-enemigos[i].radioHorizontal, enemigos[i].y+enemigos[i].radioVertical+0.2, 0);
                    glEnd();
                    glBegin(GL_QUADS);
                    glColor3ub(160,38,0);
                    glVertex3f(enemigos[i].x-enemigos[i].radioHorizontal,enemigos[i].y+enemigos[i].radioVertical, 0);
                    glVertex3f(enemigos[i].x+enemigos[i].radioHorizontal,enemigos[i].y+enemigos[i].radioVertical, 0);
                    glVertex3f(enemigos[i].x+enemigos[i].radioHorizontal,enemigos[i].y+enemigos[i].radioVertical+0.2, 0);
                    glVertex3f(enemigos[i].x-enemigos[i].radioHorizontal,enemigos[i].y+enemigos[i].radioVertical+0.2, 0);
                    glEnd();
                }

                //Enemigo
                glEnable(GL_LIGHTING);
                glEnable(GL_LIGHT0);
                glTranslated(enemigos[i].x,enemigos[i].y,0);
                glRotated(90, 0, 1, 0);
                glRotated(90,0,0,1);
                glScalef(1,1,1);
                glColor3ub(70,200,130);
                //glutSolidSphere(0.5,20,20);
                //glmDraw(&models[ESPERMA_MOD], GLM_COLOR | GLM_FLAT);
                glmDraw(&espermas[espermaAnimacion], GLM_COLOR | GLM_SMOOTH);
                glPopMatrix();
                glDisable(GL_LIGHT0);
                glDisable(GL_LIGHTING);
            }
        }

        //Jefes
        for(int i=0; i<2; i++) {
            // Dibuja al enemigo
            if(jefes[i].vivo) {
                glPushMatrix();

                /*
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
                */

                // Barra de vida del jefe
                if(camaraFP) {
                    glBegin(GL_QUADS);
                    glColor3ub(1,115,20);
                    glVertex3f(jefes[i].x,jefes[i].y+jefes[i].radioHorizontal,jefes[i].radioVertical);
                    glVertex3f(jefes[i].x, (jefes[i].y+jefes[i].radioHorizontal)
                               -(jefes[i].vida*(2*jefes[i].radioHorizontal)/jefes[i].vidaMax),
                               jefes[i].radioVertical);
                    glVertex3f(jefes[i].x,(jefes[i].y+jefes[i].radioHorizontal)
                               -(jefes[i].vida*(2*jefes[i].radioHorizontal)/jefes[i].vidaMax),
                                jefes[i].radioVertical+0.2);
                    glVertex3f(jefes[i].x,jefes[i].y+jefes[i].radioHorizontal, jefes[i].radioVertical+0.2);
                    glEnd();

                    glBegin(GL_QUADS);
                    glColor3ub(160,38,0);
                    glVertex3f(jefes[i].x,jefes[i].y+jefes[i].radioHorizontal,jefes[i].radioVertical);
                    glVertex3f(jefes[i].x,jefes[i].y-jefes[i].radioHorizontal,jefes[i].radioVertical);
                    glVertex3f(jefes[i].x,jefes[i].y-jefes[i].radioHorizontal,jefes[i].radioVertical+0.2);
                    glVertex3f(jefes[i].x,jefes[i].y+jefes[i].radioHorizontal,jefes[i].radioVertical+0.2);
                    glEnd();
                }
                else {
                    glBegin(GL_QUADS);
                    glColor3ub(1,115,20);
                    glVertex3f(jefes[i].x-jefes[i].radioHorizontal,jefes[i].y+jefes[i].radioVertical, 0);
                    glVertex3f((jefes[i].x-jefes[i].radioHorizontal)+(jefes[i].vida*(2*jefes[i].radioHorizontal)/jefes[i].vidaMax),
                               jefes[i].y+jefes[i].radioVertical, 0);
                    glVertex3f((jefes[i].x-jefes[i].radioHorizontal)+(jefes[i].vida*(2*jefes[i].radioHorizontal)/jefes[i].vidaMax),
                                jefes[i].y+jefes[i].radioVertical+0.2, 0);
                    glVertex3f(jefes[i].x-jefes[i].radioHorizontal, jefes[i].y+jefes[i].radioVertical+0.2, 0);
                    glEnd();
                    glBegin(GL_QUADS);
                    glColor3ub(160,38,0);
                    glVertex3f(jefes[i].x-jefes[i].radioHorizontal,jefes[i].y+jefes[i].radioVertical, 0);
                    glVertex3f(jefes[i].x+jefes[i].radioHorizontal,jefes[i].y+jefes[i].radioVertical, 0);
                    glVertex3f(jefes[i].x+jefes[i].radioHorizontal,jefes[i].y+jefes[i].radioVertical+0.2, 0);
                    glVertex3f(jefes[i].x-jefes[i].radioHorizontal,jefes[i].y+jefes[i].radioVertical+0.2, 0);
                    glEnd();
                }

                // Dibuja al enemigo
                glEnable(GL_LIGHTING);
                glEnable(GL_LIGHT0);
                glTranslated(jefes[i].x,jefes[i].y,0);
                //glRotated(90,0,0,1);

                if(i == 0){
                    glScalef(1.5,1.5,1.5);
                    glColor3ub(90,150,50);
                    glmDraw(&models[JEFE1_MOD], GLM_COLOR | GLM_SMOOTH);
                }
                else {
                    glRotated(-90, 0, 1, 0);
                    glScalef(2,2,2);
                    glColor3ub(90,150,50);
                    //glutSolidSphere(jefes[i].radioHorizontal,20,20);
                    glmDraw(&models[JEFE2_MOD], GLM_COLOR | GLM_SMOOTH);
                }
                glPopMatrix();
                glDisable(GL_LIGHT0);
                glDisable(GL_LIGHTING);


            }
        }
    }
    glPopMatrix();
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
            if(!camaraFP)
                globulo.movArriba = true;
            else
                globulo.movDer = true;
            break;
        case 's':
        case 'S':
            if(!camaraFP)
                globulo.movAbajo = true;
            else
                globulo.movIzq = true;
            break;
        case 'a':
        case 'A':
            if(!camaraFP)
                globulo.movIzq = true;
            else
                globulo.movArriba = true;
            break;
        case 'd':
        case 'D':
            if(!camaraFP)
                globulo.movDer = true;
            else
                globulo.movAbajo = true;
            break;
        case 'i':
        case 'I':
            if(estado == MENU)
                estado = INSTRUCCIONES;
            break;
        case 'c':
        case 'C':
            if(estado == MENU)
                estado = CREDITOS;
            break;
        case 'r':
        case 'R':
            // regresa desde las instrucciones, win o creditos
            if(estado == INSTRUCCIONES || estado == CREDITOS)
                estado = MENU;
            else if(estado == WIN || estado == GAMEOVER) {
                estado = MENU;
                reiniciaJuego();
            }
            break;
        case 'p':
        case 'P':
            // Pausa el juego
            if(estado == JUEGO)
                estado = PAUSA;
            else if(estado == PAUSA)
                estado = JUEGO;
            break;
        case 'z':
        case 'Z':
            // Cambia la camara en el juego
            if(estado == JUEGO) {
                camaraFP = !camaraFP;
                moviendoCamara = true;
            }
            break;
        case '1':
            globulo.armaActual = CONDON;
            break;
        case '2':
            globulo.armaActual = PASTILLA;
            break;
        case '3':
            globulo.armaActual = INYECCION;
            break;
        case 13:
            //Cambia del menu a la historia
            if(estado == MENU)
                estado = HISTORIA;
            else
            // Cambia las texturas de la historia
            if(estado == HISTORIA){
                if(textura > HISTORIA){
                    estado = JUEGO;
                    rutaMusica = fullPath + "Sounds/Game.wav";
                    PlaySound(TEXT(rutaMusica.c_str()),NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
                }
                textura++;
            }
            else
            //Cambia las instrucciones
            if(estado == INSTRUCCIONES) {
                estado = JUEGO;
            }
            break;
        case 32:
            {   if(estado == JUEGO) {
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
                else if(disparo.tipo == INYECCION){
                    disparo.velocidad = 0.4;
                    disparo.damage = 10 + extraDamageInyeccion;
                    disparo.radioVertical = 0.3;
                    disparo.radioHorizontal = 0.3;
                }

                if(disparo.tipo == CONDON && !condonShot) {
                    disparos.push_back(disparo);
                    condonShot = true;
                }
                else if(disparo.tipo == PASTILLA && !pastillaShot) {
                    disparos.push_back(disparo);
                    pastillaShot = true;
                }
                else if(disparo.tipo == INYECCION && !inyeccionShot) {
                    disparos.push_back(disparo);
                    inyeccionShot = true;
                }
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
            if(!camaraFP)
                globulo.movArriba = false;
            else
                globulo.movDer = false;
            break;
        case 's':
        case 'S':
            if(!camaraFP)
                globulo.movAbajo = false;
            else
                globulo.movIzq = false;
            break;
        case 'a':
        case 'A':
            if(!camaraFP)
                globulo.movIzq = false;
            else
                globulo.movArriba = false;
            break;
        case 'd':
        case 'D':
            if(!camaraFP)
                globulo.movDer = false;
            else
                globulo.movAbajo = false;
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    //Filtros de ampliacion y redución con cálculo mas cercano no es tan bueno pero es rápido
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);

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

    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/Creditos.bmp");
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

    //Jefe 2
    ruta = fullPath + "objects/sida.obj";
    models[JEFE2_MOD] = *glmReadOBJ(ruta.c_str());
    glmUnitize(&models[JEFE2_MOD]);
    glmVertexNormals(&models[JEFE2_MOD], 90.0, GL_TRUE);

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

    //inyeccion
    ruta = fullPath + "objects/inyeccion.obj";
    models[INYECCION_MOD] = *glmReadOBJ(ruta.c_str());
    glmUnitize(&models[INYECCION_MOD]);
    glmVertexNormals(&models[INYECCION_MOD], 90.0, GL_TRUE);

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

    //Espermas animados
    ruta = fullPath + "objects/esperma.obj";
    espermas[ESPERMA_MOD1] = *glmReadOBJ(ruta.c_str());
    glmUnitize(&espermas[ESPERMA_MOD1]);
    glmVertexNormals(&espermas[ESPERMA_MOD1], 90.0, GL_TRUE);

    ruta = fullPath + "objects/esperma2.obj";
    espermas[ESPERMA_MOD2] = *glmReadOBJ(ruta.c_str());
    glmUnitize(&espermas[ESPERMA_MOD2]);
    glmVertexNormals(&espermas[ESPERMA_MOD2], 90.0, GL_TRUE);

}

/*
///////////////////////////////////////////////////////////////////
/////////////    Inicializacion de Iluminacion    ////////////////
///////////////////////////////////////////////////////////////////
void initLightning()
{
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glShadeModel (GL_SMOOTH);//   GL_SMOOTH

}
*/

///////////////////////////////////////////////////////////////////
/////////////    Inicializacion principal    //////////////////////
///////////////////////////////////////////////////////////////////
void init()
{
    glClearColor (0.95, 0.54, 0.65, 0.7);
    glColor3f(1.0, 1.0, 1.0);

    initModels();
    initRendering();
    //initLightning();

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

    rutaMusica = fullPath + "Sounds/Menu.wav";
    PlaySound(TEXT(rutaMusica.c_str()),NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
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
    //gluLookAt(0, 0, 15, 0, 0, 0, 0, 1, 1);
    //gluLookAt(-16, 0, 2, 0, 0, 0, 1, 0, 1);
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
