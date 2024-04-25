#ifndef _ANGULO
#define _ANGULO

//Librería en la que se desarrollan las diferentes funciones necesarias para el cálculo del
//ángulo óptimo de inclinación del tracker, con posibilidad de tener en cuenta el backtracking

//Includes
#include "stdint.h"
#include "math.h"

//Estructuras
struct constantes{ //Valores constantes necesarios para realizar cálculos
	double UTCOffset; //Offset al estar en una zona horaria distinta de la de greenwich, por convenio con positivas hacia el este, en españa es +1
	float temperatura;
	float presion;
	double GCR; //Ratio cobertura/terreno
	double backtrack; //valor 0 no hay backtracking, valor distinto de 0 hay backtracking
	double angulo_max; //Ángulo máximo medido desde el zenit al que se puede inclinar el tracker
};

struct tiempo{
	double dia;
	double mes;
	double anio;
	double hora;
	double minuto;
	double segundo;
};

struct localizacion{
	double latitud;
	double longitud;
	double altitud;
};

struct ephemeris{ //Ángulos más importantes dados por las ecuaciones efemérides
	float Azimut;
	float Elevacion;
	float Aparente_Elevacion;
	float horaSolar;
};

struct posSol{ //Coordenadas cartesianas del Sol
	float x;
	float y;
	float z;
};

//Funciones
struct constantes constantes_defecto(); //Devuelve los valores de constantes génericas necesarias para los cálculos establecidos por defecto
uint8_t es_bisiesto(uint16_t anio); //Devuelve 1 si el año es bisiesto o 0 si no lo es
double fecha2doy(struct tiempo t); //Devuelve el día dentro del año de una fecha (1 de enero corresponde a 1 y 12 de diciembre a 365 o 366)
float Dec2Rad(float ang); //Conversión del ángulo dado en sexagesimal a radianes
float Rad2Dec(float ang); //Conversión del ángulo dado en radianes a sexagesimal
float sign(float num); //Devuelve 1 para números +, -1 para - y 0 para el 0
struct ephemeris ephemeritas(struct constantes, struct tiempo t, struct localizacion local); //Cálculo de los ángulos de posición del sol
struct posSol pos_Sol (float azimut, float elevacion); //función cálculo coordenadas cartesianas del sol, covenio:
//eje x -> oeste
//eje y -> sur
//eje z -> zenit
//los ángulos hay que introducirlos en grados
struct posSol pos_Sol_inclinado_y_desviado (struct posSol pos, float inclinacion, float azimut); //Correccion si el tracker esta inclinado o desviado del sur, el azimut por convenio es 0 en el sur
float angulo_track_wid(struct posSol pos); //Cálculo del ángulo ideal de inclinacion wid
float backtracking(struct constantes, float angulo_wid, struct posSol pos); //Cálculo del ángulo widc si hay backtracking
float producto_escalar(float widc, struct posSol pos); //Cálculos intermedios para obtener AOI
float angulo_incidencia(float prod_escalar); //Cálculo del AOI

//De la parte de pvl_singleaxis no estoy calculando el SurfTilt y el SurfAz porque creo que no son necesarios, posible añadido

#endif
