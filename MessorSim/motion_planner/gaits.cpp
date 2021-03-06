#include "gaits.h"
#include <math.h>
//#include <unistd.h>

CGaits::CGaits(void)
{
	setInitZeroAngle();
	smart_gait_iterator = 0;
	fposlizg = fopen("poslizg.txt","w");
}

CGaits::~CGaits(void)
{

}

/// ustawia poczatkowa konfiguracje neutralna dla robota (0, 24, -114)
void CGaits::setInitZeroAngle()
{
	for (int i=0;i<6;i++){
	    	if ((i==0)||(i==5)) robot->leg[i].zero_angle[0]=(float)0.785;
		if ((i==1)||(i==4)) robot->leg[i].zero_angle[0]=(float)0;
	    	if ((i==2)||(i==3)) robot->leg[i].zero_angle[0]=(float)-0.785;
		robot->leg[i].zero_angle[1]=(float)0.31867;
		robot->leg[i].zero_angle[2]=(float)-1.4887;
	}
}

/// ustawia konczyne w pozycji neutralnej, offset_up - wysokosc podnoszenia stop
bool CGaits::zeroPosition(unsigned char leg_no, float offset_up, float speed)
{
	//leg "leg_no" up
	if (!robot->changeFootRobot(0, 0, offset_up, leg_no, speed))
		return false;

	//leg to neutral position
	int part;
	if (leg_no<3) part=1;
	else part=0;
	CPunctum neutral_point = robot->leg[leg_no].forward_kinematic(robot->leg[leg_no].zero_angle[0],robot->leg[leg_no].zero_angle[1],robot->leg[leg_no].zero_angle[2],part);
	CPunctum end_point = robot->leg[leg_no].forward_kinematic(robot->leg[leg_no].getAngle(0),robot->leg[leg_no].getAngle(1),robot->leg[leg_no].getAngle(2),part);
	end_point = neutral_point-end_point;//zadane przesuniecie do pozycji neutralnej
	if (!robot->changeFoot(end_point.position.getElement(1,4), end_point.position.getElement(2,4), end_point.position.getElement(3,4), leg_no, speed))
		return false;
	return true;
}

/// ustawia konczyny w pozycji neutralnej
bool CGaits::setZeroPosition(float offset_up, float speed)
{
	if (!zeroPosition(0, offset_up, speed)) return false;
	robot->sleepODE(100);
	if (!zeroPosition(3, offset_up, speed)) return false;
	robot->sleepODE(100);
	if (!zeroPosition(1, offset_up, speed)) return false;
	robot->sleepODE(100);
	if (!zeroPosition(4, offset_up, speed)) return false;
	robot->sleepODE(100);
	if (!zeroPosition(2, offset_up, speed)) return false;
	robot->sleepODE(100);
	if (!zeroPosition(5, offset_up, speed)) return false;
	return true;
}

/// prepare to tripod gait - set neutral position of legs and move platform to init level
bool CGaits::tripodPrepare(unsigned int delay_up, unsigned int delay_forward, float x, float y, float z, float alpha, float beta, float gamma, float offset_up, float speed)
{
	//setZeroPosition(offset_up, speed);
	if (!robot->changePlatformRobot(0, 0, z, 0, 0, 0, speed,1)) return false;
	//unosimy parzyste
	float x_ref[6],y_ref[6],z_ref[6];
	for (int i=0;i<6;i++){
	    	x_ref[i]=0; y_ref[i]=0; 
		if ((i==1)||(i==3)||(i==5)) z_ref[i]=offset_up; else z_ref[i]=0;
	}
	if (!robot->changeAllFootsRobot(x_ref, y_ref, z_ref, speed)) return false;
	robot->sleepODE(delay_up/1000);//usleep(delay_up);
	return true;
}

/// tripod step
bool CGaits::tripodStep(unsigned int delay_up, unsigned int delay_forward, float x, float y, float z, float alpha, float beta, float gamma, float offset_up, float speed,int accel)
{
	x=x/4.0; y=y/4.0; z=z/4.0;
	alpha=alpha/4.0; beta=beta/4.0; gamma=gamma/4.0;
	float x_ref[6],y_ref[6],z_ref[6],alpha_ref[6],beta_ref[6],gamma_ref[6];
	//8 faz ruchu
	//zakrok na nieparzystych
	for (int i=0;i<6;i=i+2){
	    	x_ref[i]=x; y_ref[i]=y; z_ref[i]=z; alpha_ref[i]=alpha; beta_ref[i]=beta; gamma_ref[i]=gamma;
	}
	for (int i=1;i<6;i=i+2){
	    	x_ref[i]=-x; y_ref[i]=-y; z_ref[i]=z; alpha_ref[i]=-alpha; beta_ref[i]=-beta; gamma_ref[i]=-gamma;
	}
	if (!robot->changePlatformRobot(x_ref, y_ref, z_ref, alpha_ref, beta_ref, gamma_ref, speed, accel)) return false;
	robot->sleepODE(delay_forward/1000);//usleep(delay_forward);
	//opuszczamy parzyste
	for (int i=0;i<6;i++){
	    	x_ref[i]=0; y_ref[i]=0; 
		if ((i==1)||(i==3)||(i==5)) z_ref[i]=-offset_up; else z_ref[i]=0;
	}
	if (!robot->changeAllFootsRobot(x_ref, y_ref, z_ref, speed)) return false;
	robot->sleepODE(delay_up/1000);//usleep(delay_up);
	//podnosimy nieparzyste
	for (int i=0;i<6;i++){
	    	x_ref[i]=0; y_ref[i]=0; 
		if ((i==0)||(i==2)||(i==4)) z_ref[i]=offset_up; else z_ref[i]=0;
	}
	if (!robot->changeAllFootsRobot(x_ref, y_ref, z_ref, speed)) return false;
	robot->sleepODE(delay_up/1000);//usleep(delay_up);
	//przejscie do polowy na parzystych
	for (int i=0;i<6;i=i+2){
	    	x_ref[i]=-x; y_ref[i]=-y; z_ref[i]=z; alpha_ref[i]=-alpha; beta_ref[i]=-beta; gamma_ref[i]=-gamma;
	}
	for (int i=1;i<6;i=i+2){
	    	x_ref[i]=x; y_ref[i]=y; z_ref[i]=z; alpha_ref[i]=alpha; beta_ref[i]=beta; gamma_ref[i]=gamma;
	}
	if (!robot->changePlatformRobot(x_ref, y_ref, z_ref, alpha_ref, beta_ref, gamma_ref, speed,accel)) return false;
	robot->sleepODE(delay_forward/1000);//usleep(delay_forward);
	//zakrok na parzystych
	if (!robot->changePlatformRobot(x_ref, y_ref, z_ref, alpha_ref, beta_ref, gamma_ref, speed,accel)) return false;
	//opuszczamy nieparzyste
	for (int i=0;i<6;i++){
	    	x_ref[i]=0; y_ref[i]=0; 
		if ((i==0)||(i==2)||(i==4)) z_ref[i]=-offset_up; else z_ref[i]=0;
	}
	if (!robot->changeAllFootsRobot(x_ref, y_ref, z_ref, speed)) return false;
	robot->sleepODE(delay_forward/1000);//usleep(delay_forward);
	//podnosimy parzyste
	for (int i=0;i<6;i++){
	    	x_ref[i]=0; y_ref[i]=0; 
		if ((i==1)||(i==3)||(i==5)) z_ref[i]=offset_up; else z_ref[i]=0;
	}
	if (!robot->changeAllFootsRobot(x_ref, y_ref, z_ref, speed)) return false;
	robot->sleepODE(delay_up/1000);//usleep(delay_up);
	// przejscie do pozycji neutralnej na nieparzystych
	for (int i=0;i<6;i=i+2){
	    	x_ref[i]=x; y_ref[i]=y; z_ref[i]=z; alpha_ref[i]=alpha; beta_ref[i]=beta; gamma_ref[i]=gamma;
	}
	for (int i=1;i<6;i=i+2){
	    	x_ref[i]=-x; y_ref[i]=-y; z_ref[i]=z; alpha_ref[i]=-alpha; beta_ref[i]=-beta; gamma_ref[i]=-gamma;
	}
	if (!robot->changePlatformRobot(x_ref, y_ref, z_ref, alpha_ref, beta_ref, gamma_ref, speed,accel)) return false;
	robot->sleepODE(delay_forward/1000);//usleep(delay_forward);
	return true;
}

/// short tripod step  - cycle is shorter:triangular trajectory of the leg's tip
bool CGaits::shortTripodStep(unsigned int delay_up, unsigned int delay_forward, float x, float y, float z, float alpha, float beta, float gamma, float offset_up, float speed, int accel){
	x=x/4.0; y=y/4.0; z=z/4.0;
	alpha=alpha/4.0; beta=beta/4.0; gamma=gamma/4.0;
	float x_ref[6],y_ref[6],z_ref[6],alpha_ref[6],beta_ref[6],gamma_ref[6];
	//8 faz ruchu
	//zakrok na nieparzystych
	for (int i=0;i<6;i=i+2){
	    	x_ref[i]=x; y_ref[i]=y; z_ref[i]=z; alpha_ref[i]=alpha; beta_ref[i]=beta; gamma_ref[i]=gamma;
	}
	for (int i=1;i<6;i=i+2){
	    	x_ref[i]=-x; y_ref[i]=-y; z_ref[i]=z+offset_up; alpha_ref[i]=-alpha; beta_ref[i]=-beta; gamma_ref[i]=-gamma;
	}
	if (!robot->changePlatformRobot(x_ref, y_ref, z_ref, alpha_ref, beta_ref, gamma_ref, speed,accel)) return false;
	robot->sleepODE(delay_forward/1000);//usleep(delay_forward);

	//przejscie do polowy na parzystych
	for (int i=0;i<6;i=i+2){
	    	x_ref[i]=-x; y_ref[i]=-y; z_ref[i]=z-offset_up; alpha_ref[i]=-alpha; beta_ref[i]=-beta; gamma_ref[i]=-gamma;
	}
	for (int i=1;i<6;i=i+2){
	    	x_ref[i]=x; y_ref[i]=y; z_ref[i]=z; alpha_ref[i]=alpha; beta_ref[i]=beta; gamma_ref[i]=gamma;
	}
	if (!robot->changePlatformRobot(x_ref, y_ref, z_ref, alpha_ref, beta_ref, gamma_ref, speed, accel)) return false;
	robot->sleepODE(delay_forward/1000);//usleep(delay_forward);
	//zakrok na parzystych
	for (int i=0;i<6;i=i+2){
	    	x_ref[i]=-x; y_ref[i]=-y; z_ref[i]=z+offset_up; alpha_ref[i]=-alpha; beta_ref[i]=-beta; gamma_ref[i]=-gamma;
	}
	if (!robot->changePlatformRobot(x_ref, y_ref, z_ref, alpha_ref, beta_ref, gamma_ref, speed, accel)) return false;
	robot->sleepODE(delay_forward/1000);//usleep(delay_forward);
	// przejscie do pozycji neutralnej na nieparzystych
	for (int i=0;i<6;i=i+2){
	    	x_ref[i]=x; y_ref[i]=y; z_ref[i]=z; alpha_ref[i]=alpha; beta_ref[i]=beta; gamma_ref[i]=gamma;
	}
	for (int i=1;i<6;i=i+2){
	    	x_ref[i]=-x; y_ref[i]=-y; z_ref[i]=z-offset_up; alpha_ref[i]=-alpha; beta_ref[i]=-beta; gamma_ref[i]=-gamma;
	}
	if (!robot->changePlatformRobot(x_ref, y_ref, z_ref, alpha_ref, beta_ref, gamma_ref, speed, accel)) return false;
	robot->sleepODE(delay_forward/1000);//usleep(delay_forward);
	return true;
}


/// finish tripod gait - set neutral position of legs and move platform to init level
bool CGaits::tripodFinish(unsigned int delay_up, unsigned int delay_forward, float x, float y, float z, float alpha, float beta, float gamma, float offset_up, float speed)
{
	//setZeroPosition(offset_up, speed);
	if (!robot->changePlatformRobot(0, 0, 0.0, 0, 0, 0, speed,1)) return false;
	//unosimy parzyste
	float x_ref[6],y_ref[6],z_ref[6];
	for (int i=0;i<6;i++){
	    	x_ref[i]=0; y_ref[i]=0; 
		if ((i==1)||(i==3)||(i==5)) z_ref[i]=-offset_up; else z_ref[i]=0;
	}
	if (!robot->changeAllFootsRobot(x_ref, y_ref, z_ref, speed)) return false;
	robot->sleepODE(delay_up/1000);//usleep(delay_up);
	return true;
}


/// prepare to wave gait - set neutral position of legs and move platform to init level
bool CGaits::wavePrepare(unsigned int delay_up, unsigned int delay_forward, float x, float y, float z, float alpha, float beta, float gamma, float offset_up, float speed, int accel)
{
	float x_ref[6],y_ref[6],z_ref[6],alpha_ref[6],beta_ref[6],gamma_ref[6];

	//setZeroPosition(offset_up, speed);
	//robot->changePlatformRobot(0, 0, z, 0, 0, 0, speed);

	for (int i=0;i<6;i++){	x_ref[i]=0; y_ref[i]=0; z_ref[i]=0; alpha_ref[i]=0; beta_ref[i]=0; gamma_ref[i]=0;} //init

	int i = 0;
	float dividor = 0;
	while (i<6) {
		// noga 1 do gory
		if (!robot->changeFootRobot(0, 0, offset_up, i, speed)) return false;
		robot->sleepODE(delay_up/1000);//usleep(delay_up);

		// noga i do przodu
		x_ref[i]=x*(-0.5+dividor); y_ref[i]=y*(-0.5+dividor); z_ref[i]=-z*(-0.5+dividor); alpha_ref[i]=alpha*(-0.5+dividor); beta_ref[i]=beta*(-0.5+dividor); gamma_ref[i]=gamma*(-0.5+dividor);
		if (!robot->changePlatformRobot(x_ref, y_ref, z_ref, alpha_ref, beta_ref, gamma_ref, speed, accel)) return false;
		robot->sleepODE(delay_forward/1000);//usleep(delay_forward);
		x_ref[i]=0; y_ref[i]=0; z_ref[i]=0; alpha_ref[i]=0; beta_ref[i]=0; gamma_ref[i]=0;

		// noga i na dol
		if (!robot->changeFootRobot(0, 0, -offset_up, i, speed)) return false;
		robot->sleepODE(delay_up/1000);//usleep(delay_up);

		if (i==0) i = 5;
		else if (i==1) i = 4;
		else if (i==2) i = 3;
		else if (i==4) i = 2;
		else if (i==5) i = 1;
		else i = 6; //koniec cyklu
		dividor+=0.2;
	}
	return true;
}


/// wave step
bool CGaits::waveStep(unsigned int delay_up, unsigned int delay_forward, float x, float y, float z, float alpha, float beta, float gamma, float offset_up, float speed, int accel)
{
	float x_ref[6],y_ref[6],z_ref[6],alpha_ref[6],beta_ref[6],gamma_ref[6];
	
	for (int i=0;i<6;i++){	x_ref[i]=0; y_ref[i]=0; z_ref[i]=0; alpha_ref[i]=0; beta_ref[i]=0; gamma_ref[i]=0;} //init

	int leg_no = 3;
	while (leg_no<6) {
		// noga leg_no do gory
		if (!robot->changeFootRobot(0, 0, offset_up, leg_no, speed)) return false;
		z_ref[leg_no]=0;

		// noga leg_no do przodu, pozostale o 1/5 do tylu
		for (int i=0;i<6;i++) {
			if (i==leg_no) {
				x_ref[i]=-x; y_ref[i]=-y; z_ref[i]=-z; alpha_ref[i]=-alpha; beta_ref[i]=-beta; gamma_ref[i]=-gamma;
			}
			else {
				x_ref[i]=x*0.2; y_ref[i]=y*0.2; z_ref[i]=z*0.2; alpha_ref[i]=alpha*0.2; beta_ref[i]=beta*0.2; gamma_ref[i]=gamma*0.2;
			}
		}
		if (!robot->changePlatformRobot(x_ref, y_ref, z_ref, alpha_ref, beta_ref, gamma_ref, speed, accel)) return false;
		robot->sleepODE(delay_forward/1000);//usleep(delay_forward);
	
		// noga leg_no na ziemie
		if (!robot->changeFootRobot(0, 0, -offset_up-z, leg_no, speed)) return false;
		robot->sleepODE(delay_up/1000);//usleep(delay_up);

		if (leg_no==1) leg_no = 5;
		else if (leg_no==2) leg_no = 4;
		else if (leg_no==3) leg_no = 2;
		else if (leg_no==4) leg_no = 1;
		else if (leg_no==5) leg_no = 0;
		else leg_no = 6; //koniec cyklu
	}
	return true;
}

/// short wave step - cycle is shorter:triangular trajectory of the leg's tip
bool CGaits::shortWaveStep(unsigned int delay_up, unsigned int delay_forward, float x, float y, float z, float alpha, float beta, float gamma, float offset_up, float speed, int accel)
{
	float x_ref[6],y_ref[6],z_ref[6],alpha_ref[6],beta_ref[6],gamma_ref[6];
	
	for (int i=0;i<6;i++){	x_ref[i]=0; y_ref[i]=0; z_ref[i]=0; alpha_ref[i]=0; beta_ref[i]=0; gamma_ref[i]=0;} //init

	int leg_no = 3;
	while (leg_no<6) {
		//narastajace ramie trojkata
		for (int i=0;i<6;i++) {
			if (i==leg_no) {
				x_ref[i]=-x/2.0; y_ref[i]=-y/2.0; z_ref[i]=-z/2.0-offset_up; alpha_ref[i]=-alpha/2.0; beta_ref[i]=-beta/2.0; gamma_ref[i]=-gamma/2.0;
			}
			else {
				x_ref[i]=x*0.1; y_ref[i]=y*0.1; z_ref[i]=z*0.1; alpha_ref[i]=alpha*0.1; beta_ref[i]=beta*0.1; gamma_ref[i]=gamma*0.1;
			}
		}
		if (!robot->changePlatformRobot(x_ref, y_ref, z_ref, alpha_ref, beta_ref, gamma_ref, speed, accel)) return false;
		robot->sleepODE(delay_forward/1000);//usleep(delay_forward);
		//opadajace ramie trojkat
		for (int i=0;i<6;i++) {
			if (i==leg_no) {
				x_ref[i]=-x/2.0; y_ref[i]=-y/2.0; z_ref[i]=-z/2.0+offset_up; alpha_ref[i]=-alpha/2.0; beta_ref[i]=-beta/2.0; gamma_ref[i]=-gamma/2.0;
			}
			else {
				x_ref[i]=x*0.1; y_ref[i]=y*0.1; z_ref[i]=z*0.1; alpha_ref[i]=alpha*0.1; beta_ref[i]=beta*0.1; gamma_ref[i]=gamma*0.1;
			}
		}
		if (!robot->changePlatformRobot(x_ref, y_ref, z_ref, alpha_ref, beta_ref, gamma_ref, speed, accel)) return false;
		robot->sleepODE(delay_forward/1000);//usleep(delay_forward);

		if (leg_no==1) leg_no = 5;
		else if (leg_no==2) leg_no = 4;
		else if (leg_no==3) leg_no = 2;
		else if (leg_no==4) leg_no = 1;
		else if (leg_no==5) leg_no = 0;
		else leg_no = 6; //koniec cyklu
	}
	return true;
}


/// prepare to smart gait
bool CGaits::SmartGait(float x, float y, float z, float rotx, float roty, float rotz, float foot_up, float speed, int accel){
	float iter_no = 10;
	float imu_rot[3];
	float x_foot1[6];//wspolrzedne x stop robota
	float y_foot1[6];
	float z_foot1[6];//wspolrzedne z stop robota
	float x_foot_after[6];//wspolrzedne x stop robota
	float y_foot_after[6];
	float z_foot_after[6];//wspolrzedne z stop robota
	float foot_distance[6]; // poslizg
	/*for (int i=0;i<3*iter_no;i++){
		if (!robot->changePlatformRobot(0,0,0.005,0,0,0,speed,accel))//przenosimy platforme
		  return false;
		else
			robot->modifyRobotPosition(0,0,0.005,0,0,0);
		//make scan
		usleep(100000);
		rpccaller_map->makeScan(-1.57, 1.57, 1, robot->robot_global_pos.getElement(1,4),robot->robot_global_pos.getElement(2,4),robot->robot_global_pos.getElement(3,4),robot->robot_platform_pos[3],robot->robot_platform_pos[4],robot->robot_platform_pos[5]);
	}
	for (int i=0;i<3*iter_no;i++){
		if (!robot->changePlatformRobot(0,0,-0.005,0,0,0,speed,accel))//przenosimy platforme
		  return false;
		else
			robot->modifyRobotPosition(0,0,-0.005,0,0,0);
		//make scan
		usleep(100000);
		rpccaller_map->makeScan(-1.57, 1.57, 1, robot->robot_global_pos.getElement(1,4),robot->robot_global_pos.getElement(2,4),robot->robot_global_pos.getElement(3,4),robot->robot_platform_pos[3],robot->robot_platform_pos[4],robot->robot_platform_pos[5]);
	}	*/
        CPunctum movement;
	movement.createTRMatrix(rotx, roty, rotz, x, y, z);
	CPunctum  robot_pos = robot->robot_global_pos * movement;//pozycja robota po ruchu
	CPunctum rob_pos = robot->getRobotState();
	z=0.215-rob_pos.position.getElement(3,4);
	//obliczenie pozycji stop po wykonaniu ruchu
	// obliczenie trajektorii
	float x_foot[6]={0,0,0,0,0,0};
	float y_foot[6]={0,0,0,0,0,0};
	float z_foot[6]={0.0,0,0.0,0,0.0,0};
	for (int i=smart_gait_iterator;i<6;i+=2) z_foot[i]=0.14;
	while (!robot->changeAllFootsRobot(x_foot, y_foot, z_foot, speed)){//move legs up
		for (int i=smart_gait_iterator;i<6;i+=2) z_foot[i]-=0.01;
		if (z_foot[smart_gait_iterator]<=0) 
			return false;
	}
	//SleepODE(500000);
	float x_robot[6]={x/iter_no,x/iter_no,x/iter_no,x/iter_no,x/iter_no,x/iter_no};
	float y_robot[6]={y/iter_no,y/iter_no,y/iter_no,y/iter_no,y/iter_no,y/iter_no};
	float z_robot[6]={z/iter_no,z/iter_no,z/iter_no,z/iter_no,z/iter_no,z/iter_no};
	float alpha_robot[6]={rotx/iter_no,rotx/iter_no,rotx/iter_no,rotx/iter_no,rotx/iter_no,rotx/iter_no};
	float beta_robot[6]={roty/iter_no,roty/iter_no,roty/iter_no,roty/iter_no,roty/iter_no,roty/iter_no};
	float gamma_robot[6]={rotz/iter_no,rotz/iter_no,rotz/iter_no,rotz/iter_no,rotz/iter_no,rotz/iter_no};
	for (int i=smart_gait_iterator;i<6;i+=2) {
	    x_robot[i]=-x_robot[i]; y_robot[i]=-y_robot[i]; z_robot[i]=-z_robot[i];
	    alpha_robot[i]=-alpha_robot[i]; beta_robot[i]=-beta_robot[i]; gamma_robot[i]=-gamma_robot[i];
	}
	for (int n=0;n<6;n++)//odczyt pozycji stopy przed ruchem
		// robot->dynamicWorld->rec_robot_leg[n].getLastPoint(&x_foot1[n],&y_foot1[n],&z_foot1[n]);
	for (int i=0;i<iter_no;i++){
		for (int j=smart_gait_iterator;j<6;j+=2){
			x_robot[j]=(i+1)*(-x/iter_no); y_robot[j]=(i+1)*(-y/iter_no); z_robot[j]=(i+1)*(-z/iter_no);
			alpha_robot[j]=(i+1)*(-rotx/iter_no); beta_robot[j]=(i+1)*(-roty/iter_no); gamma_robot[j]=(i+1)*(-rotz/iter_no);
		}
		
		if (!robot->changePlatformRobotNeutral(x_robot,y_robot,z_robot,alpha_robot,beta_robot,gamma_robot,z_foot[smart_gait_iterator]-foot_up,smart_gait_iterator,speed,accel))//przenosimy platforme
			return false;
		else
			robot->modifyRobotPosition(x/iter_no, y/iter_no, z/iter_no, rotx/iter_no, roty/iter_no, rotz/iter_no);
		//make scan
		//SleepODE(100000);
		robot->getRotAngles(imu_rot);
	//	rpccaller_map->makeScan(-1.57, 1.57, 1, robot->robot_global_pos.getElement(1,4),robot->robot_global_pos.getElement(2,4),robot->robot_global_pos.getElement(3,4), imu_rot[0],imu_rot[1],0);
	}
	for (int n=0;n<6;n++)//odczyt pozycji stopy po ruchu
		// robot->dynamicWorld->rec_robot_leg[n].getLastPoint(&x_foot_after[n],&y_foot_after[n],&z_foot_after[n]);
	for (int n=0;n<6;n++)//obliczenie poslizgu
		foot_distance[n]=sqrt(pow(x_foot_after[n]-x_foot1[n],2)+pow(y_foot_after[n]-y_foot1[n],2)+pow(z_foot_after[n]-z_foot1[n],2));
	//odczyt aktualnej pozycji
	fprintf(fposlizg,"%f, ",foot_distance[1]+foot_distance[3]+foot_distance[5]); //zapisujemy dane wejsciowe
		
	//SleepODE(500000);
	for (int i=0;i<6;i++) z_foot[i]=0;
	if (!MoveLegsDown(smart_gait_iterator,0,0,&z_foot[0]))//nieparzyste nogi na ziemie
	  return false;
	for (int i=smart_gait_iterator;i<6;i+=2) z_foot[i]=-z_foot[i];
	if (!robot->changeAllFootsRobot(x_foot, y_foot, z_foot, speed))//opuszczamy konczyny na dol
	  return false;
//	if (!robot->PlaceFoots(0.005, smart_gait_iterator, speed))
//	  return false;
//	robot->stabilizePlatform(speed, accel);
	SleepODE(100);
	robot->stabilizePlatform(0.05, accel);
	if (smart_gait_iterator == 1) //next step
	   smart_gait_iterator = 0;
	else
	   smart_gait_iterator = 1;
	return true;
}

/// prepare to smart gait
bool CGaits::SmartGaitFoothold(float x, float y, float z, float rotx, float roty, float rotz, float foot_up, float speed, int accel){
	float iter_no = 10;
	float imu_rot[3];
	float x_foot1[6];//wspolrzedne x stop robota
	float y_foot1[6];
	float z_foot1[6];//wspolrzedne z stop robota
	float x_foot_after[6];//wspolrzedne x stop robota
	float y_foot_after[6];
	float z_foot_after[6];//wspolrzedne z stop robota
	float foot_distance[6]; // poslizg
	/*for (int i=0;i<3*iter_no;i++){
		if (!robot->changePlatformRobot(0,0,0.005,0,0,0,speed,accel))//przenosimy platforme
		  return false;
		else
			robot->modifyRobotPosition(0,0,0.005,0,0,0);
		//make scan
		usleep(100000);
		rpccaller_map->makeScan(-1.57, 1.57, 1, robot->robot_global_pos.getElement(1,4),robot->robot_global_pos.getElement(2,4),robot->robot_global_pos.getElement(3,4),robot->robot_platform_pos[3],robot->robot_platform_pos[4],robot->robot_platform_pos[5]);
	}
	for (int i=0;i<3*iter_no;i++){
		if (!robot->changePlatformRobot(0,0,-0.005,0,0,0,speed,accel))//przenosimy platforme
		  return false;
		else
			robot->modifyRobotPosition(0,0,-0.005,0,0,0);
		//make scan
		usleep(100000);
		rpccaller_map->makeScan(-1.57, 1.57, 1, robot->robot_global_pos.getElement(1,4),robot->robot_global_pos.getElement(2,4),robot->robot_global_pos.getElement(3,4),robot->robot_platform_pos[3],robot->robot_platform_pos[4],robot->robot_platform_pos[5]);
	}	*/
        CPunctum movement;
	movement.createTRMatrix(rotx, roty, rotz, x, y, z);
	CPunctum  robot_pos = robot->robot_global_pos * movement;//pozycja robota po ruchu
	CPunctum rob_pos = robot->getRobotState();
	z=0.215-rob_pos.position.getElement(3,4);
	//obliczenie pozycji stop po wykonaniu ruchu
	// obliczenie trajektorii
	float x_foot[6]={0,0,0,0,0,0};
	float y_foot[6]={0,0,0,0,0,0};
	float z_foot[6]={0.0,0,0.0,0,0.0,0};
	for (int i=smart_gait_iterator;i<6;i+=2) z_foot[i]=0.14;
	while (!robot->changeAllFootsRobot(x_foot, y_foot, z_foot, speed)){//move legs up
		for (int i=smart_gait_iterator;i<6;i+=2) z_foot[i]-=0.01;
		if (z_foot[smart_gait_iterator]<=0) 
			return false;
	}
	//SleepODE(500000);
	float x_robot[6]={x/iter_no,x/iter_no,x/iter_no,x/iter_no,x/iter_no,x/iter_no};
	float y_robot[6]={y/iter_no,y/iter_no,y/iter_no,y/iter_no,y/iter_no,y/iter_no};
	float z_robot[6]={z/iter_no,z/iter_no,z/iter_no,z/iter_no,z/iter_no,z/iter_no};
	float alpha_robot[6]={rotx/iter_no,rotx/iter_no,rotx/iter_no,rotx/iter_no,rotx/iter_no,rotx/iter_no};
	float beta_robot[6]={roty/iter_no,roty/iter_no,roty/iter_no,roty/iter_no,roty/iter_no,roty/iter_no};
	float gamma_robot[6]={rotz/iter_no,rotz/iter_no,rotz/iter_no,rotz/iter_no,rotz/iter_no,rotz/iter_no};
	for (int i=smart_gait_iterator;i<6;i+=2) {
	    x_robot[i]=-x_robot[i]; y_robot[i]=-y_robot[i]; z_robot[i]=-z_robot[i];
	    alpha_robot[i]=-alpha_robot[i]; beta_robot[i]=-beta_robot[i]; gamma_robot[i]=-gamma_robot[i];
	}
	for (int n=0;n<6;n++)//odczyt pozycji stopy przed ruchem
//		// robot->dynamicWorld->rec_robot_leg[n].getLastPoint(&x_foot1[n],&y_foot1[n],&z_foot1[n]);
	for (int i=0;i<iter_no;i++){
		for (int j=smart_gait_iterator;j<6;j+=2){
			x_robot[j]=(i+1)*(-x/iter_no); y_robot[j]=(i+1)*(-y/iter_no); z_robot[j]=(i+1)*(-z/iter_no);
			alpha_robot[j]=(i+1)*(-rotx/iter_no); beta_robot[j]=(i+1)*(-roty/iter_no); gamma_robot[j]=(i+1)*(-rotz/iter_no);
		}
		
		if (!robot->changePlatformRobotNeutral(x_robot,y_robot,z_robot,alpha_robot,beta_robot,gamma_robot,z_foot[smart_gait_iterator]-foot_up,smart_gait_iterator,speed,accel))//przenosimy platforme
			return false;
		else
			robot->modifyRobotPosition(x/iter_no, y/iter_no, z/iter_no, rotx/iter_no, roty/iter_no, rotz/iter_no);
		//make scan
		//SleepODE(100000);
		robot->getRotAngles(imu_rot);
	//	rpccaller_map->makeScan(-1.57, 1.57, 1, robot->robot_global_pos.getElement(1,4),robot->robot_global_pos.getElement(2,4),robot->robot_global_pos.getElement(3,4), imu_rot[0],imu_rot[1],0);
	}
	for (int n=0;n<6;n++)//odczyt pozycji stopy po ruchu
		// robot->dynamicWorld->rec_robot_leg[n].getLastPoint(&x_foot_after[n],&y_foot_after[n],&z_foot_after[n]);
	for (int n=0;n<6;n++)//obliczenie poslizgu
		foot_distance[n]=sqrt(pow(x_foot_after[n]-x_foot1[n],2)+pow(y_foot_after[n]-y_foot1[n],2)+pow(z_foot_after[n]-z_foot1[n],2));
	//odczyt aktualnej pozycji
	fprintf(fposlizg,"%f, ",foot_distance[1]+foot_distance[3]+foot_distance[5]); //zapisujemy dane wejsciowe
		
	//SleepODE(500000);
	for (int i=0;i<6;i++) z_foot[i]=0;
	if (!MoveLegsDown(smart_gait_iterator,0,0,&z_foot[0]))//nieparzyste nogi na ziemie
	  return false;
	for (int i=smart_gait_iterator;i<6;i+=2) z_foot[i]=-z_foot[i]+0.03;
	if (!robot->changeAllFootsRobot(x_foot, y_foot, z_foot, speed))//opuszczamy konczyny na dol
	  return false;
	if (!robot->PlaceFoots(0.005, smart_gait_iterator, speed))
	  return false;
//	robot->stabilizePlatform(speed, accel);
	SleepODE(100);
	robot->stabilizePlatform(0.05, accel);
	if (smart_gait_iterator == 1) //next step
	   smart_gait_iterator = 0;
	else
	   smart_gait_iterator = 1;
	return true;
}

//robot porusza sie po trajektorii zadanej
bool CGaits::executeTrajectory(float speed){
	CPunctum body, body_prev;
	CPunctum foots[6],foots_prev[6],foots_temp[6];
	char prev_foothold=0;
	for (int i=0;i<6;i++)
		legs_traj[i].getFirst(&foots_prev[i]);
	robot_platform_traj.getFirst(&body_prev);
	float dpos[6];
	CPunctum robot_pos;
	CPunctum body_offset; body_offset.setIdentity();
	CPunctum foot_offset[6];
	for (int i=0; i<6;i++) foot_offset[i].setIdentity();

	int size=robot_platform_traj.getSize()-1;
	for (int i=0;i<size;i++){
		robot_platform_traj.getNext(&body);
		for (int i=0;i<6;i++)
			legs_traj[i].getNext(&foots[i]);
		if ((foots[0].isFoothold())&&(prev_foothold==1)) {//jezeli nogi nieparzyste maja byc opuszczane
			prev_foothold=0;
			/*robot->sleepODE(100);
			robot_pos = robot->getRobotState();
			dpos[0] = body.position.getElement(1,4)-robot_pos.position.getElement(1,4);
			dpos[1] = body.position.getElement(2,4)-robot_pos.position.getElement(2,4);
			dpos[2] = body.position.getElement(3,4)-robot_pos.position.getElement(3,4);
			robot->getRotAngles(&dpos[3]);
			dpos[3] = body.orientation[0]-dpos[3];
			dpos[4] = body.orientation[1]-dpos[4];
			dpos[5] = body.orientation[2]-dpos[5];
			CPunctum delta = robot_pos.inv(robot_pos)*body;
			//if (!robot->changePlatformRobotTripod(dpos[0],dpos[1],dpos[2],dpos[3],dpos[4],dpos[5],0,speed,1))
			if (!robot->changePlatformRobotTripod(delta.position.getElement(1,4),delta.position.getElement(2,4),delta.position.getElement(3,4),dpos[3],dpos[4],dpos[5],1,speed,1))
				return false;*/
			if (!robot->PlaceFoots(0.005, 0, speed))
				return false;
			//robot->getFullRobotState(&body, foots);
		}
		else if ((foots[1].isFoothold())&&(prev_foothold==0)){//jezeli nogi parzyste maja byc opuszczane
			prev_foothold=1;
			/*robot->sleepODE(100);
			robot_pos = robot->getRobotState();
			dpos[0] = body.position.getElement(1,4)-robot_pos.position.getElement(1,4);
			dpos[1] = body.position.getElement(2,4)-robot_pos.position.getElement(2,4);
			dpos[2] = body.position.getElement(3,4)-robot_pos.position.getElement(3,4);
			robot->getRotAngles(&dpos[3]);
			dpos[3] = body.orientation[0]-dpos[3];
			dpos[4] = body.orientation[1]-dpos[4];
			dpos[5] = body.orientation[2]-dpos[5];
			CPunctum delta = robot_pos.inv(robot_pos)*body;
			//if (!robot->changePlatformRobotTripod(dpos[0],dpos[1],dpos[2],dpos[3],dpos[4],dpos[5],0,speed,1))
			if (!robot->changePlatformRobotTripod(delta.position.getElement(1,4),delta.position.getElement(2,4),delta.position.getElement(3,4),dpos[3],dpos[4],dpos[5],0,speed,1))
				return false;*/
			if (!robot->PlaceFoots(0.005, 1, speed))
				return false;
//			robot->getFullRobotState(&body, foots);
		}
		else {// w przeciwnym wypadku idz do kolejnego punktu trajektorii
			//// robot->dynamicWorld->body=body;
			//body_prev = robot->getRobotState();
			CPunctum body_real = robot->getRobotState();
			CPunctum delta = body_real.inv(body_real)*body_prev;
			CPunctum delta1 = body_real.inv(body_prev)*body_real;
		/*	for (int j=0;j<6;j++){
				if (!foots[j].isFoothold())
					foots[j]=foots[j]*delta;
			}
			//	// robot->dynamicWorld->foots[j]=foots[j];
			
			body = body*delta;*/
			if (!robot->move2GlobalPosition(body_real,body,foots_prev,foots,speed,1))
				return false;
		}
		body_prev=body;
		for (int i=0;i<6;i++)
			foots_prev[i]=foots[i];
	}
	return true;
}


//robot porusza sie po trajektorii zadanej
bool CGaits::executeTrajectoryWave(float speed){
	CPunctum body, body_prev;
	CPunctum foots[6],foots_prev[6];
	char prev_foothold[6]={1,1,1,1,1,1};
	for (int i=0;i<6;i++)
		legs_traj[i].getFirst(&foots_prev[i]);
	robot_platform_traj.getFirst(&body_prev);
	float dpos[6];
	CPunctum robot_pos;
	CPunctum body_offset; body_offset.setIdentity();
	CPunctum foot_offset[6];
	for (int i=0; i<6;i++) foot_offset[i].setIdentity();

	while (robot_platform_traj.getNext(&body)){
		for (int i=0;i<6;i++)
			legs_traj[i].getNext(&foots[i]);
		if (((foots[0].isFoothold())&&(prev_foothold[0]==0))||((foots[1].isFoothold())&&(prev_foothold[1]==0))||((foots[2].isFoothold())&&(prev_foothold[2]==0))||((foots[3].isFoothold())&&(prev_foothold[3]==0))||((foots[4].isFoothold())&&(prev_foothold[4]==0))||((foots[5].isFoothold())&&(prev_foothold[5]==0))) {//jezeli nogi nieparzyste maja byc opuszczane
			robot->sleepODE(100);
/*			robot_pos = robot->getRobotState();
			dpos[0] = body.position.getElement(1,4)-robot_pos.position.getElement(1,4);
			dpos[1] = body.position.getElement(2,4)-robot_pos.position.getElement(2,4);
			dpos[2] = body.position.getElement(3,4)-robot_pos.position.getElement(3,4);
			robot->getRotAngles(&dpos[3]);
			dpos[3] = body.orientation[0]-dpos[3];
			dpos[4] = body.orientation[1]-dpos[4];
			dpos[5] = body.orientation[2]-dpos[5];
			CPunctum delta = robot_pos.inv(robot_pos)*body;
			if (!robot->changePlatformRobotSense(delta.position.getElement(1,4),delta.position.getElement(2,4),delta.position.getElement(3,4),dpos[3],dpos[4],dpos[5],speed,1))
				return false;*/
			if (!robot->PlaceFoots(0.005, speed))
				return false;
			for (int i=0;i<6;i++) if (foots[i].isFoothold()) prev_foothold[i]=1; else prev_foothold[i]=0;
		}
		else {// w przeciwnym wypadku idz do kolejnego punktu trajektorii
			CPunctum body_real = robot->getRobotState();
			CPunctum delta = body_real.inv(body_real)*body_prev;
			CPunctum delta1 = body_real.inv(body_prev)*body_real;
		/*	for (int j=0;j<6;j++){
				if (!foots[j].isFoothold())
					foots[j]=foots[j]*delta;
			}
			// robot->dynamicWorld->foots[j]=foots[j];
			
			body = body*delta;*/
			if (!robot->move2GlobalPosition(body_real,body,foots_prev,foots,speed,1))
				return false;
			for (int i=0;i<6;i++) if (foots[i].isFoothold()) prev_foothold[i]=1; else prev_foothold[i]=0;
		}
		body_prev=body;
		for (int i=0;i<6;i++)
			foots_prev[i]=foots[i];
	}
	return true;
}

//wyciecie z zaplanowanej sciezki fragmentow wykraczajacych poza zadana odleglosc
bool CGaits::reduceTrajectory(float distance){
	CPunctum body, body_prev, feet[6];
	robot_platform_traj.getFirst(&body_prev);
	for (int i=0;i<6;i++)
		legs_traj[i].getFirst(&feet[i]);
	int iter=0,num;
	bool rest;
	while (robot_platform_traj.getNext(&body)){
		rest=true;
		for (int i=0;i<6;i++){
			legs_traj[i].getNext(&feet[i]);
			if (!feet[i].isFoothold())
				rest=false;
		}
		if (rest)
			num=iter;
		if ((rest)&&(sqrt(pow(body.position.getElement(1,4)-body_prev.position.getElement(1,4),2)+pow(body.position.getElement(2,4)-body_prev.position.getElement(2,4),2))>distance)){
			num = iter;
			break;
		}
		if (iter>robot_platform_traj.getSize())
			break;
		iter++;
	}
	robot_platform_traj.eraseLast(robot_platform_traj.getSize()-num-2);
	for (int i=0;i<6;i++)
		legs_traj[i].eraseLast(legs_traj[i].getSize()-num-2);
	return true;
}

/// connect operation
int CGaits::Connect(Crrt * tree, CrrtNode * q_new){

	return 1;
}

/// rrt connect
bool CGaits::rrtConnect(float pos_end[], float rot_end[], float distance2ground, float *map_boundaries, int shift){
	rrt_begin.erase();
	rrt_finish.erase();
//	rrt_begin.robot->dynamicWorld->ground = robot->dynamicWorld->ground;
//	rrt_finish.robot->dynamicWorld->ground = robot->dynamicWorld->ground;
	float angles[3];
	float position[3];
	CPunctum start_pos = robot->getRobotState();
	for (int i=0;i<3;i++)
		position[i]=start_pos.position.getElement(i+1,4);
	robot->getRotAngles(angles);
	CPunctum body; CPunctum foots[6];
	robot->getFullRobotState(&body, foots);
	rrt_begin.setRoot(position, angles, body, foots);

	rrt_finish.computeOrientationAndHeight(pos_end[0],pos_end[1],&pos_end[2],rot_end, 0.1);//oblicza orientacje i wysokosc platformy na koncu ruchu na podstawie uksztaltowania ternu
	body.createTRMatrix(rot_end[0], rot_end[1], rot_end[2], pos_end[0], pos_end[1], pos_end[2]);
	rrt_finish.createRobotStateFoothold(pos_end, rot_end, &body, foots,pos_end);
	rrt_finish.setRoot(pos_end, rot_end, body, foots);

	CrrtNode q_rand, q_new;
	for (int k=0;k<48000;k++){
		q_rand.randomConfig(map_boundaries[0], map_boundaries[1], map_boundaries[2], map_boundaries[3]);
		//Sleep(5000);
		if (rrt_begin.ExtendOpt(q_rand, &q_new,false,distance2ground,shift,false,position)!=0){
		//	Sleep(5000);
			if (rrt_finish.ExtendOpt(q_new, &q_rand,true,distance2ground,shift,true,position)==1){
				createPath(q_new, q_rand,shift);
				pathSmoothing();
				return true;
			}
		}
		q_rand.randomConfig(map_boundaries[0], map_boundaries[1], map_boundaries[2], map_boundaries[3]);
		//Sleep(5000);
		if (rrt_finish.ExtendOpt(q_rand, &q_new,true,distance2ground,shift,false,position)!=0){
			//Sleep(5000);
			if (rrt_begin.ExtendOpt(q_new, &q_rand,false,distance2ground,shift,true,position)==1){
				createPath(q_rand, q_new,shift);
				pathSmoothing();
				return true;
			}
		}
		if (((k>10)&&(rrt_finish.GetNodeNo()<3))||(k>500))
			return false;
	}
	return false;
}

/*
bool CGaits::rrtConnectBegin(float pos_end[], float rot_end[], float distance2ground, float *map_boundaries, int shift){
	rrt_begin.erase();
	rrt_finish.erase();
	rrt_begin.// robot->dynamicWorld->ground = // robot->dynamicWorld->ground;
	rrt_finish.// robot->dynamicWorld->ground = // robot->dynamicWorld->ground;
	float angles[3];
	float position[3];
	CPunctum start_pos = robot->getRobotState();
	for (int i=0;i<3;i++)
		position[i]=start_pos.position.getElement(i+1,4);
	robot->getRotAngles(angles);
	CPunctum body; CPunctum foots[6];
	robot->getFullRobotState(&body, foots);
	rrt_begin.setRoot(position, angles, body, foots);

	rrt_finish.computeOrientationAndHeight(pos_end[0],pos_end[1],&pos_end[2],rot_end, 0.1);//oblicza orientacje i wysokosc platformy na koncu ruchu na podstawie uksztaltowania ternu
	rrt_finish.createRobotStateFoothold(pos_end, rot_end, &body, foots);
	rrt_finish.setRoot(pos_end, rot_end, body, foots);

	CrrtNode q_rand, q_new;
	for (int k=0;k<4800;k++){

		q_rand.randomConfig(position[0]-1.2, position[0]+1.2, position[1]-1.2, position[1]+1.2);
		//Sleep(5000);
		rrt_begin.Extend(q_rand, &q_new,false,distance2ground,shift,false);
		if (sqrt(pow(q_new.pos[0]-position[0],2)+pow(q_new.pos[1]-position[1],2))>0.8) {
			createPath(q_new,shift);
				return true;
		}

		q_rand.randomConfig(pos_end[0]-1.6, pos_end[0]+1.6, pos_end[1]-1.6, pos_end[1]+1.6);

		rrt_begin.Extend(q_rand,&q_new, false,distance2ground,shift,false);
		if (sqrt(pow(q_new.pos[0]-position[0],2)+pow(q_new.pos[1]-position[1],2))>0.8) {
			createPath(q_new,shift);
				return true;
		}
	}
	return false;
}
*/
/// create path according to obtained tree
void CGaits::createPath(CrrtNode q_begin, CrrtNode q_finish, int shift){
	robot_platform_traj.setDelay(0);
	robot_platform_traj.clear();
	for (int j=0;j<6;j++)
		legs_traj[j].clear();

	//robot_platform_traj.savePositionAndOrientation(q_begin.pos[0],q_begin.pos[1],q_begin.pos[2],q_begin.rot[0],q_begin.rot[1],q_begin.rot[2]);
	CrrtNode q = q_begin;
	while (q.GetParentIndex()!=-1) {
		//robot_platform_traj.savePositionAndOrientation(q.pos[0],q.pos[1],q.pos[2],q.rot[0],q.rot[1],q.rot[2]);
		if (q.GetParentIndex()!=-1){
			for (int i=shift-1;i>-1;i--){
				for (int j=0;j<6;j++)
					legs_traj[j].savePunctum(q.legs_traj[j][i]);
				robot_platform_traj.savePunctum(q.body_traj[i]);
			}
//			robot_platform_traj.savePunctum(q.body);
		}
		q = rrt_begin.getNode(q.GetParentIndex());
	}
	robot_platform_traj.reverseOrder();
	for (int j=0;j<6;j++)
		legs_traj[j].reverseOrder();
	q = q_finish;
	for (int j=0;j<6;j++)
		legs_traj[j].savePunctum(q_begin.foots[j]);
	robot_platform_traj.savePositionAndOrientation(q_finish.pos[0],q_finish.pos[1],q_finish.pos[2],q_finish.rot[0],q_finish.rot[1],q_finish.rot[2]);
	while (q.GetParentIndex()!=-1) {
		//robot_platform_traj.savePositionAndOrientation(q.pos[0],q.pos[1],q.pos[2],q.rot[0],q.rot[1],q.rot[2]);
		if ((q.GetParentIndex()!=-1)){
			for (int i=0;i<shift;i++){
				for (int j=0;j<6;j++)
					legs_traj[j].savePunctum(q.legs_traj[j][i]);
				robot_platform_traj.savePunctum(q.body_traj[i]);
			}
//			robot_platform_traj.savePunctum(q.body);
		}
		q = rrt_finish.getNode(q.GetParentIndex());
	}
}

/// path smoothing
void CGaits::pathSmoothing(	float alpha, float beta, float alpha_leg, float beta_leg){
	CPunctum point, f_point[6];
	CPositionRecorder new_robot_platform_traj;
	CPositionRecorder new_feet[6];

	robot_platform_traj.getFirst(&point);//copy old path to the new path
	new_robot_platform_traj.savePunctum(point);
	for (int j=0;j<6;j++){
		legs_traj[j].getFirst(&f_point[j]);
		new_feet[j].savePunctum(f_point[j]);
	}
	int list_size = robot_platform_traj.getSize();
	if (list_size>0){
		for (int i=1;i<list_size;i++){
			robot_platform_traj.getNext(&point);
			new_robot_platform_traj.savePunctum(point);
			for (int j=0;j<6;j++){
				if (legs_traj[j].getNext(&point))
					new_feet[j].savePunctum(point);
			}
		}
	}

	float change=10;
	float tolerance =0.001;
	CPunctum new_point, new_prev_point, new_next_point;
	CPunctum f_new_point[6], f_new_prev_point[6], f_new_next_point[6];
	CPunctum prev_point, next_point;
	CPunctum f_prev_point[6], f_next_point[6];
	float vect[6];

	bool * can_modify = new bool[list_size];
	for (int i=0;i<list_size;i++) can_modify[i]=true;

	if (list_size>2){//modyfication of the path
		while (change>tolerance){
			change=0;
	//		for (int j=0;j<6;j++)
//				legs_traj[j].setBegin();

			robot_platform_traj.getFirst(&prev_point);
			new_robot_platform_traj.getFirst(&new_prev_point);
			robot_platform_traj.getNext(&point);
			new_robot_platform_traj.getNext(&new_point);
			for (int k=0;k<6;k++){
				legs_traj[k].getFirst(&f_prev_point[k]);
				new_feet[k].getFirst(&f_new_prev_point[k]);
				legs_traj[k].getNext(&f_point[k]);
				new_feet[k].getNext(&f_new_point[k]);
			}
			for (int i=2;i<list_size;i++){
				robot_platform_traj.getNext(&next_point);
				new_robot_platform_traj.getNext(&new_next_point);
				for (int j=0;j<3;j++){
					float ch1 = alpha*(point.position.getElement(j+1,4)-new_point.position.getElement(j+1,4))+beta*(new_next_point.position.getElement(j+1,4)+new_prev_point.position.getElement(j+1,4)-2*new_point.position.getElement(j+1,4));
					vect[j] = new_point.position.getElement(j+1,4)+ch1;
					float ch2 = alpha*(point.orientation[j]-new_point.orientation[j])+beta*(new_next_point.orientation[j]+new_prev_point.orientation[j]-2*new_point.orientation[j]);
					vect[j+3] = new_point.orientation[j]+ch2;
					change += abs(new_point.position.getElement(j+1,4)-vect[j]);
					change += abs(new_point.orientation[j]-vect[j+3]);
				}

				new_point.createTRMatrix(vect[3],vect[4],vect[5],vect[0],vect[1],vect[2]);
				for (int j=0;j<3;j++) new_point.orientation[j]=vect[j+3];

				for (int k=0;k<6;k++){
					legs_traj[k].getNext(&f_next_point[k]);
					new_feet[k].getNext(&f_new_next_point[k]);
					if (!f_point[k].isFoothold()){
						for (int j=0;j<3;j++){
							float ch1 = alpha_leg*(f_point[k].position.getElement(j+1,4)-f_new_point[k].position.getElement(j+1,4))+beta_leg*(f_new_next_point[k].position.getElement(j+1,4)+f_new_prev_point[k].position.getElement(j+1,4)-2*f_new_point[k].position.getElement(j+1,4));
							vect[j] = f_new_point[k].position.getElement(j+1,4)+ch1;
						}
						f_new_point[k].createTRMatrix(0,0,0,vect[0],vect[1],vect[2]);
						if (f_point[k].isFoothold())
							f_new_point[k].setFoothold(true);
					}
				}

				if (1){
					new_robot_platform_traj.updatePreviousElement(&new_point);
					for (int k=0;k<6;k++){
						if (this->verifyAchievability(new_point,f_new_point[k],k)){
							new_feet[k].updatePreviousElement(&f_new_point[k]);
						}
					}
				}
									/*
					if ((rrt_begin.robot->computeKinematicMargin(new_point,f_new_point)>0.05)){//&&(rrt_begin.robot->computeStabilityMargin(new_point,f_new_point)>0.05)){
						new_robot_platform_traj.updatePreviousElement(&new_point);
						for (int k=0;k<6;k++)
							new_feet[k].updatePreviousElement(&f_new_point[k]);
					}
					else
						can_modify[i]=false;*/
				new_prev_point=new_point;
				new_point=new_next_point;
				prev_point=point;
				point=next_point;
				for (int k=0;k<6;k++){
					f_new_prev_point[k]=f_new_point[k];
					f_new_point[k]=f_new_next_point[k];
					f_prev_point[k]=f_point[k];
					f_point[k]=f_next_point[k];
				}
			}
		}

		robot_platform_traj.clear();//copy new path to the old path
		for (int j=0;j<6;j++)
			legs_traj[j].clear();
		if (new_robot_platform_traj.getSize()>0){
			new_robot_platform_traj.getFirst(&point);
			robot_platform_traj.savePunctum(point);
			for (int j=0;j<6;j++){
				new_feet[j].getFirst(&f_point[j]);
				legs_traj[j].savePunctum(f_point[j]);
			}
			for (int j=1;j<list_size;j++){
				new_robot_platform_traj.getNext(&point);
				robot_platform_traj.savePunctum(point);
				for (int k=0;k<6;k++){
					new_feet[k].getNext(&f_point[k]);
					legs_traj[k].savePunctum(f_point[k]);
				}
			}
		}
	}
}

/// create path according to obtained tree
void CGaits::createPath(CrrtNode q_begin, int shift){
	robot_platform_traj.setDelay(0);
	robot_platform_traj.clear();
	for (int j=0;j<6;j++)
		legs_traj[j].clear();

	//robot_platform_traj.savePositionAndOrientation(q_begin.pos[0],q_begin.pos[1],q_begin.pos[2],q_begin.rot[0],q_begin.rot[1],q_begin.rot[2]);
	CrrtNode q = q_begin;
	while (q.GetParentIndex()!=-1) {
		//robot_platform_traj.savePositionAndOrientation(q.pos[0],q.pos[1],q.pos[2],q.rot[0],q.rot[1],q.rot[2]);
		if (q.GetParentIndex()!=-1){
			for (int i=shift-1;i>-1;i--){
				for (int j=0;j<6;j++)
					legs_traj[j].savePunctum(q.legs_traj[j][i]);
				robot_platform_traj.savePunctum(q.body_traj[i]);
			}
		}
		q = rrt_begin.getNode(q.GetParentIndex());
	}
	robot_platform_traj.reverseOrder();
	for (int j=0;j<6;j++)
		legs_traj[j].reverseOrder();
}

///finds path using a-star algorithm
int CGaits::astarFind(std::vector< void* > *path, float * init_pos, float * dest_pos){
	return rrt_begin.astarFind(path, init_pos, dest_pos);
}