#include "laser_rangefinder.h"

CRayTracing raytracer;

CRangefinder::CRangefinder(void)
{
}

CRangefinder::~CRangefinder(void)
{
}

CHokuyoRangefinder::CHokuyoRangefinder(CIdealMap* map)
{
	this->map=map;

	//ustawienie parametr�w czujnika Hokuyo
	sensor_position[0] = 0; //pozycja sensora w uk�adzie robota w osi: x
	sensor_position[1] = 0.202;//y
	sensor_position[2] = 0.15;//z

	sensor_orientation[0] = -0.785398; //-45 stopni - pochylenie sensora w uk�adzie robota wok� osi: x (pitch)
	sensor_orientation[1] = 0; //y (roll)
	sensor_orientation[2] = 0; //z (yaw)

	range=1500; //1.5 m
	resolution = 0.006283185307; //0.36 stopnia
}

CHokuyoRangefinder::~CHokuyoRangefinder(void)
{
}

void CHokuyoRangefinder::rangefinding(float min, float max, float* robot_position, float* robot_orientation, vector<long>* data, vector<float>* kat_radiany)
{
	//ustawienie wst�pnej orientacji sensora - sk�d zaczynamy pomiar wzgl�dem k�ta 0 le��cego na osi y robota (orientacja ujemna - robot "patrzy" sensorem w prawo, dodatnia - w lewo)
	float sensor_scan_orientation[3];
	sensor_scan_orientation[0]=sensor_orientation[0];
	sensor_scan_orientation[1]=sensor_orientation[1];
	sensor_scan_orientation[2]=sensor_orientation[2]+min;
	//czyszczenie wektor�w danych wynikowych
	while (data->size()>0)
		data->pop_back();
	while (kat_radiany->size()>0)
		kat_radiany->pop_back();
	//ustalenie liczby dokonanych pomiar�w na podstawie okre�lonego maksymalnego i minimalnego k�ta oraz rozdzielczo�ci pomiaru dla danego sensora
	float scans = (max-min)/resolution;
	//pomiary
	for (int i=0; i<scans; i++)
	{
		//obracanie sensora w osi z o k�t okre�lony jako jego rozdzielczo��
		sensor_scan_orientation[2]=sensor_scan_orientation[2]+resolution;
		if(raytracer.groundTrace(map, robot_position, sensor_position, robot_orientation, sensor_scan_orientation, range, data, kat_radiany))
		{//je�eli pomiar wykonany zosta� prawid�owo (nie przekroczono zasi�gu sensora, do wektora 'data' zapisany zosta� pomiar odleg�o�ci) dodajemy do wektora 'kat_radiany' pomiar k�ta
			kat_radiany->push_back(((i+1)*resolution)+min);
		}
	}
}