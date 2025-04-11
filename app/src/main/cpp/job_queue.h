#ifndef JOBQUEUE_H
#define JOBQUEUE_H

#include <deque>
#include <string>
#include "miner.h"

class JobQueue {
private:
	std::deque<work> jobs;
	const size_t maxJobs = 6;

public:
	void addJob(const work& job);
	work getJob();
	void printJobs();
	size_t size();
};

#endif