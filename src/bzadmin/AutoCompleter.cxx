#include <algorithm>
#include <limits>

#include "AutoCompleter.h"


void AutoCompleter::registerWord(const string& str) {
  words.insert(lower_bound(words.begin(), words.end(), str), str);
}


void AutoCompleter::unregisterWord(const string& str) {
  vector<string>::iterator iter = lower_bound(words.begin(), words.end(), str);
  if (iter != words.end() && *iter == str)
    words.erase(iter);
}


string AutoCompleter::complete(const string& str) {
  
  if (str.size() == 0)
    return str;
  
  // find the first and last word with the prefix str
  vector<string>::iterator first, last;
  first = lower_bound(words.begin(), words.end(), str);
  if (first == words.end() || first->substr(0, str.size()) != str)
    return str;
  string tmp = str;
  tmp[tmp.size() - 1]++;
  last = lower_bound(first, words.end(), tmp) - 1;
  
  // return the largest common prefix
  unsigned int i;
  for (i = 0; i < first->size() && i < last->size(); ++i)
    if ((*first)[i] != (*last)[i])
      break;
  return first->substr(0, i);
}
