#include "angulo.h"

struct constantes constantes_defecto(){
	struct constantes c;
	c.UTCOffset = 2;
	c.temperatura = 12;
	c.presion = 101325;
	c.GCR = 4/13.2;
	c.backtrack = 1;
	c.angulo_max = 60;
	return c;
}

uint8_t es_bisiesto(uint16_t anio){
	if((fmod(anio,4) == 0 && fmod(anio,100) != 0) || fmod(anio, 400) == 0){
		return 1;
	}
	else{
		return 0;
	}
}

double fecha2doy(struct tiempo t){
	uint8_t bisiesto = es_bisiesto(t.anio);
	uint8_t offsetMes[] = {1,2,0,1,1,2,2,3,4,4,5,5};
	uint8_t mes = t.mes;
	double dia1 = ((t.mes - 1) * 30) + offsetMes[mes - 1];
	double doy = dia1 + (t.dia - 1);
	if(t.mes > 2){
		doy += bisiesto;
	}
	return doy;
}

float Dec2Rad(float ang){
	return ang * M_PI/180.0;
}
float Rad2Dec(float ang){
	return ang * 180/M_PI;
}

float sign(float num){
	if(num > 0){
		return 1;
	}
	else if (num < 0){
		return -1;
	}
	else{
		return 0;
	}
}

struct ephemeris ephemeritas(struct constantes c, struct tiempo t, struct localizacion local){
	double UTCOffset = -1*c.UTCOffset;
	float longitud = -1*local.longitud;
	uint16_t doy = fecha2doy(t);
	float horaDec = t.hora + t.minuto/60.0 + t.segundo/3600.0;
	float fechaUniv = doy + floor((horaDec + UTCOffset)/24.0);
	float horaUniv = fmod((horaDec + UTCOffset), 24);
	float anio = t.anio - 1900;
	float anio_ini = 365 * anio + floor((anio-1)/4.0) - 0.5;
	float Ezero = anio_ini + fechaUniv; //no se que significa Ezero
	float T = Ezero / 36525.0; //siglo Julian de 36525 días
	float GMST0 = 6 / 24.0 + 38 / 1440.0 + (45.836 + 8640184.542 * T + 0.0929 * pow(T,2)) / 86400.0; //día sidéreo en el meridiano de Greenwich hora 0
	GMST0 = 360 * (GMST0 - floor(GMST0));
	float GMSTi = fmod(GMST0 + 360*(1.0027379093 * horaUniv / 24.0),360); //día sidéreo a la hora que es
	float LAST = fmod((360 + GMSTi - longitud), 360); //día sidereo aparente teniendo en cuenta la localización

	if(LAST < 0){ //corrección necesaria ya que fmod en c y en matlab no trabajan igual con numeros negativos
		LAST = 360 - LAST;
	}

	float fechaEpoch = Ezero + horaUniv/24.0;
	float T1 = fechaEpoch / 36525.0; //siglo julian de la epochDate
	float oblicuidad = Dec2Rad(23.452294 - 0.0130125 * T1 - 0.00000164 * pow(T1,2) + 0.000000503 * pow(T1,3)); //Calculo oblicuidad de la eclíptica
	float perigeo = 281.22083 + 0.0000470684 * fechaEpoch + 0.000453 * pow(T1,2) + 0.000003 * pow(T1,3); //cálculo perigeo del sol
	float meanSolarAnomaly = fmod((358.47583 + 0.985600267 * fechaEpoch - 0.00015 * pow(T1,2) - 0.000003 * pow(T1,3)), 360); //ángulo solar anomaly

	if(meanSolarAnomaly < 0){ //corrección necesaria ya que fmod en c y en matlab no trabajan igual con numeros negativos
		meanSolarAnomaly = 360 - meanSolarAnomaly;
	}

	float excentricidad = 0.01675104 - 0.0000418 * T1 - 0.000000126 * pow(T1,2);
	float ExcentAnom = meanSolarAnomaly;
	float E = 0;

	while(fabs(ExcentAnom - E) > 0.0001){
		E = ExcentAnom;
		ExcentAnom = meanSolarAnomaly + Rad2Dec(excentricidad*sin(Dec2Rad(E)));
	}

	float anomVerdadera = 2 * fmod(Rad2Dec(atan2(pow(((1 + excentricidad) / (1 - excentricidad)), 0.5) * tan(Dec2Rad(ExcentAnom / 2)), 1)), 360) ;

	if(anomVerdadera < 0){ //corrección necesaria ya que fmod en c y en matlab no trabajan igual con numeros negativos
		anomVerdadera = 2*360 + anomVerdadera;
	}

	float Abber = 20/3600.0;
	float ExLon = fmod(perigeo + anomVerdadera, 360) - Abber;
	float ExLon_Rad = Dec2Rad(ExLon);
	float Dec_Rad = asin(sin(oblicuidad) * sin(ExLon_Rad));
	float Dec = Rad2Dec(Dec_Rad);
	float RtAscen = Rad2Dec(atan2(cos(oblicuidad) * sin(ExLon_Rad), cos(ExLon_Rad)));
	float angHora = LAST - RtAscen;
	float angHora_Rad = Dec2Rad(angHora);

	angHora = angHora - (360 * sign(angHora) * (fabs(angHora) > 180));

	float Latitud_Rad = Dec2Rad(local.latitud);
	float Azimut_Sol = Rad2Dec(atan2(-1 * sin(angHora_Rad), cos(Latitud_Rad) * tan(Dec_Rad) - sin(Latitud_Rad) * cos(angHora_Rad)));
	Azimut_Sol = Azimut_Sol + (Azimut_Sol < 0) * 360; //Cambio de rango de [-180,180] a [0,360]
	Azimut_Sol = Azimut_Sol + 180; //Cambio ángulo 0 es el Sur en vez de el Norte
	if(Azimut_Sol >= 360){
		Azimut_Sol = Azimut_Sol - 360;
	}
	float Elevacion_Sol = Rad2Dec(asin(cos(Latitud_Rad) * cos(Dec_Rad) * cos(angHora_Rad) + sin(Latitud_Rad) * sin(Dec_Rad)));

	float horaSolar = (180 + angHora) / 15.0;

	//Calculo refraccion de la luz hasta que el centro del sol está 1º por debajo del horizonte
	float tangenteElevacion = tan(Dec2Rad(Elevacion_Sol));
	float refraccion;
	if(Elevacion_Sol > 5 && Elevacion_Sol <= 85){
		refraccion = 58.1 / tangenteElevacion - 0.07 / pow(tangenteElevacion, 3) + 0.000086 / pow(tangenteElevacion, 5);
	}
	else if(Elevacion_Sol > -0.575 && Elevacion_Sol <= 5){
		refraccion = Elevacion_Sol *(-518.2 + Elevacion_Sol * (103.4 + Elevacion_Sol * (-12.79 + Elevacion_Sol * 0.711))) + 1735;
	}
	else if(Elevacion_Sol > -1 && Elevacion_Sol <= -0.575){
		refraccion = -20.774 / tangenteElevacion;
	}
	else{
		refraccion = 0;
	}

	refraccion = refraccion * (283 / (273.0 + c.temperatura)) * c.presion / 101325 / 3600;

	//La elevacion aparente tiene en cuenta la refraccion
	float Elevacion_Aparente = Elevacion_Sol + refraccion;

	struct ephemeris datos;
	datos.Azimut = Azimut_Sol;
	datos.Elevacion = Elevacion_Sol;
	datos.Aparente_Elevacion = Elevacion_Aparente;
	datos.horaSolar = horaSolar;
	return datos; //los ángulos se devuelven en grados

}

struct posSol pos_Sol (float azimut, float elevacion){
	struct posSol pos;
	pos.x = cos(Dec2Rad(elevacion)) * sin(Dec2Rad(azimut));
	pos.y = cos(Dec2Rad(elevacion)) * cos(Dec2Rad(azimut));
	pos.z = sin(Dec2Rad(elevacion));
	return pos;
}

struct posSol pos_Sol_inclinado_y_desviado (struct posSol pos, float inclinacion, float azimut){
	struct posSol pos_res;
	pos_res.x = pos.x * cos(Dec2Rad(azimut)) - pos.y * sin(Dec2Rad(azimut));
	pos_res.y = pos.x * cos(Dec2Rad(inclinacion)) * sin(Dec2Rad(azimut)) + pos.y * cos(Dec2Rad(inclinacion)) * cos(Dec2Rad(azimut)) - pos.z * sin(Dec2Rad(inclinacion));
	pos_res.z = pos.x * sin(Dec2Rad(inclinacion)) * sin(Dec2Rad(azimut)) + pos.y * sin(Dec2Rad(inclinacion)) * cos(Dec2Rad(azimut)) + pos.z * cos(Dec2Rad(inclinacion));
	return pos_res;
}

float angulo_track_wid(struct posSol pos){
	float angulo = atan(pos.x / pos.z);
	//Por convenio cuando el sol se pone colocamos el tracker en posición horizontal, es decir, mirando al zenit
	if(pos.z <= 0){
		angulo = 0;
	}
	return Rad2Dec(angulo);
}

float backtracking(struct constantes c, float angulo_wid, struct posSol pos){
	float angulo_widc;
	float wc;
	if(c.backtrack == 0){
		angulo_widc = angulo_wid;
	}
	else{
		float LEW = 1/c.GCR;
		float cos_wc = LEW * cos(Dec2Rad(angulo_wid));
		if(cos_wc > 1){
			cos_wc = 1;
		}
		wc = acos(cos_wc); //angulo backtracking, siempre +
		wc = Rad2Dec(wc);
		if(angulo_wid >= 0){
			angulo_widc = angulo_wid - wc;
		}else{
			angulo_widc = angulo_wid + wc;
		}
	}

	//Aplicar correcciones de ángulo máximo y mínimo
	float angulo_tracker;
	if(pos.z <= 0){
		angulo_tracker = 0; //el sol esta por debajo de la horizontal, por convenio colocamos el tracker en posicion horizontal
	}
	else{
		angulo_tracker = angulo_widc;
	}

	if(angulo_tracker > c.angulo_max){
		angulo_tracker = c.angulo_max;
	}
	else if(angulo_tracker < (-c.angulo_max)){
		angulo_tracker = -c.angulo_max;
	}

	return angulo_tracker;
}

float producto_escalar(float widc, struct posSol pos){
	//coordenadas vector normal a la superficie panel
	float norm_x = sin(Dec2Rad(widc));
	float norm_y = 0;
	float norm_z = cos(Dec2Rad(widc));

	float prod_esc = norm_x * pos.x + norm_y * pos.y + norm_z * pos.z;

	return prod_esc;
}

float angulo_incidencia(float prod_escalar){
	float AOI = acos(fabs(prod_escalar));
	return Rad2Dec(AOI);
}
