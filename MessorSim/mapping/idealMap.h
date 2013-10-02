#pragma once
#include "../ODE/OdeWorld.h"

class CIdealMap
{
public:
	CIdealMap(COdeWorld* dynamicWorld);
	~CIdealMap(void);
public:
	// zwraca maksymaln� wysoko��
	double getMaxHeight();
	// compute k1 coefficient
	float computeK1(int size, int x, int y);
	// compute k2 coefficient
	float computeK2(int size, int x, int y);
	///zwraca wysoko�� gruntu w danym punkcie 
	double getHeight(int x, int y);
	/// oblicz wspolczynniki jakosci wariancja sferyczna
	float ComputeSphericalVariance(int size, int x, int y);
	//get max height from the square
	double getMaxSquareHeight(int x, int y, int size);
	///zwraca pewno�� wysoko�ci gruntu w danym punkcie 
	double getReliability(int x, int y);
	///przelicza wspolrzedne "powierzchni" na wspolrzedne robota
	void CalculateGlobalCoordinates(int x, int y, float * result);
	///przelicza wspolrzedne robota na wspolrzedne terenu
	void CalculateGroundCoordinates(double x, double y, int * result);
	///wyznacza wsp�czynnik skali dla X
	void getScaleFactor(float *factor);
	///wyznacza wsp�czynnik skali dla Y
	double getScaleFactorY();
	///wyznacza wsp�czynnik skali dla X
	double getScaleFactorX();
	///get mesh_x_size
	int getXnumPoints();
	///get mesh_y_size
	int getYnumPoints();
	///get length (x) in meteres
	double getXsize();
	///get length (y) in meters
	double getYsize();
	///wyznacza wsp�rz�dn� Y �rodka terenu w jego wspolrzednych
	double getCenterX();
	///wyznacza wsp�rz�dn� Y �rodka terenu w jego wspolrzednych
	double getCenterY();
	//funkcja do zaokraglania
	int my_round (double x);
public:
	//tablica zawieraj�ca wysoko�� punkt�w okre�lonych wsp�rz�dnymi x i y
	double ** map_ideal;
	COdeWorld* dynamicWorld;
};