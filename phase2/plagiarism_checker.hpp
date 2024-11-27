#include "structures.hpp"
// -----------------------------------------------------------------------------
#include <chrono>
#include <condition_variable>
#include <map>
#include <queue>
#include <set>
#include <thread>

// You are free to add any STL includes above this comment, below the --line--.
// DO NOT add "using namespace std;" or include any other files/libraries.
// Also DO NOT add the include "bits/stdc++.h"

// OPTIONAL: Add your helper functions and classes here

struct state {
    int len, link;
    std::map<int, int> next;
};

// Suffix Automaton, adapted from
// https://cp-algorithms.com/string/suffix-automaton.html
class SuffixAutomaton {
  public:
    static const int MAXLEN = 100000;
    state *states;
    int state_count, last_state;

    SuffixAutomaton() {
        states = new state[MAXLEN * 2];
        states[0].len = 0;
        states[0].link = -1;
        state_count = 1;
        last_state = 0;
    }

    ~SuffixAutomaton() { delete[] states; }

    // Adds a token to the suffix automaton (we add tokens sequentially to build
    // it for the entire stream)
    void sa_extend(int c) {
        int cur = state_count++;
        states[cur].len = states[last_state].len + 1;
        int p = last_state;
        while (p != -1 && !states[p].next.count(c)) {
            states[p].next[c] = cur;
            p = states[p].link;
        }
        if (p == -1) {
            states[cur].link = 0;
        } else {
            int q = states[p].next[c];
            if (states[p].len + 1 == states[q].len) {
                states[cur].link = q;
            } else {
                int clone = state_count++;
                states[clone].len = states[p].len + 1;
                states[clone].next = states[q].next;
                states[clone].link = states[q].link;
                while (p != -1 && states[p].next[c] == q) {
                    states[p].next[c] = clone;
                    p = states[p].link;
                }
                states[q].link = states[cur].link = clone;
            }
        }
        last_state = cur;
    }
};

// A struct to streamline the process of dealing with submissions
struct exsubmission_t {
    std::shared_ptr<submission_t> submission;
    std::chrono::time_point<std::chrono::system_clock> timestamp;
    std::vector<int> tokens;
    bool flagged = false;

    // Avoid multiple flagging
    void flagsubmission() {
        if (!flagged) {
            if (submission->student)
                submission->student->flag_student(submission);
            if (submission->professor)
                submission->professor->flag_professor(submission);
            flagged = true;
        }
    }
};

class plagiarism_checker_t {
    // You should NOT modify the public interface of this class.
  public:
    plagiarism_checker_t(void);
    plagiarism_checker_t(
        std::vector<std::shared_ptr<submission_t>> __submissions);
    ~plagiarism_checker_t(void);
    void add_submission(std::shared_ptr<submission_t> __submission);

  protected:
    // TODO: Add members and function signatures here

    void thread_loop();
    void check_submission(exsubmission_t __submission);

    // Submissions vector mantained by the check_submission function.
    // Jobs queue shared by the check submissions and add_submissions functions.
    // This is shared between threads and hence needs the use of mutex to avoid
    // data races. The condition variable and the queue_mutex are for the above
    // purpose The worker thread is the thread in which the actual plagiarism
    // checking logic would work.
    std::vector<exsubmission_t> submissions;
    std::mutex queue_mutex;
    std::condition_variable mutex_condition;
    std::thread worker;
    std::queue<exsubmission_t> jobs;
    bool stop;
    // End TODO
};
