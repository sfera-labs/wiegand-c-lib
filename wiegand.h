/*
  wiegand.h - Wiegand C library

    Copyright (C) 2022 Sfera Labs S.r.l. - All rights reserved.
    For information, see:
    http://www.sferalabs.cc/

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  See file LICENSE.txt for further informations on licensing terms.
*/

#ifndef WIEGAND_LIB_H
#define WIEGAND_LIB_H

#include <stdint.h>

#define WIEGAND_MAX_BITS 64

typedef volatile struct WiegandItf {
	int d0;
	int d1;
	int activeLine;
	unsigned long pulseIntervalMin_usec;
	unsigned long pulseIntervalMax_usec;
	unsigned long pulseWidthMin_usec;
	unsigned long pulseWidthMax_usec;
	uint64_t data;
	int bitCount;
	int noise;
	unsigned long lastBitTs_usec;
} wiegandItf;

#ifdef __cplusplus
extern "C" {
#endif

unsigned long wiegandMicros();

void wiegandSetup(wiegandItf* w,
                  unsigned long pulseIntervalMin_usec,
                	unsigned long pulseIntervalMax_usec,
                	unsigned long pulseWidthMin_usec,
                	unsigned long pulseWidthMax_usec);
void wiegandOnData(wiegandItf* w, int line, int val);
int wiegandGetData(wiegandItf* w, uint64_t* data);
int wiegandGetNoise(wiegandItf* w);

#ifdef __cplusplus
}
#endif

#endif
