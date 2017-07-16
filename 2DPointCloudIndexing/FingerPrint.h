#pragma once

struct FingerPrint
{
	FingerPrint(int y1, int y2, int dx) :Y1{ y1 }, Y2{ y2 }, DX{ dx } {}
	int Y1;
	int Y2;
	int DX;
};

/*
struct FingerPrint
{
FingerPrint(unsigned y1, unsigned y2, unsigned dx) :Y1{ y1 }, Y2{ y2 }, DX{ dx } {}
unsigned Y1;
unsigned Y2;
unsigned DX;
};
*/
