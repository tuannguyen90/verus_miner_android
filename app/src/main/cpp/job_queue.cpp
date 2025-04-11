#include "job_queue.h"
#include "iostream"
#include "globals.h"

void JobQueue::addJob(const work& job) {
	if (jobs.size() >= maxJobs) {
		std::cout << "Queue full, removing oldest job: " << jobs.front().job_id << std::endl;
		jobs.pop_front();

		// Dừng job hiện tại do đã quá lâu
		stop_threads.store(true);
		std::cout << "\nDung job do qua lau!\n";
	}
	jobs.push_back(job);
	std::cout << "Added job: " << job.job_id << std::endl;
	char buf[128];
	snprintf(buf, sizeof(buf), "Added job: %s", job.job_id);
    add_status_line(buf);
//	printJobs();
}

work JobQueue::getJob() {
	if (jobs.empty()) {
		std::cout << "No job available.\n";
		work emptyWork;
//		strncpy_s(emptyWork.job_id, "", sizeof(emptyWork.job_id) - 1);
		strncpy(emptyWork.job_id, "", sizeof(emptyWork.job_id));
		emptyWork.job_id[sizeof(emptyWork.job_id) - 1] = '\0';
		return emptyWork;
	}

	work job = jobs.back();
	jobs.pop_back();
	std::cout << "Retrieved job: " << job.job_id << std::endl;
	return job;
}

void JobQueue::printJobs() {
	std::cout << "Current jobs: ";
	for (const auto& job : jobs) {
		std::cout << job.job_id << " & ";
	}
	std::cout << std::endl;
}

size_t JobQueue::size() {
	return jobs.size();
}
