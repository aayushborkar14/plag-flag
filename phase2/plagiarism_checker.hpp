#include "structures.hpp"
// -----------------------------------------------------------------------------
#include <chrono>
#include <condition_variable>
#include <functional>
#include <map>
#include <queue>
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
    int total_length = u.first;
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
            total_length += u.first;
        }
    }
    return matches;
}

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
    void check_submission(std::shared_ptr<submission_t> __submission);

    std::mutex queue_mutex;
    std::condition_variable mutex_condition;
    std::thread worker;
    std::queue<std::shared_ptr<submission_t>> jobs;
    bool stop;
    // End TODO
};
