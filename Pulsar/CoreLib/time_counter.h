#pragma once

#include <ctime>

struct time_counter_t
{
private:
	clock_t start_time_ = 0;
	clock_t count_ = 0;
public:
	void start() { this->start_time_ = ::clock(); }

	clock_t end() { return this->count_ = current(); }
	float end_sec() { return this->count_ = current() / 1000.f; }

	clock_t current() const { return ::clock() - this->start_time_; }
	float current_sec() const { return current() / 1000.f; }

	clock_t time() const { return this->count_; }
	float time_sec() const { return this->count_ / 1000.f; }
};
