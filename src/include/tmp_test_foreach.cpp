
#include <stdio.h>
#include <ctype.h>
#include <set>
#include <string>

#include "foreach.h"


int main(int argc, char** argv) {
  std::set<std::string> s;
  s.insert("foo");
  s.insert("bar");
  s.insert("ann");
  s.insert("bob");
  foreach (v, s) {
    printf("%s\n", v.c_str());
  }
  foreach (c, std::string("testing")) {
    putchar(toupper(c));
  }
  putchar('\n');
  return 0;
}
