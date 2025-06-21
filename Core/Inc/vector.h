/*
 * vector.h
 *
 *  Created on: 5 mei 2017
 *      Author: Mark
 */

#ifndef VECTOR_H_
#define VECTOR_H_

//#include "stdint.h"

#define degrad 57.295779513 /* converts from radians to degrees */
#define raddeg 0.0174532925 /* converts from degrees to radians */

//const float degrad = 57.295779513; /* converts from radians to degrees */
//const float raddeg = 0.0174532925; /* converts from degrees to radians */

typedef struct
{
	float x;
	float y;
	float z;
}vectors_t;



//  Berekend de positie van het doel
void calc_target_pos(hw_info_t *hw_info, coord_t sunpos, motorpos_t mirror, motorpos_t * target);


// Berekend de positie van de spiegel voor een doel
void calc_mirror_pos(hw_info_t *hw_info, coord_t sunpos, motorpos_t * mirror, motorpos_t target);


#endif /* VECTOR_H_ */
