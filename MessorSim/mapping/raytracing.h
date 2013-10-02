#include "idealMap.h"
#include "../math/punctum.h"
#include <vector>

using namespace std;

class CRayTracing
{
public:
	CRayTracing(void);
	~CRayTracing(void);
	//szukanie punktu przeci�cia promienia sensora z gruntem
	bool groundTrace(CIdealMap* map, float* position, float* sensor_position, float* orientation, float* sensor_orientation, int range, vector<long>* data, vector<float>* kat_radiany);
private:
	//macierze do oblicze� przekszta�ce� geometrycznych
	//pozycja robota
	CPunctum robot_pos;
	//macierz oblicze� tymczasowych
	CPunctum temp;
	//aktualnie sprawdzany punkt promienia (macierz w�druj�ca wzd�u� promienia a� do przeci�cia z gruntem)
	CPunctum ray;
	//pozycja sensora
	CPunctum sensor_pos;
	int coordinates[2];
};