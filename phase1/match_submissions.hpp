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
        state s = sa2.st[0];
        bool found = true;
        for (int i = u.second; i < u.second + u.first; i++) {
            if (s.next.count(T[i]))
                s = sa2.st[s.next[T[i]]];
            else {
                found = false;
                break;
            }
        }
        if (!found) {
            sa2.sa_extend(--non_occuring_token);
            for (int i = u.second; i < u.second + u.first; i++)
                sa2.sa_extend(T[i]);
            total_length += u.first;
        }
    }
    return total_length;
}

// returns {length, start1}
std::pair<int, int> levenshtein_search(const std::vector<int> &T,
                                       const std::vector<int> &P, float alpha,
                                       int start2) {
    int m = P.size() - start2;
    int n = T.size();

    std::vector<int> C(m + 1, 0), Cd(m + 1, 0);
    std::vector<int> l(m + 1, 0), ld(m + 1, 0);

    for (int i = 0; i <= m; i++) {
        C[i] = i;
        Cd[i] = i;
    }
    std::pair<int, int> result{0, -1};
    for (int j = 1; j <= n; j++) {
        for (int i = 1; i <= m; i++) {
            if (P[i - 1 + start2] == T[j - 1]) {
                Cd[i] = C[i - 1];
                ld[i] = l[i - 1] + 1;
            } else {
                if (C[i] <= Cd[i - 1] && C[i] <= C[i - 1]) {
                    Cd[i] = C[i] + 1;
                    ld[i] = l[i] + 1;
                } else if (C[i - 1] <= Cd[i - 1] && C[i - 1] <= C[i]) {
                    Cd[i] = C[i - 1] + 1;
                    ld[i] = l[i - 1] + 1;
                } else {
                    Cd[i] = Cd[i - 1] + 1;
                    ld[i] = ld[i - 1];
                }
            }
            int len = (ld[i] + i) >> 1;
            if (Cd[i] <= alpha * len && len > result.first)
                result = {len, j - ld[i]};
        }
        C = Cd;
        l = ld;
    }
    return result;
}

std::array<int, 5> match_submissions(std::vector<int> &submission1,
                                     std::vector<int> &submission2) {
    std::array<int, 5> result = {0, 0, 0, 0, 0};
    result[1] = length_subarrays(submission1, submission2);

    for (int i = 0; i + std::max(30, result[2]) <= submission2.size(); i++) {
        std::pair<int, int> p =
            levenshtein_search(submission1, submission2, 0.05, i);
        if (p.first > result[2]) {
            result[2] = p.first;
            result[3] = p.second;
            result[4] = i;
        }
    }
    return result;
}
