/*
 * vector.c
 *
 *  Created on: 5 mei 2017
 *      Author: Mark
 */

#include "main.h"
#include "vars.h"

#include "math.h"
#include "stdint.h"
#include "vector.h"

static void co2vecz(coord_t pos, vectors_t * vector);
static void vecz2co(coord_t * coord, vectors_t * vector);
static void mirror_vector(coord_t sunpos, coord_t * mirror, coord_t reflection);
static void reflection_vector(coord_t sun, coord_t mirrorpos, coord_t * reflection);


/****************************************************************************
 *  Berekend de positie van het doel
 *  Input:
 *  - Hardware info
 *  - Positie van de zon
 *  - positie van de spiegel
 *  Output
 *  - integers van target in stappen
 *
 */
void calc_target_pos(hw_info_t *hw_info, coord_t sunpos, motorpos_t mirror, motorpos_t * target)
{
	coord_t mirror_f; // motor position in real azimuth and elevation
	coord_t target_f; // reglection_f position in real azimuth and elevation

	mirror_f.azimuth = (float)(hw_info->hw_offset.x + mirror.x) / (float)hw_info->steps.x;     // miror position unit:decrees
	if (mirror_f.azimuth > 360)
		mirror_f.azimuth = mirror_f.azimuth - 360;
	mirror_f.elevation = (float) (hw_info->hw_offset.y + mirror.y) /(float) hw_info->steps.y;

	reflection_vector(sunpos, mirror_f, &target_f);

	target->x = target_f.azimuth * hw_info->steps.x;
	target->y = target_f.elevation * hw_info->steps.y;
}

/****************************************************************************
 *  Berekent de positie van de spiegel voor een doel
 *  Input:
 *  - Hardware info
 *  - Positie van de zon
 *  - doelpositie
 *  Output
 *  - x en y waardes van spiegel in stappen
 *
 */
void calc_mirror_pos(hw_info_t *hw_info, coord_t sunpos, motorpos_t * mirror, motorpos_t target)
{
	coord_t target_f;
	coord_t mirror_f;

	target_f.azimuth = (float) (target.x) / (float) hw_info->steps.x;
	target_f.elevation = (float) (target.y) / (float)hw_info->steps.y;

	mirror_vector(sunpos, &mirror_f, target_f);

	if ((hw_info->home_location.latitude < 0) && (mirror_f.azimuth < 180))
		mirror->x = (float) (((360 + mirror_f.azimuth) * hw_info->steps.x) - hw_info->hw_offset.x); // quadrant 1: 360-450º
	else
		mirror->x = (float) ((mirror_f.azimuth * hw_info->steps.x) - hw_info->hw_offset.x);

	mirror->y = (float) ((mirror_f.elevation * hw_info->steps.y) - hw_info->hw_offset.y);

}


/****************************************************************************
 * R= 1/2(R-L)+L
 *
 * Calculation of the mirror coordinates
 */
static void mirror_vector(coord_t sunpos, coord_t * mirror, coord_t reflection)
{
	vectors_t L; // light source
	vectors_t N; // mirror
	vectors_t R; // target
	vectors_t * p_N;

	p_N = &N;

	co2vecz(sunpos, &L);
	co2vecz(reflection, &R);

	N.x = 0.5 * (R.x - L.x) + L.x;
	N.y = 0.5 * (R.y - L.y) + L.y;
	N.z = 0.5 * (R.z - L.z) + L.z;

	vecz2co(mirror, p_N);
}

/****************************************************************************
 * R= 2(N.L).N-L
 *
 * Calculation of the reflection
 */
static void reflection_vector(coord_t sun, coord_t mirrorpos, coord_t * reflection)
{
	static vectors_t L; // light source
	static vectors_t N; // mirror
	static vectors_t R; // target
	static vectors_t * p_R; // target
	float w;

	p_R = &R;

	co2vecz(sun, &L);
	co2vecz(mirrorpos, &N);

	w = 2 * ((L.x * N.x) + (L.y * N.y) + (L.z * N.z)); // R = 2 * (Lx * nx + Ly * ny + Lz * nz) * n - L

	R.x = (w * N.x) - L.x;
	R.y = (w * N.y) - L.y;
	R.z = (w * N.z) - L.z;

	vecz2co(reflection, p_R);
}



/****************************************************************************
 * coordinate into vector
 */
static void co2vecz(coord_t pos, vectors_t * vector)
{
	float w =0;
	if (pos.azimuth >= 45 && pos.azimuth <= 135)
	{
		w = cos(raddeg * pos.elevation); // flat length of vector
		vector->z = sin(raddeg * (90 - pos.azimuth)) * w;
		vector->x = cos(raddeg * (90 - pos.azimuth)) * w;
	}
	else if (pos.azimuth >= 135 && pos.azimuth <= 225)
	{
		w = cos(raddeg * pos.elevation); // flat length of vector
		vector->x = sin(raddeg * (180 - pos.azimuth)) * w;
		vector->z = -cos(raddeg * (180 - pos.azimuth)) * w;
	}
	else if (pos.azimuth >= 225 && pos.azimuth <= 315)
	{
		w = cos(raddeg * pos.elevation); // flat length of vector
		vector->z = -sin(raddeg * (270 - pos.azimuth)) * w;
		vector->x = -cos(raddeg * (270 - pos.azimuth)) * w;
	}
	else if (pos.azimuth >= 315 || pos.azimuth <= 45)
	{
		w = cos(raddeg * pos.elevation); // flat length of vector
		vector->x = sin(raddeg * (pos.azimuth)) * w;
		vector->z = cos(raddeg * (pos.azimuth)) * w;
	}
	vector->y = tan(raddeg * pos.elevation) * w;
  //printf("calc: az=%.6f el=%.6f x=%.6f  y=%.6f z=%.6f \n\r",pos.azimuth,pos.elevation,vector->x,vector->y,vector->z);
}

/****************************************************************************
 * vector into coordinate
 *
 */
static void vecz2co(coord_t * coord, vectors_t * vector)
{
	if (vector->x >= 0 && vector->z >= 0) // q1->
	{
		coord->azimuth = degrad * atan(vector->x / vector->z);
		coord->elevation= degrad * atan(vector->y / sqrt((vector->x * vector->x) + (vector->z * vector->z))); // length of vector
	}
	else if (vector->x >= 0 && vector->z <= 0) // q2
	{
		coord->azimuth = 180 + (degrad * atan(vector->x / vector->z));
		coord->elevation = degrad * atan(vector->y / sqrt((vector->x * vector->x) + (vector->z * vector->z))); // length of vector
	}
	else if (vector->x <= 0 && vector->z <= 0) // q3
	{
		coord->azimuth = 180 + degrad * atan(vector->x / vector->z);
		coord->elevation = degrad * atan(vector->y / sqrt((vector->x * vector->x) + (vector->z * vector->z))); // length of vector
	}
	else if (vector->x <= 0 && vector->z >= 0) // q4
	{
		coord->azimuth = 360 + degrad * atan(vector->x / vector->z);
		coord->elevation = degrad * atan(vector->y / sqrt((vector->x * vector->x) + (vector->z * vector->z))); // length of vector
	}
}
