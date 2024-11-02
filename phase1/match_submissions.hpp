#include <array>
#include <iostream>
#include <span>
#include <vector>
#include <cmath>
// -----------------------------------------------------------------------------
#include <map>
#include <queue>

// You are free to add any STL includes above this comment, below the --line--.
// DO NOT add "using namespace std;" or include any other files/libraries.
// Also DO NOT add the include "bits/stdc++.h"

// OPTIONAL: Add your helper functions and data structures here

struct state {
    int len, link;
    std::map<int, int> next;
};

struct VectorSizeCompare {
    bool operator()(const std::vector<int> &lhs,
                    const std::vector<int> &rhs) const {
        return lhs.size() < rhs.size();
    }
};

class SuffixAutomaton {
  public:
    static const int MAXLEN = 100000;
    state *st;
    int sz, last;

    SuffixAutomaton() {
        st = new state[MAXLEN * 2];
        st[0].len = 0;
        st[0].link = -1;
        sz = 1;
        last = 0;
    }

    ~SuffixAutomaton() { delete[] st; }

    void sa_extend(int c) {
        int cur = sz++;
        st[cur].len = st[last].len + 1;
        int p = last;
        while (p != -1 && !st[p].next.count(c)) {
            st[p].next[c] = cur;
            p = st[p].link;
        }
        if (p == -1) {
            st[cur].link = 0;
        } else {
            int q = st[p].next[c];
            if (st[p].len + 1 == st[q].len) {
                st[cur].link = q;
            } else {
                int clone = sz++;
                st[clone].len = st[p].len + 1;
                st[clone].next = st[q].next;
                st[clone].link = st[q].link;
                while (p != -1 && st[p].next[c] == q) {
                    st[p].next[c] = clone;
                    p = st[p].link;
                }
                st[q].link = st[cur].link = clone;
            }
        }
        last = cur;
    }
};

int length_subarrays(std::vector<int> &S, std::vector<int> &T) {
    SuffixAutomaton sa{};
    for (int x : S)
        sa.sa_extend(x);

    int v{}, l{};
    std::vector<int> length(T.size(), 0);
    for (int i = 0; i < T.size(); i++) {
        while (v && !sa.st[v].next.count(T[i])) {
            v = sa.st[v].link;
            l = sa.st[v].len;
        }
        if (sa.st[v].next.count(T[i])) {
            v = sa.st[v].next[T[i]];
            l++;
        }
        length[i] = l;
    }
    int total_length{};
    std::priority_queue<std::vector<int>, std::vector<std::vector<int>>,
                        VectorSizeCompare>
        pq{};
    for (int i = 0; i < length.size(); i++) {
        if (i + 1 < length.size() && length[i + 1] > length[i])
            continue;
        if (length[i] >= 10)
            pq.push(std::vector<int>(T.begin() + i - length[i] + 1,
                                     T.begin() + i + 1));
    }
    SuffixAutomaton sa2{};
    std::vector<int> arr = pq.top();
    pq.pop();
    total_length = arr.size();
    for (int x : arr)
        sa2.sa_extend(x);
    int non_occuring_token{};
    while (!pq.empty()) {
        arr = pq.top();
        pq.pop();
        state s = sa2.st[0];
        bool found = true;
        for (int x : arr) {
            if (s.next.count(x))
                s = sa2.st[s.next[x]];
            else {
                found = false;
                break;
            }
        }
        if (!found) {
            sa2.sa_extend(--non_occuring_token);
            for (int x : arr)
                sa2.sa_extend(x);
            total_length += arr.size();
        }
    }
    return total_length;
}

std::array<int, 5> match_submissions(std::vector<int> &submission1,
                                     std::vector<int> &submission2) {
    std::array<int, 5> result = {0, 0, 0, 0, 0};
    result[1] = length_subarrays(submission1, submission2);
    return result;
}
