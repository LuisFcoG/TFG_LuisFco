#ifndef _RTC
#define _RTC

//Librería desarrollada como intermediario para la llamada de las funciones del RTC de la
//librería HAL

#include "main.h"
#include "angulo.h"

//Funciones
void set_time (RTC_HandleTypeDef hrtc, struct tiempo t); //Establece la inicialización del RTC con los valores indicados
struct tiempo get_time(RTC_HandleTypeDef hrtc); //Devuelve el tiempo real del RTC
unsigned char hex_to_bcd(unsigned char data); //Conversión de hexadecimal a BCD ya que la librería HAL trabaja con BCD y el código desarrollado de efemérides con hexadecimal

#endif
