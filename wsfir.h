/*
	Windowed Sinc FIR Generator
	Bob Maling (BobM.DSP@gmail.com)
	Contributed to musicdsp.org Source Code Archive
	Last Updated: April 12, 2005

	Usage:
		Lowpass:	wsfirLP(h, N, WINDOW, fc)
		Highpass:	wsfirHP(h, N, WINDOW, fc)
		Bandpass:	wsfirBP(h, N, WINDOW, fc1, fc2)
		Bandstop:	wsfirBS(h, N, WINDOW, fc1, fc2)

	where:
		h (double[N]):	filter coefficients will be written to this array
		N (int):		number of taps
		WINDOW (int):	Window (W_BLACKMAN, W_HANNING, or W_HAMMING)
		fc (double):	cutoff (0 < fc < 0.5, fc = f/fs)
						--> for fs=48kHz and cutoff f=12kHz, fc = 12k/48k => 0.25
		
		fc1 (double):	low cutoff (0 < fc < 0.5, fc = f/fs)
		fc2 (double):	high cutoff (0 < fc < 0.5, fc = f/fs)


	Windows included here are Blackman, Hanning, and Hamming. Other windows	can be
	added by doing the following:
		1. "Window type constants" section: Define a global constant for the new window.
		2. wsfirLP() function: Add the new window as a case in the switch() statement.
		3. Create the function for the window
		   
		   For windows with a design parameter, such as Kaiser, some modification
		   will be needed for each function in order to pass the parameter.
*/
#ifndef WSFIR_H
#define WSFIR_H

#include <math.h>

// Function prototypes
void wsfirLP(double h[], const int &N, const int &WINDOW, const double &fc);
void wsfirHP(double h[], const int &N, const int &WINDOW, const double &fc);
void wsfirBS(double h[], const int &N, const int &WINDOW, const double &fc1, const double &fc2);
void wsfirBP(double h[], const int &N, const int &WINDOW, const double &fc1, const double &fc2);
void genSinc(double sinc[], const int &N, const double &fc);
void wBlackman(double w[], const int &N);
void wHanning(double w[], const int &N);
void wHamming(double w[], const int &N);

// Window type contstants
const int W_BLACKMAN = 1;
const int W_HANNING = 2;
const int W_HAMMING = 3;

#endif