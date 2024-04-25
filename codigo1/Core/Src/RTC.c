#include "RTC.h"

void set_time (RTC_HandleTypeDef hrtc, struct tiempo t)
{
	//Conversión de los datos de las estructuras utilizadas en las efemérides a la utilizada
	//por la librería HAL (ADVERTENCIA: HAL trabaja en BCD, necesaria conversión de los datos)
	RTC_TimeTypeDef sTime;
	sTime.Hours = hex_to_bcd(t.hora); // set hours
	sTime.Minutes = hex_to_bcd(t.minuto); // set minutes
	sTime.Seconds = hex_to_bcd(t.segundo); // set seconds
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;

	RTC_DateTypeDef sDate;
	sDate.WeekDay = RTC_WEEKDAY_THURSDAY; //day
	sDate.Month = hex_to_bcd(t.mes); //month
	sDate.Date = hex_to_bcd(t.dia); //date
	sDate.Year = hex_to_bcd(t.anio - 2000); //year

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
	  Error_Handler();
  }

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
	  Error_Handler();
  }
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x32F2); // backup register
}

struct tiempo get_time(RTC_HandleTypeDef hrtc){
	//Llamada a las funciones HAL para obtener los valores del tiempo
	RTC_TimeTypeDef gTime;
	HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN);
	RTC_DateTypeDef gDate;
	HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BIN);

	//Conversión de los datos a estructuras utilizadas en las efemérides
	struct tiempo t;
	t.anio = gDate.Year + 2000;
	t.mes = gDate.Month;
	t.dia = gDate.Date;
	t.hora = gTime.Hours;
	t.minuto = gTime.Minutes;
	t.segundo = gTime.Seconds;
	return t;
}

unsigned char hex_to_bcd(unsigned char data)
{
    unsigned char temp;

    temp = (((data/10)<<4) + (data%10));
    return temp;
}
