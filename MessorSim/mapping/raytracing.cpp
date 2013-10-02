#include "raytracing.h"

CRayTracing::CRayTracing(void)
{
}

CRayTracing::~CRayTracing(void)
{
}

//szukanie punktu przeci�cia promienia sensora z gruntem
bool CRayTracing::groundTrace(CIdealMap* map, float* position, float* sensor_position, float* orientation, float* sensor_orientation, int range, vector<long>* data, vector<float>* kat_radiany)
{
	//macierz po�o�enia robota
	robot_pos.createTRMatrix(orientation[0],orientation[1],orientation[2],position[0],position[1],position[2]);
	//macierz przesuni�cia i rotacji sensora wzgl�dem robota
	temp.createTRMatrix(sensor_orientation[0],sensor_orientation[1],sensor_orientation[2],sensor_position[0],sensor_position[1],sensor_position[2]);
	//macierz po�o�enia sensora
	sensor_pos=robot_pos*temp;
	
	temp.createTRMatrix(0,0,0,0,0,0);
	
	//�ledzimy promie� wysy�any przez czujnik a� do jego przeci�cia z gruntem lub przekroczenia zasi�gu skanera
	for (int i=0; i<range; i++)
	{
		//ustawiamy wsp�rz�dne badanego punktu (kolejny wzd�u� promienia
		temp.setElement((double)i/1000.,2,4);
		//macierz po�o�enia badanego punktu
		ray=sensor_pos*temp;
		//pobieramy wysoko�� terenu w tym punkcie
		map->CalculateGroundCoordinates(ray.getElement(1,4),ray.getElement(2,4),coordinates);
		
		if (ray.getElement(3,4) < map->map_ideal[coordinates[0]][coordinates[1]])
		{//je�eli wysoko�� na kt�rej znajduje si� badany punkt promienia jest mniejsza ni� wysoko�� gruntu - pobieramy pomiar
			data->push_back(i);
			return true;
		}
	}
	//je�eli nie znajdziemy punktu przeci�cia
	return false;
}