#ifndef __SENSORS__
#define __SENSORS__

#define DUMP_ALL_SENSORS 133

#define SENSOR_NOTIFIER 0
#define WAIT_REQUEST 1
#define SENSORS_DUMP_REQUEST 2
#define SENSORS_CREATE_NOTIFIER 3
#define SENSOR_SERVER_NAME "SS"

void SensorNotifier();
void SensorCourier();
void SensorServer();
void SensorNotifierNoCourier();
int waitForSensors( char *sensors, int len, int timeOut );
int sensorToInt (char module, int num);
int getSensorComplement (int num);

int getLatestSensors ( char current_sensor_state[10]) ;

#endif
