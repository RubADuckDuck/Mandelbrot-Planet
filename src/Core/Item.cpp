#include "Core/Item.h"

Item* Item2Probability::RandomRollDrop() {
	float totalProbability = 0.0f;

	for (const auto& pair : this->item2ProbMap) {
		totalProbability += pair.second;
	}

	float randomValue = static_cast<float>(rand()) / RAND_MAX * totalProbability;
	float cumulativeProbability = 0.0f;

	for (const auto& pair : this->item2ProbMap) {
		cumulativeProbability += pair.second;
		if (randomValue <= cumulativeProbability) {
			return pair.first;
		}
	}

	return nullptr; // Default case if no item is selected
}