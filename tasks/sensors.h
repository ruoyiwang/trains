#ifndef __SENSORS__
#define __SENSORS__

#define DUMP_ALL_SENSORS 133

#define SENSOR_NOTIFIER 0
#define WAIT_REQUEST 1
#define SENSORS_DUMP_REQUEST 2
#define SENSOR_SERVER_NAME "Sensor Server"

void SensorNotifier();
void SensorCourier();
void SensorServer();
int waitForSensor( int sensor );
int getLatestSensors ( char current_sensor_state[10]);

#endif
