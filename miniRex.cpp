#include <cctype>
#include <iostream>
#include <memory>
#include <set>
#include <stack>
#include <string>
#include <vector>
using namespace std;

// this assumes you will write concatenation as a.b and not as ab

struct State {
  char symbol = 0;
  State *onSymbol = nullptr;
  vector<State *> epsilon_t;
};

struct Fragment {
  State *starting;
  State *accepting;
};

vector<unique_ptr<State>> states;

State *getState() {
  states.push_back(make_unique<State>());
  return states.back().get();
}

int precedence(char op) {
  if (op == '|')
    return 1;    // union
  if (op == '.') // concat
    return 2;
  return 0;
}

string toPostFix(const string &re) {
  string out;
  stack<char> ops;

  for (char c : re) {
    if (c == '(') {
      ops.push(c);
    } else if (c == ')') {
      while (ops.top() != '(') {
        out += ops.top();
        ops.pop();
      }
      ops.pop(); // the '('
    } else if (c == '*') {
      out += c;
    } else if (c == '|' || c == '.') {
      while (!ops.empty() && ops.top() != '(' &&
             precedence(ops.top()) >= precedence(c)) {
        out += ops.top();
        ops.pop();
      }
      ops.push(c);
    } else {
      out += c;
    }
  }

  while (!ops.empty()) {
    out += ops.top();
    ops.pop();
  }
  return out;
}

Fragment toNfa(const string &postfix) {
  stack<Fragment> st;

  for (char c : postfix) {
    if (isalpha(c)) {
      State *starting = getState();
      State *accepting = getState();
      starting->symbol = c;
      starting->onSymbol = accepting;
      st.push({starting, accepting});
    } else if (c == '.') {
      Fragment B = st.top();
      st.pop();
      Fragment A = st.top();
      st.pop();
      A.accepting->epsilon_t.push_back(B.starting);
      st.push({A.starting, B.accepting});
    } else if (c == '|') {
      Fragment B = st.top();
      st.pop();
      Fragment A = st.top();
      st.pop();
      State *s0 = getState();
      State *sf = getState();
      s0->epsilon_t.push_back(A.starting);
      s0->epsilon_t.push_back(B.starting);
      A.accepting->epsilon_t.push_back(sf);
      B.accepting->epsilon_t.push_back(sf);
      st.push({s0, sf});
    } else if (c == '*') {
      Fragment A = st.top();
      st.pop();
      State *s0 = getState();
      State *sf = getState();
      s0->epsilon_t.push_back(A.starting);
      s0->epsilon_t.push_back(sf);
      A.accepting->epsilon_t.push_back(A.starting);
      A.accepting->epsilon_t.push_back(sf);
      st.push({s0, sf});
    }
  }
  return st.top();
}

void eTransitions(State *start, set<State *> &current) {
  if (current.count(start))
    return;
  current.insert(start);
  for (State *s : start->epsilon_t) {
    eTransitions(s, current);
  }
}

bool isLanguage(const Fragment &nfa, const string &word) {
  set<State *> current;
  eTransitions(nfa.starting, current);

  for (char c : word) {
    set<State *> next;
    for (State *s : current) {
      if (s->symbol == c) {
        eTransitions(s->onSymbol, next);
      }
    }
    current = next;
  }
  return current.count(nfa.accepting) > 0;
}

int main() {
  string regex = "a.b*.c";
  string postfix = toPostFix(regex);
  Fragment nfa = toNfa(postfix);

  cout << "regex:   " << regex << "\n";
  cout << "postfix: " << postfix << "\n\n";

  vector<string> tests = {"ac", "abc", "abbbc", "a", "c", "abcb", ""};
  for (const string &t : tests) {
    cout << "\"" << t << "\" -> " << (isLanguage(nfa, t) ? "match" : "no match")
         << "\n";
  }
}
