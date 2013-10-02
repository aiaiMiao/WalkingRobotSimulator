#pragma once
#include "raytracing.h"
#include "idealMap.h"
#include <gl/glut.h>
#include <vector>

using namespace std;

class CRangefinder
{
public:
	CRangefinder(void);
	~CRangefinder(void);
	virtual void rangefinding(){};
	CIdealMap* map;
};

//czujnik Hokuyo
class CHokuyoRangefinder:CRangefinder
{
public:
	CHokuyoRangefinder(CIdealMap* map);
	~CHokuyoRangefinder(void);
	//pomiar odleg�o�ci punktu od robota
	void rangefinding(float min, float max, float* robot_position, float* robot_orientation, vector<long>* data, vector<float>* kat_radiany);
public:
	//po�o�enie sensora w uk�adzie robota
	float sensor_position[3];
	//rotacja sensora w uk�adzie robota
	float sensor_orientation[3];
	//zasi�g czujnika
	int range;
	//rozdzielczo�� dokonywania pomiar�w
	float resolution;
};