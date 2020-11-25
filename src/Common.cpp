#include "Common.hpp"

Timer::Timer(float timeLimit) :
	timeLimit(timeLimit), startTime(std::chrono::high_resolution_clock::now()) {

}

bool Timer::isTimeLeft() const {
	auto now = std::chrono::high_resolution_clock::now();
	return std::chrono::duration<float>(now - startTime).count() * 1000 < timeLimit;
}
