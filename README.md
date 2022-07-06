# Wiegand C library

This is a platform-independent C library for reading data from Wiegand devices (keypads, card readers, biometric readers, ...).    
It is agnostic to data format (parity bits, card data, facility code, etc. are not interpreted) and provides for noise detection and filtering.

It can be used on any platform with one or more Wiegand-compatible interfaces, i.e. two TTL (5V level) lines (GPIO pins), normally pulled up (internally or with external resistors), where you connect the DATA0 and DATA1 lines of the Wiegand device(s). When data is transmitted those lines are pulled down (set low) by the Wiegand device.

To integrate the library in your code go through the following steps.

Include it:
```
#include "wiegand.h"
```

Define a `wiegandItf` instance for each Wiegand interface you want to have:
```
wiegandItf w;
```

Implement `wiegandMicros()` according to your platform, returning a monotonic timestamp in microseconds (µs) as `unsigned long`, e.g.:
```
unsigned long wiegandMicros() {
  return getMicros();
}
```

For each Wiegand interface, call `wiegandSetup()` once, to initialize it:
```
unsigned long pulseIntervalMin_usec = 700;
unsigned long pulseIntervalMax_usec = 3000;
unsigned long pulseWidthMin_usec = 10;
unsigned long pulseWidthMax_usec = 150;

void wiegandSetup(&w, pulseIntervalMin_usec, pulseIntervalMax_usec, pulseWidthMin_usec, pulseWidthMax_usec);
```
the `pulseXXX_usec` parameters are used to discern noise on the data lines from actual data and discard it:
- `pulseIntervalMin_usec`: minimum interval between pulses accepted, in µs;
- `pulseIntervalMax_usec`: maximum interval between pulses accepted, in µs, after which the bits sequence transmission is considered complete;
- `pulseWidthMin_usec`: minimum bit pulse width accepted, in µs;
- `pulseWidthMax_usec`: maximum bit pulse width accepted, in µs.

Values in the example above correspond to a wide range in which most Wiegand devices fall. Set stricter ranges to fit your specific device characteristics and get better noise filtering.

Monitor the GPIO lines of each Wiegand interface and call `wiegandOnData()` when a state change is detected:
```
wiegandOnData(&w, line, gpioRead(pin));
```
where `w` is the `wiegandItf` instance the line belongs to, `line` is `0` or `1` respectively for DATA0 or DATA1 and the last parameter is set to the current state of the line.

If using polling for monitoring the lines, make sure to poll them with a maximum interval smaller than the smallest `pulseXXX_usec` parameter set with `wiegandSetup()` on the same interface instance.

If using interrupts, make sure to detect both falling and rising edges.

Periodically call `wiegandGetData()` to retrieve incoming data:
```
uint64_t data;
int bits = wiegandGetData(&w, &data);
```
When data is available, a positive value is returned, corresponding to the number of bits received, and the corresponding data is stored in the lower bits of the `uint64_t` variable passed to the function as a pointer. Subsequent calls will return `0` until new data is available. If `-1` is returned it indicates that a transmission is in progress, so data will shortly be available. 

Use `wiegandGetNoise()` to check for noise on the interface and possibly adjust the setup parameters:
```
int noise = wiegandGetNoise(&w);
```
The returned value corresponds to the latest noise event detected on the interface, according to the table below. After a positive value is returned, subsequent calls will return `0` until a new event occurs.

|Noise value|Description|
|:---------:|-----------|
10 | Fast pulses on lines <sup>[Note 1](#note-1)</sup>
11 | Pulses interval too short
12/13 | Concurrent movement on both DATA0/DATA1 lines <sup>[Note 2](#note-2)</sup>
14 | Pulse too short
15 | Pulse too long

<a name="note-1">**Note 1**</a>: Other noise values have higher priority, i.e. `10` is returned only if there are no other previous events not returned yet. This effectively indicates that `wiegandOnData()` is being called subsequently with the GPIO state parameter unchanged (which might happen when using interrupts upon fast pulses on the line) or the code is calling the function not only when the state has changed, e.g. it is called periodically with the current state of the line. This is a functionally acceptable implementation, in this case `10` corresponds to no noise.

<a name="note-2">**Note 2**</a>: Possibly indicates a short circuit between the two lines.

## Examples

For platform specific implementation examples check the [examples directory](./examples).

For a simplified Arduino library wrapping this one, check:
https://github.com/sfera-labs/arduino-wiegand

