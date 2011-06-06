
#include <stdio.h>
#include <set>
#include <string>

#include "foreach.h"


int main(int argc, char** argv) {
  std::set<std::string> s;
  s.insert("foo");
  s.insert("bar");
  s.insert("ann");
  s.insert("bob");
  foreach (it, s) {
    printf("%s\n", it->c_str());
  }
  return 0;
}