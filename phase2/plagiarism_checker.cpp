#include "plagiarism_checker.hpp"
#include <mutex>
// You should NOT add ANY other includes to this file.
// Do NOT add "using namespace std;".

// TODO: Implement the methods of the plagiarism_checker_t class
plagiarism_checker_t::plagiarism_checker_t(void) {
    worker = std::thread(&plagiarism_checker_t::thread_loop, this);
}

plagiarism_checker_t::plagiarism_checker_t(
    std::vector<std::shared_ptr<submission_t>> __submissions) {
    // TODO
    worker = std::thread(&plagiarism_checker_t::thread_loop, this);
}

plagiarism_checker_t::~plagiarism_checker_t(void) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    mutex_condition.notify_one();
    if (worker.joinable())
        worker.join();
}

void plagiarism_checker_t::thread_loop() {
    while (true) {
        std::shared_ptr<submission_t> submission;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            mutex_condition.wait(lock,
                                 [this]() { return !jobs.empty() || stop; });
            if (stop && jobs.empty())
                return;
            submission = std::move(jobs.front());
            jobs.pop();
        }
        check_submission(submission);
    }
}

void plagiarism_checker_t::add_submission(
    std::shared_ptr<submission_t> __submission) {
    const std::chrono::time_point<std::chrono::system_clock> timestamp =
        std::chrono::system_clock::now();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if (stop)
            return;
        jobs.push(std::move(__submission));
    }
    mutex_condition.notify_one();
}

// End TODO
