#include "plagiarism_checker.hpp"
#include <mutex>
// You should NOT add ANY other includes to this file.
// Do NOT add "using namespace std;".

// TODO: Implement the methods of the plagiarism_checker_t class
plagiarism_checker_t::plagiarism_checker_t(void) {
    stop = false;
    worker = std::thread(&plagiarism_checker_t::thread_loop, this);
}

plagiarism_checker_t::plagiarism_checker_t(
    std::vector<std::shared_ptr<submission_t>> __submissions) {
    stop = false;
    const std::chrono::time_point<std::chrono::system_clock> timestamp =
        std::chrono::system_clock::now();
    tokenizer_t tokeni1(__submissions[0]->codefile);
    for (auto s : __submissions)
        submissions.push_back({s, timestamp, tokeni1.get_tokens(), true});
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

// The thread loop is the function run by the thread. It feeds the elements of
// the jobs queue sequentially into the check_submission function.
void plagiarism_checker_t::thread_loop() {
    while (true) {
        exsubmission_t submission;
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

// Pushes a new submission with the appropriate timestamp to the jobs queue.
void plagiarism_checker_t::add_submission(
    std::shared_ptr<submission_t> __submission) {
    const std::chrono::time_point<std::chrono::system_clock> timestamp =
        std::chrono::system_clock::now();

    // Uses the mutex to lock the jobs queue while add_submission is modifying
    // it to avoid data races.
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if (stop)
            return;
        jobs.push(exsubmission_t{__submission, timestamp, {}, false});
    }

    mutex_condition.notify_one();
}

// Find total length of exact matches
// Return vector of pairs where each pair {i, j} denotes
// match of lenght i starting at index j in T
std::vector<std::pair<int, int>> length_subarrays(std::vector<int> &S,
                                                  std::vector<int> &T) {
    SuffixAutomaton sa{};
    std::vector<std::pair<int, int>> matches{};
    for (int x : S)
        sa.sa_extend(x);

    int v{}, l{};
    std::vector<int> length(T.size(), 0);
    for (int i = 0; i < T.size(); i++) {
        while (v && !sa.states[v].next.count(T[i])) {
            v = sa.states[v].link;
            l = sa.states[v].len;
        }
        if (sa.states[v].next.count(T[i])) {
            v = sa.states[v].next[T[i]];
            l++;
        }
        length[i] = l;
    }
    std::priority_queue<std::pair<int, int>> pq{};
    for (int i = 0; i < length.size(); i++) {
        if (i + 1 < length.size() && length[i + 1] > length[i])
            continue;
        if (length[i] >= 15)
            pq.emplace(length[i], i - length[i] + 1);
    }
    SuffixAutomaton sa2{};
    if (pq.empty())
        return {};
    std::pair<int, int> u = pq.top();
    pq.pop();
    for (int i = u.second; i < u.second + u.first; i++)
        sa2.sa_extend(T[i]);
    int non_occuring_token{};
    while (!pq.empty()) {
        u = pq.top();
        pq.pop();
        state s = sa2.states[0];
        bool found = true;
        for (int i = u.second; i < u.second + u.first; i++) {
            if (s.next.count(T[i]))
                s = sa2.states[s.next[T[i]]];
            else {
                found = false;
                break;
            }
        }
        if (!found) {
            sa2.sa_extend(--non_occuring_token);
            for (int i = u.second; i < u.second + u.first; i++)
                sa2.sa_extend(T[i]);
            matches.push_back(u);
        }
    }
    return matches;
}

void plagiarism_checker_t::check_submission(exsubmission_t __submission) {
    tokenizer_t tokeni1(__submission.submission->codefile);
    std::vector<int> tokens = tokeni1.get_tokens();
    __submission.tokens = tokens;
    submissions.push_back(__submission);

    std::set<int> se{};
    // Loop through each previoius submission
    for (auto s : submissions) {

        // Here we are checking for plagiarism between two submissions.
        if (s.submission->id != __submission.submission->id) {
            std::vector<std::pair<int, int>> plag =
                length_subarrays(s.tokens, __submission.tokens);

            for (auto p : plag) {
                se.insert(p.second);
            }
            auto diff = s.timestamp - __submission.timestamp;
            if (plag.size() >= 10) {
                if (std::abs(
                        // If the time difference is less than one second then
                        // we flag both submissions otherwise only one
                        std::chrono::duration_cast<std::chrono::milliseconds>(
                            diff)
                            .count()) <= 1000)
                    s.flagsubmission();
                __submission.flagsubmission();
            } else {
                for (auto p : plag) {
                    if (p.first >= 75) {
                        // Same time distance logic
                        if (std::abs(std::chrono::duration_cast<
                                         std::chrono::milliseconds>(diff)
                                         .count()) <= 1000)
                            s.flagsubmission();
                        __submission.flagsubmission();
                    }
                }
            }
        }
    }

    // Used for patchwork plagiarism
    if (se.size() >= 20) {
        __submission.flagsubmission();
    }
}

// End TODO
