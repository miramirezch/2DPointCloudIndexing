#pragma once
struct ShazamHashParameters
{
	unsigned Delta;
	unsigned DelayX;
	unsigned DeltaX;
	unsigned DeltaY;
	unsigned CombinationLimit;

	ShazamHashParameters(unsigned delta, unsigned delayX, unsigned deltaX, unsigned deltaY, unsigned limit) :Delta{ delta }, DelayX{ delayX }, DeltaX{ deltaX }, DeltaY{ deltaY }, CombinationLimit{ limit } {}
};