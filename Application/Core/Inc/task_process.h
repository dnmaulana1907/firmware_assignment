/*
 * task_process.h
 *
 *  Created on: Feb 8, 2026
 *      Author: danimaulana
 */

#ifndef INC_TASK_PROCESS_H_
#define INC_TASK_PROCESS_H_

typedef enum
{

	STATE_INIT,
	STATE_IDLE,
	STATE_READ,
	STATE_TRANSMIT,
	STATE_ERROR,
	STATE_STOP

}StateMAchine_t;


void task_update_process(void);
void task_monitoring_process(void);
void task_telemetry_process(void);
void SensorTimerCallback(void const * argument);
#endif /* INC_TASK_PROCESS_H_ */
