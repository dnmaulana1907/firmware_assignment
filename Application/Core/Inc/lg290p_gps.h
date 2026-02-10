/*
 * lg290p_gps.h
 *
 *  Created on: Feb 10, 2026
 *      Author: danimaulana
 */

#ifndef INC_LG290P_GPS_H_
#define INC_LG290P_GPS_H_

#include <stdint.h>

typedef struct {
    char talker_id[3];
    char utc_time[11];
    char status;
    double latitude;
    char ns_indicator;
    double longitude;
    char ew_indicator;
    float speed_knot;
    float course_deg;
    char date[7];
    char mode_indicator;
} LG290P_RMC_t;


uint8_t lg290p_validatechecksum(const char *nmea_str);
void lg290p_parse_rmc(char *nmea_str, LG290P_RMC_t *data_out);

#endif /* INC_LG290P_GPS_H_ */
