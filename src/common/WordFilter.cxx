/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* no header other than WordFilter.h should be included here */

#ifdef _WIN32
#pragma warning( 4:4786)
#endif

#include "WordFilter.h"


/* protected */

bool WordFilter::simpleFilter(char *input) const
{
  bool filtered = false;
  filter_t findWord;

  std::string line = input;
  int startPosition = line.find_first_of(alphabet);

  int endPosition;
  std::string word;
  unsigned int firstchar;
  
  /* here we iterate over all of the words in the input and replace
   * exact matches with asterisks
   */
  while (startPosition >= 0) {
    endPosition = line.find_first_not_of(alphabet, startPosition+1);
    if (endPosition < 0)
      endPosition = line.length();

    word = line.substr(startPosition, endPosition-startPosition);
    findWord.word = word;
    firstchar = (unsigned char)word[0];
    if (filters[firstchar].find(findWord) != \
	filters[firstchar].end()) {

      /* fill with asterisks */
      //      memset(input+startPosition,'*', endPosition-startPosition);

      /* fill with random filter chars */
      if (filterCharacters(input, startPosition, endPosition-startPosition, true) > 0) {
	filtered=true;
      }
    }
    startPosition = line.find_first_of(alphabet, endPosition);
  }
  return filtered;
} // end simpleFilter


bool WordFilter::aggressiveFilter(char *input) const
{
#if HAVE_REGEX_H
  bool filtered = false;
  static regmatch_t match[1];
  char errorBuffer[512];

  /* implicit limit of only match up to 256 words per input */
  /* !!! make dynamic */
  int matchPair[MAX_WORDS * 2];
  int matchCount = 0;

  int regCode;

  int startPosition = firstAlphanumeric(input);
  if (startPosition < 0) {
    return false;
  }
  std::string line = input + startPosition;
  std::bitset<MAX_WORDS * 4> startBoundaryArray = 0;
  std::bitset<MAX_WORDS * 4> endBoundaryArray = 0;

  startBoundaryArray.set(startPosition);
  int inputPosition = startPosition;
  startPosition = 0;

  int endPosition;
  unsigned char wordIndices[MAX_WORDS];
  unsigned int wordIndexLength=0;

  char characterIndex;

  /* clear memory for arrays */
  memset(matchPair, 0, MAX_WORDS * 2 * sizeof matchPair[0]);
  memset(wordIndices, 0, MAX_WORDS * sizeof wordIndices[0]);

  /* iterate over all the words start position in the input and keep track
   * of the starting character of each word (we only need to check those)
   * XXX could consider replacing with a single regexp call
   */
  int counter=0;
  int characterFound;
  while (startPosition >= 0) {
    endPosition = firstNonalphanumeric(line);
    if (endPosition < 0) {
      endPosition = line.length();
    }
    // record the position of where words start and end
    endBoundaryArray.set(endPosition+inputPosition-1);

    // words are hashed by lowercase first letter
    characterIndex = tolower(line[startPosition]);

    characterFound=0;
    for (unsigned int i=0; i < wordIndexLength; i++) {
      if (wordIndices[i] == characterIndex) {
	characterFound=1;
	break;
      }
    }
    if (!characterFound) {
      counter++;
      wordIndices[wordIndexLength] = characterIndex;
      wordIndexLength++;
    }

    // trim off the front of the line including the last non-printable
    line.erase(0, endPosition + 1);
    inputPosition += endPosition + 1;

    startPosition = firstAlphanumeric(line); // should be zero most of the time
    if (startPosition == 0) {
      //record the position of where words start and end
      startBoundaryArray.set(inputPosition);

    } else if (startPosition > 0) {
      //record the position of where words start and end
      startBoundaryArray.set(startPosition+inputPosition);

      line.erase(0,startPosition);
      inputPosition += startPosition;
      startPosition = 0;
    }
  }

  /*
    std::cout << "counted " << counter << " words" << std::endl;
    std::cout << "boundary array:[" << boundaryArray << "]" << std::endl;
  */

  counter = 0;
  /* iterate over the filter words for each unique initial word character */
  int startMatchPos;
  for (unsigned int j = 0; j < wordIndexLength; j++) {
    for (std::set<filter_t, expressionCompare>::iterator i = filters[wordIndices[j]].begin(); \
	 i != filters[wordIndices[j]].end();
	 ++i) {

      startMatchPos = 0;
      while (startMatchPos >=0) {
	regCode = regexec(i->compiled, input + startMatchPos, 1, match, 0);
	counter++;

	if ( regCode == 0 ) {

	  /* make sure we only match on word boundaries */
	  if (!startBoundaryArray.test(match[0].rm_so + startMatchPos)) {
	    //	  std::cout << "matched non-word start boundary at " << match[0].rm_so + startMatchPos << std::endl;
	    startMatchPos += match[0].rm_eo;
	    continue;
	  }
	  if (!endBoundaryArray.test(match[0].rm_eo-1 + startMatchPos)) {
	    //	  std::cout << "matched non-word end boundary at " << match[0].rm_eo + startMatchPos << std::endl;
	    startMatchPos += match[0].rm_eo;
	    continue;
	  }

	  matchPair[matchCount * 2] = match[0].rm_so + startMatchPos; /* position */
	  matchPair[(matchCount * 2) + 1] = (match[0].rm_eo - match[0].rm_so) + startMatchPos; /* length */
	  matchCount++;
	  filtered = true;

	  startMatchPos += match[0].rm_eo;

	} else if ( regCode == REG_NOMATCH ) {
	  // do nothing
	  //			continue;
	  startMatchPos = -1;
	} else {
	  regerror(regCode, i->compiled, errorBuffer, 512);
	  std::cout << errorBuffer << std::endl;
	  startMatchPos = -1;
	}

      } /* end iteration over multiple matches */
    } /* iterate over words in a particular character bin */
  } /* iterate over characters */


  /*
    std::cout << "searched " << counter << " words" << std::endl;
  */

  /* finally filter the input.  only filter actual alphanumerics. */
  for (int i=0; i < matchCount; i++) {
    //			std::cout << "i: " << i << "  " << matchPair[i*2] << " for " << matchPair[(i*2)+1] << std::endl;

    if (filterCharacters(input, matchPair[i*2], matchPair[(i*2)+1]) <= 0) {
      // XXX with multiple matching, we will be unable to filter overlapping matches
      //      std::cerr << "Unable to filter characters" << std::endl;
    }
  }


  return filtered;

#else /* HAVE_REGEX_H */
  std::cerr << "Regular expressions are not available" << std::endl;
  return simpleFilter(input);

#endif /* HAVE_REGEX_H */
} // end aggressiveFilter


// provides a pointer to a fresh compiled expression for some given expression
regex_t *WordFilter::getCompiledExpression(const std::string &word) const
{
#if HAVE_REGEX_H
  regex_t *compiledReg;

  /* XXX need to convert this to use new/delete */
  if ( (compiledReg = (regex_t *)calloc(1, sizeof(regex_t))) == NULL ) {

    perror("calloc failed");
    std::cerr << "Warning: unable to allocate memory for compiled regular expression";
    return (regex_t *)NULL;

  }

  if ( regcomp(compiledReg, word.c_str(), REG_EXTENDED | REG_ICASE) != 0 ) {
    std::cerr << "Warning: unable to compile regular expression for [" << word << "]" << std::endl;
    return (regex_t *)NULL;
  }
  return compiledReg;

#else /* HAVE_REGEX_H */
  return (regex_t *)NULL;

#endif /* HAVE_REGEX_H */
}


std::string WordFilter::expressionFromString(const std::string &word) const
{
  /* create the regular expression description */
  std::string expression;
  unsigned int length = word.length();
  char c[8];

  /* individual characters expand into a potential set of matchable characters */
  for (unsigned int i = 0; i < length; i++) {
    c[0] = c[1] = c[2] = c[3] = c[4] = c[5] = c[6] = c[7] = 0;
    // convert to lowercase for simplicity and speed
    c[0] = tolower(word[i]);

    /* we specifically will create a regular expression that should at least
     * match exactly the given input, including any spaces or special
     * characters.  including spaces or other characters in the input will
     * make them required to create a match.  BUT.. they need to be escaped
     * so that the regexp compiles properly.
     */
    if (!isAlphanumeric(c[0])) {
      /* escape the non-alphanumeric */
      c[1] = c[0];
      c[0] = '\\';
    }
    /* character classes for l33t-speak */
    if ( c[0] == 'l' ) {
      c[1] = 'i';
      c[2] = '1';
      c[4] = '!';
      c[5] = '|';
      c[6] = '/';
      c[7] = '\\';
      //      c[8] = '\\';
    } else if ( c[0] == 'i' ) {
      c[1] = 'l';
      c[2] = '|';
      c[3] = '!';
      c[4] = '/';
      c[5] = '\\';
      //      c[6] = '\\';
    } else if ( c[0] == 'e' ) {
      c[1] = '3';
    } else if ( c[0] == 's' ) {
      c[1] = '$'; // should never be the last character
      c[2] = 'z';
      c[3] = '5';
    } else if ( c[0] == 't' ) {
      c[1] = '+';
    } else if ( c[0] == 'c' ) {
      c[1] = '\\';
      c[2] = '(';
    } else if ( c[0] == 'a' ) {
      c[1] = '4';
      c[2] = '@';
    } else if ( c[0] == 'b' ) {
      c[1] = '8';
    } else if ( c[0] == 'o' ) {
      c[1] = '0';
    } else if ( c[0] == 'g' ) {
      c[1] = '9';
    } else if ( c[0] == 'z' ) {
      c[1] = 's';
    }
    // need to handle 'f' special for f=>(f|ph)

    /* append multi-letter expansions */
    if (c[0] == 'f') {
      /* ensure we don't capture non-printables after end of word */
      if (i != length - 1) {
	expression.append("[fp]+[^[:alpha:]]*h?[^[:alpha:]]*");
      } else {
	expression.append("[fp]+h?");
      }
    } else {
      if ( c[1] != 0 ) {
	/* appends characters classes */
	expression.append("[");
	expression.append(c);
	expression.append("]");
      } else {
	/* append single characters */
	expression.append(c);
      }

      /* ensure we don't capture non-printables after end of word. these do
       * not get appended to the special "f" case.
       */
      if (i != length - 1) {
	expression.append("+[^[:alpha:]]*");
      } else {
	expression.append("+");
      }

    } // end test for multi-letter expansions

  } // end iteration over word letters

  //  std::cout << "[" <<  expression << "]" << std::endl;

  return expression;
}


/* public: */

WordFilter::WordFilter()
{
  filter_t fix;

  /* set up the alphabet for simple filtering */
  alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

  /* filter characters randomly used to replace filtered text */
  filterChars = "!@#$%^&*";

  /* SUFFIXES */

#if 1
  // noun
  fix.word = "dom";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "ity";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "ment";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "sion";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "tion";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "ness";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "ance";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "ence";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "er";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "or";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "ist";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  // adjective
  fix.word = "ive";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "en";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "ic";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "al";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "able";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "y";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "ous";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "ful";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "less";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  // verb
  fix.word = "en";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "ize";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "ate";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "ify";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "fy";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "ed";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  // adverb
  fix.word = "ly";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  // slang
  fix.word = "a";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "e";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "i";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "o";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "u";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "z";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "r";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "ah";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "io";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "rs";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "rz";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "in";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "n";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "ster";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "meister";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  // plurality
  fix.word = "s";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  fix.word = "es";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  // imperfect verb
  fix.word = "ing";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);
  // diminutive
  fix.word = "let";
  fix.expression = expressionFromString(fix.word);
  suffixes.insert(fix);

#endif

  /* PREFIXES */

  // bz-specific

#if 1
  /* XXX adding prefixes _significantly_ increases the expression count
   * and is rather expensive (slow, XN+N extra checks for N words)
   */
  fix.word = "bz";
  fix.expression = expressionFromString(fix.word);
  prefixes.insert(fix);
  fix.word = "bez";
  fix.expression = expressionFromString(fix.word);
  prefixes.insert(fix);
  fix.word = "beze";
  fix.expression = expressionFromString(fix.word);
  prefixes.insert(fix);
  fix.word = "u";
  fix.expression = expressionFromString(fix.word);
  prefixes.insert(fix);
  fix.word = "you";
  fix.expression = expressionFromString(fix.word);
  prefixes.insert(fix);


#endif

  return;
}

/** default copy constructor */
WordFilter::WordFilter(const WordFilter& filter)
  : alphabet(filter.alphabet),
    filterChars(filter.filterChars),
    suffixes(filter.suffixes),
    prefixes(filter.prefixes)
{
  for (int i=0; i < MAX_FILTERS; i++) {
    filters[i] = filter.filters[i];
  }
}



/** destructor releases the compiled bad words */
WordFilter::~WordFilter(void)
{
  for (int j = 0; j < MAX_FILTERS; ++j) {
    for (std::set<filter_t, expressionCompare>::iterator i = filters[j].begin();
	 i != filters[j].end();
	 ++i) {
      free(i->compiled);
    }
  }

  return;
}
// consider calling regfree()


// adds an individual word to the filter list
bool WordFilter::addToFilter(const std::string &word, const std::string &expression, bool append)
{
  long int length = (long int)word.length();
  if (0 >= length) {
    std::cerr << "Someone tried to add an empty word to the filter" << std::endl;
    return false;
  } // end check if word is empty

  if (expression.size() == 0) {
    /* make sure to create an expression if it wasn't given */
    std::string expression = expressionFromString(word);
    return addToFilter(word, expression, append);

  } else if (append) {
    /* add words with all suffixes appended */
    std::set<filter_t, expressionCompare>::iterator i;
#if 1
    std::string fullSuffix = "((";
    for (i = suffixes.begin();
	 i != suffixes.end();) {
      fullSuffix.append(i->word);
      if (++i != suffixes.end()) {
	fullSuffix.append(")|(");
      }
    }
    fullSuffix.append("))*");
    //    std::cout << "prefixes: " << fullSuffix << std::endl;
    return addToFilter(word, expression +  fullSuffix, false);
#else
    for (std::set<filter_t, expressionCompare>::iterator i = suffixes.begin();
	 i != suffixes.end(); ++i) {
      addToFilter(word + i->word, expression + i->expression, false);
    }
#endif /* suffixes */

    /* add words with all prefixes prepended */
#if 0
    std::string fullPrefix = "((";
    for (i = prefixes.begin();
	 i != prefixes.end();) {
      fullPrefix.append(i->word);
      if (++i != prefixes.end()) {
	fullPrefix.append(")|(");
      }
    }
    fullPrefix.append("))*");
    return addToFilter(word, fullPrefix + expression + fullSuffix, false);
#else
    for (i = prefixes.begin();
	 i != prefixes.end(); ++i) {
      addToFilter(i->word + word, i->expression + expression, false);

      /* add words with prefix and suffix appended */
      for (std::set<filter_t, expressionCompare>::iterator j = suffixes.begin();
	   j != suffixes.end(); ++j) {
	addToFilter(i->word + word + j->word,
		    i->expression + expression + j->expression,
		    false);
      }
    }
#endif  /* prefixes */

    /* don't forget to add the unadulterated word */
    return addToFilter(word, expression, false);
  } else {
    /* base case */
    filter_t newFilter;

    newFilter.word = word;
    newFilter.expression = expression;
    newFilter.compiled = getCompiledExpression(expression);

    unsigned int firstchar = (unsigned char)tolower(word[0]);
    /* check if the word is already added */
    if (filters[firstchar].find(newFilter) != \
	filters[firstchar].end()) {
      return false;
    } else {
      filters[firstchar].insert(newFilter);
    }
    return true;
  }
} // end addToFilter
bool WordFilter::addToFilter(const std::string &word, const std::string &expression)
{
  return addToFilter(word, expression, false);
}
bool WordFilter::addToFilter(const std::string &word, bool append)
{
  return addToFilter(word, "", append);
}
bool WordFilter::addToFilter(const std::string &word)
{
  return addToFilter(word, "", false);
}



/** loads a set of bad words from a specified file */
unsigned int WordFilter::loadFromFile(const std::string &fileName, bool verbose)
{
  char buffer[2048];
  unsigned int totalAdded=0;
  std::ifstream filterStream(fileName.c_str());

  if (!filterStream) {
    std::cerr << "Warning: '" << fileName << "' bad word filter file not found" << std::endl;
    return 0;
  }

  while (filterStream.good()) {
    filterStream.getline(buffer,2048);

    std::string filterWord = buffer;

    int position = filterWord.find_first_not_of("\r\n\t ");

    // trim leading whitespace
    if (position > 0) {
      filterWord = filterWord.substr(position);
    }

    position = filterWord.find_first_of("#\r\n");

    // trim trailing comments
    if ((position >= 0) && (position < (int)filterWord.length())) {
      filterWord = filterWord.substr(0, position);
    }

    position = filterWord.find_last_not_of(" \t\n\r");
    // first whitespace is at next character position
    position += 1;

    // trim trailing whitespace
    if ((position >=0) && (position < (int)filterWord.length())) {
      filterWord = filterWord.substr(0, position);
    }

    /* make sure the word isn't empty (e.g. comment lines) */
    if (filterWord.length() == 0) {
      continue;
    }

    /*
      std::cout << "[[[" <<	 filterWord << "]]]" << std::endl;
    */

    if (verbose) {
      std::cout << ".";
    }

    bool added = addToFilter(filterWord, "", true);
    if (!added) {
      if (verbose) {
	std::cout << std::endl << "Word is already added: " << filterWord << std::endl;
      }
    }
    totalAdded++;

  } // end iteration over input file
  if (verbose) {
    std::cout << std::endl;
  }

  return totalAdded;
} // end loadFromFile


/** filters an input message either a complex regular expression-based
 * pattern match (default) catching hundreds of variations per filter
 * word or using a simple exact word match technique (original).
 */
bool WordFilter::filter(char *input, bool simple) const
{
  if (simple) {
    return simpleFilter(input);
  }
  return aggressiveFilter(input);
}

void WordFilter::outputFilter(void) const
{
  for (int i=0; i < MAX_FILTERS; ++i) {
    int count=0;
    for (std::set<filter_t, expressionCompare>::const_iterator j = filters[i].begin(); \
	 j != filters[i].end(); \
	 ++j) {
      std::cout << count++ << ": " << j->word << std::endl;
      std::cout << "    " << j->expression << std::endl;
    }
  }

}
void WordFilter::outputWords(void) const
{
  //		std::cout << "size of compiled set is " << () << std::endl;
  for (int i=0; i < MAX_FILTERS; ++i) {
    int count=0;
    for (std::set<filter_t, expressionCompare>::const_iterator j = filters[i].begin(); \
	 j != filters[i].end(); \
	 ++j) {
      std::cout << "[" << i << "] " << count++ << ": " << j->word << std::endl;
    }
  }

}
unsigned long int WordFilter::wordCount(void) const
{
  int count=0;
  for (int i=0; i < MAX_FILTERS; ++i) {
    for (std::set<filter_t, expressionCompare>::const_iterator j = filters[i].begin(); \
	 j != filters[i].end(); \
	 ++j) {
      count += 1;
    }
  }
  return count;
}


#if UNIT_TEST
int main (int argc, char *argv[])
{
  if (argc < 2) {
    std::cerr << "missing filename" << std::endl;
    return -1;
  }

  WordFilter filter;
  //  filter.addToFilter("fuck", true);
  filter.addToFilter("test", false);
#if 0
  filter.addToFilter("a");
  filter.addToFilter("b");
  filter.addToFilter("c");
  filter.addToFilter("d");
  filter.addToFilter("e");
  filter.addToFilter("f");
  filter.addToFilter("g");
  filter.addToFilter("h");
  filter.addToFilter("i");
  filter.addToFilter("j");
  filter.addToFilter("k");
  filter.addToFilter("l");
  filter.addToFilter("m");
  filter.addToFilter("n");
  filter.addToFilter("o");
  filter.addToFilter("p");
  filter.addToFilter("q");
  filter.addToFilter("r");
  filter.addToFilter("s");
  filter.addToFilter("t");
  filter.addToFilter("u");
  filter.addToFilter("v");
  filter.addToFilter("w");
  filter.addToFilter("x");
  filter.addToFilter("y");
  filter.addToFilter("z");
#endif

  char message[1024] = " This test is a fucKing simple test; you're NOT a beezee b i t c h!! ";
  std::cout << "PRE  SIMPLE " << message << std::endl;
  filter.filter(message, true);
  std::cout << "POST SIMPLE " << message << std::endl;

  filter.addToFilter("fuking", true);

  std::cout << "Loading file" << std::endl;
  filter.loadFromFile(argv[1], true);
  std::cout << "Number of words in filter: " << filter.wordCount() << std::endl;
  filter.addToFilter("test", true);
  std::cout << "Number of words in filter: " << filter.wordCount() << std::endl;

  //  filter.outputWords();
  //  filter.outputFilter();

  char message2[1024] = "f  u  c  k  !  fuuukking 'test' ; you're NOT a beezeecun+y!!  Phuck you b1tch! ";
  char message3[1024] = "fuck  fuck fuck Phuck you!";
  char message4[1024] = "fuckmonkey fuck monkey fuck pirate fuck clown";
  char message5[1024] = "f  u  c  k  !  fuuukking test ; you're NOT a beezeecun+y!!  Phuck you b1tch! ";
  char message6[1024] = "f  u  c  k  !  fuuukking test ; you're NOT a beezeecun+y!!  Phuck you b1tch! ";
  std::cout << "PRE  AGGRESSIVE " << message2 << std::endl;

  filter.filter(message2);
  std::cout << "POST AGGRESSIVE " << message2 << std::endl;

  std::cout << "PRE  AGGRESSIVE " << message3 << std::endl;
  filter.filter(message3);
  std::cout << "POST AGGRESSIVE " << message3 << std::endl;

  std::cout << "PRE  AGGRESSIVE " << message4 << std::endl;
  filter.filter(message4);
  std::cout << "POST AGGRESSIVE " << message4 << std::endl;

  std::cout << "PRE  AGGRESSIVE " << message5 << std::endl;
  filter.filter(message5);
  std::cout << "POST AGGRESSIVE " << message5 << std::endl;

  std::cout << "PRE  AGGRESSIVE " << message6 << std::endl;
  filter.filter(message6);
  std::cout << "POST AGGRESSIVE " << message6 << std::endl;

  return 0;
}
#endif
// ex: shiftwidth=2 tabstop=8
