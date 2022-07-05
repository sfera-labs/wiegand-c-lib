/*
	wiegand.c - Wiegand C library

		Copyright (C) 2022 Sfera Labs S.r.l. - All rights reserved.
		For information, see:
		http://www.sferalabs.cc/

	This code is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.
	See file LICENSE.txt for further informations on licensing terms.
*/

#include "wiegand.h"

static void _wiegandReset(wiegandItf* w) {
	w->data = 0;
	w->bitCount = 0;
	w->activeLine = -1;
	w->d0 = -1;
	w->d1 = -1;
}

void wiegandSetup(wiegandItf* w,
									unsigned long pulseIntervalMin_usec,
									unsigned long pulseIntervalMax_usec,
									unsigned long pulseWidthMin_usec,
									unsigned long pulseWidthMax_usec) {
	w->pulseIntervalMin_usec = pulseIntervalMin_usec;
	w->pulseIntervalMax_usec = pulseIntervalMax_usec;
	w->pulseWidthMin_usec = pulseWidthMin_usec;
	w->pulseWidthMax_usec = pulseWidthMax_usec;
	_wiegandReset(w);
}

int wiegandGetData(wiegandItf* w, uint64_t* data) {
	int bits;
	unsigned long now, diff;

	now = wiegandMicros();
	diff = now - w->lastBitTs_usec;
	if (diff <= w->pulseIntervalMax_usec) {
		return -1;
	}
	*data = w->data;
	bits = w->bitCount;
	w->data = 0;
	w->bitCount = 0;
	return bits;
}

int wiegandGetNoise(wiegandItf* w) {
	int noise;
	noise = w->noise;
	w->noise = 0;
	return noise;
}

void wiegandOnData(wiegandItf* w, int line, int val) {
	unsigned long now, diff;
	volatile int *d;

	now = wiegandMicros();
	d = line == 0 ? &w->d0 : &w->d1;

	if (*d == val) {
		// didn't change state, maybe a fast pulse
		if (w->noise == 0) {
			w->noise = 10;
		}
		return;
	}

	*d = val;

	if (val == 0) {
		if (w->bitCount != 0) {
			diff = now - w->lastBitTs_usec;

			if (diff < w->pulseIntervalMin_usec) {
				// pulse too early
				w->noise = 11;
				_wiegandReset(w);
				return;
			}

			if (diff > w->pulseIntervalMax_usec) {
				w->data = 0;
				w->bitCount = 0;
			}
		}

		if (w->activeLine != -1) {
			// there's movement on both lines
			w->noise = 12;
			_wiegandReset(w);
			return;
		}

		w->activeLine = line;
		w->lastBitTs_usec = now;

	} else {
		if (w->activeLine != line) {
			// there's movement on both lines or previous noise
			w->noise = 13;
			_wiegandReset(w);
			return;
		}

		w->activeLine = -1;

		if (w->bitCount >= WIEGAND_MAX_BITS) {
			return;
		}

		diff = now - w->lastBitTs_usec;

		if (diff < w->pulseWidthMin_usec) {
			// pulse too short
			w->noise = 14;
			_wiegandReset(w);
			return;
		}

		if (diff > w->pulseWidthMax_usec) {
			// pulse too long
			w->noise = 15;
			_wiegandReset(w);
			return;
		}

		w->data <<= 1;
		w->data |= line;
		w->bitCount++;
	}
}
