#pragma once

struct FingerPrint
{
	FingerPrint(int y1, int y2, int dx) :Y1{ y1 }, Y2{ y2 }, DX{ dx } {}
	int Y1;
	int Y2;
	int DX;
};
