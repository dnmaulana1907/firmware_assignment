/*
 * lg290p_gps.c
 *
 *  Created on: Feb 10, 2026
 *      Author: danimaulana
 */
#include "lg290p_gps.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t lg290p_validatechecksum(const char *nmea_str) {
    uint8_t calculated_xor = 0;
    const char *start = strchr(nmea_str, '$');
    const char *end = strchr(nmea_str, '*');

    if (start == NULL || end == NULL) return 0;

    // XOR
    for (const char *p = start + 1; p < end; p++) {
        calculated_xor ^= *p;
    }

    uint8_t actual_checksum = (uint8_t)strtol(end + 1, NULL, 16);
    return (calculated_xor == actual_checksum);
}

void lg290p_parse_rmc(char *nmea_str, LG290P_RMC_t *data_out)
{
	char temp_buf[128];
	    strncpy(temp_buf, nmea_str, sizeof(temp_buf));

	    char *token;
	    int field = 0;

	    token = strtok(temp_buf, ",");
	    while (token != NULL) {
	        switch (field) {
	            case 0: // TalkerID & Formatter
	                strncpy(data_out->talker_id, &token[1], 2);
	                data_out->talker_id[2] = '\0';
	                break;
	            case 1: // UTC Time
	                strncpy(data_out->utc_time, token, 10);
	                break;
	            case 2: // Status
	                data_out->status = token[0];
	                break;
	            case 3: // Latitude
	                data_out->latitude = atof(token);
	                break;
	            case 4: // N/S
	                data_out->ns_indicator = token[0];
	                break;
	            case 5: // Longitude
	                data_out->longitude = atof(token);
	                break;
	            case 6: // E/W
	                data_out->ew_indicator = token[0];
	                break;
	            case 7: // SOG
	                data_out->speed_knot = atof(token);
	                break;
	            case 8: // COG
	                data_out->course_deg = atof(token);
	                break;
	            case 9: // Date
	                strncpy(data_out->date, token, 6);
	                break;
	            case 12: // Mode Indicator
	                data_out->mode_indicator = token[0];
	                break;
	        }
	        token = strtok(NULL, ",");
	        field++;
	    }
}
