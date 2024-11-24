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

void plagiarism_checker_t::add_submission(std::shared_ptr<submission_t> __submission) {
    const std::chrono::time_point<std::chrono::system_clock> timestamp = std::chrono::system_clock::now();
    tokenizer_t tokeni1(__submission->codefile);
    std::vector<int> tokens = tokeni1.get_tokens();

    submissions.push_back({__submission , timestamp , tokens});

    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if (stop)
            return;
        jobs.push(exsubmission_t{__submission , timestamp , tokens});
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
        if (length[i] >= 10)
            pq.emplace(length[i], i - length[i] + 1);
    }
    SuffixAutomaton sa2{};
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
            if (u.first >= 15) {
                matches.push_back(u);
            }
        }
    }
    return matches;
}

void plagiarism_checker_t::check_submission(exsubmission_t __submission){
    
    std::set<int> se;
    for(auto s : submissions){
        if(s.submission->id != __submission.submission->id)
        {
            std::vector<std::pair<int,int>> plag = length_subarrays(s.tokens , __submission.tokens);
            
            for(auto p: plag)
            {
                se.insert(p.second);
            }

            if(plag.size() > 10)
            {
                if(std::chrono::milliseconds(1000) >= (s.timestamp - __submission.timestamp) && (s.timestamp - __submission.timestamp) >= std::chrono::milliseconds(-1000))
                    s.flagsubmission();
                __submission.flagsubmission();
            }
            else
            {
                for(auto p: plag)
                {
                    if(p.first >= 75) 
                    {
                        if(std::chrono::milliseconds(1000) >= (s.timestamp - __submission.timestamp) && (s.timestamp - __submission.timestamp) >= std::chrono::milliseconds(-1000))
                            s.flagsubmission();
                        __submission.flagsubmission();
                    }
                }
            }
        }

    }

    if(se.size() >= 20)
    {
        __submission.flagsubmission();
    }

}

// End TODO
