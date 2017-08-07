#pragma once
struct ShazamHashParameters
{
	int Delta;
	int DelayX;
	int DeltaX;
	int DeltaY;
	int CombinationLimit;

	ShazamHashParameters(int delta, int delayX, int deltaX, int deltaY, int limit) :Delta{ delta }, DelayX{ delayX }, DeltaX{ deltaX }, DeltaY{ deltaY }, CombinationLimit{ limit } {}
};

