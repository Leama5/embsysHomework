#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "TimeParser.h"

// time format: HHMMSS (6 characters)
int time_parse(char *time) {
	
	// Check that string is not null
	if (time == NULL) {
		return TIME_ARRAY_ERROR;
	}

	// 1. Tarkistetaan pituus (pitää olla tasan 6 merkkiä)
	if (strlen(time) != 6) {
		return TIME_LEN_ERROR; // -1
	}

	// 2. Tarkistetaan, että kaikki merkit ovat numeroita
	for (int i = 0; i < 6; i++) {
		if (!isdigit((unsigned char)time[i])) {
			return TIME_NUMBER_ERROR; // -5
		}
	}

	// Otetaan kopiot paloista turvallisesti TUHOAMATTA alkuperäistä merkkijonoa
	char hour_str[3] = { time[0], time[1], '\0' };
	char min_str[3]  = { time[2], time[3], '\0' };
	char sec_str[3]  = { time[4], time[5], '\0' };

	int hours   = atoi(hour_str);
	int minutes = atoi(min_str);
	int seconds = atoi(sec_str);

	// 3. Rajatarkistukset
	if (seconds >= 0 && seconds <= 59 && minutes >= 0 && minutes <= 59 && hours >= 0 && hours <= 23) {
		
		// Lasketaan kokonaissekunnit (tunnit * 3600 + minuutit * 60 + sekunnit)
		int total_seconds = (hours * 3600) + (minutes * 60) + seconds;
		
		if (total_seconds == 0) {
			return TIME_ZERO_ERROR; // -4
		}
		
		return total_seconds;
	} 
	else {
		return TIME_VALUE_ERROR; // -3
	}
}
