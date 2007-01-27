/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// this classes interface
#include "WordFilter.h"

// system headers
#include <iostream>
#include <ctype.h>
#include <string>

// implementation-specific headers
#ifdef DEBUG
#  include "TimeKeeper.h"
#endif


/* private */

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
#if !defined(BUILD_REGEX) && !defined(HAVE_REGEX_H)

  std::cerr << "Regular expressions are not available (using the simple filter)" << std::endl;
  return simpleFilter(input);

#else /* BUILD_REGEX || HAVE_REGEX_H */

  bool filtered = false;
  regmatch_t match[1];
  if (input == NULL) return false;
  int inputLength = strlen(input);

  // a buffer to destroy during matching (includes terminating null)
  std::string sInput = input;

  /* maintain an array of match indices of the input; values are in
   * pairs.  the first number is the start position, the second is
   * a length.  array has initial size of 256 (128 words).
   */
  std::vector<int> matchPair(MAX_FILTER_SETS);

  // how many match pairs are stored
  unsigned int matchCount = 0;

  // stores which letters have been used already
  std::string wordIndices;

  /* here, we get a list of characters that might be the start of a word.
   * characters that are preceded by a non-alphabetic character are added
   * to a list of letters.  e.g. the first line of this comment consists
   * of the letters:  hwgaloctmbs
   * This means that we will only search through filter words that begin
   * with those letters.
   */
  char previousChar = 0;
  for (int counter = 0; counter < inputLength; counter++) {
    char c = tolower(sInput[counter]);

    if (!isalpha(previousChar) && TextUtils::isVisible(c)) {

      // expand punctuation to potential alphabetic characters
      if (TextUtils::isPunctuation(c)) {

	std::string puncChars = alphabeticSetFromCharacter(c);
	for (unsigned int cnt = 0; cnt < puncChars.size(); cnt++) {
	  appendUniqueChar(wordIndices, tolower(puncChars[cnt]));
	}
      }

      // add the character itself if not previously added
      appendUniqueChar(wordIndices, c);
    }
    previousChar = c;
  }

  /* check the input for a words that begin with a prefix.  if it does,
   * then add the next letter as a letter to check for during matching.
   * e.g. "bzstring" will match a prefix of "bz" and make "s" get added
   * as a bin to check for during matching.
   */
  for (ExpCompareSet::const_iterator i = prefixes.begin(); i != prefixes.end(); ++i) {
    if (regexec(i->compiled, sInput.c_str(), 1, match, 0) == 0) {
      if ( (match[0].rm_eo < inputLength) && isalpha(sInput[match[0].rm_eo]) ) {
	/* do not forget to make sure this is a true prefix */
	if ( (match[0].rm_so > 0) && isalpha(sInput[match[0].rm_so - 1]) ) {
	  continue;
	}

	/* we found a prefix -- add the letter that follows */
	appendUniqueChar(wordIndices, tolower(sInput[match[0].rm_eo]));
      }
    }
  }


//std::cout << "WordIndexLetters are [" << wordIndices << "]" << std::endl;
  // now we have a record of all potential word boundary positions


  /* iterate over the filter words for each unique initial word character */
  int regCode;
  for (unsigned int j = 0; j < wordIndices.size(); j++) {

    /* look at all of the filters that start with the letter wordIndices[j]
     */
    const unsigned int firstchar = (unsigned char)wordIndices[j];
    for (ExpCompareSet::const_iterator i = filters[firstchar].begin();
	 i != filters[firstchar].end(); ++i) {

      /* the big kahuna burger processing goes on here */
      bool matched = true;
      while (matched) {
	matched = false;

	regCode = regexec(i->compiled, sInput.c_str(), 1, match, 0);

//std::cout << "input is [" << sInput << "]" << std::endl;

	if ( regCode == 0 ) {
	  int startOffset = match[0].rm_so;
	  int endOffset = match[0].rm_eo;

//std::cout << "We matched ... ";

	  /* make sure we only match on word boundaries */
	  if ( (startOffset>1) && (isalpha(sInput[startOffset-1])) ) {

//std::cout << "but didn't match a word beginning" << std::endl;

	    /* we are in the middle of a word.. see if we can match a prefix before this */
	    bool foundit =  false;
	    for (ExpCompareSet::const_iterator k = prefixes.begin();
		 k != prefixes.end(); ++k) {
	      if (regexec(k->compiled, sInput.c_str(), 1, match, 0) == 0) {

//std::cout << "checking prefix: " << k->word << std::endl;

		if ( (match[0].rm_so > 1) && (isalpha(sInput[match[0].rm_so - 1])) ) {
		  /* we matched, but we are still in the middle of a word */
		  continue;
		}

//std::cout << "matched a prefix! " << k->word << std::endl;
		if (match[0].rm_eo == startOffset) {
		  /* perfect prefix match */
		  startOffset = match[0].rm_so;
		  foundit = true;
		  break;
		}
	      }
	    }
	    if (!foundit) {
	      /* couldn't find a prefix, so skip this match */
//std::cout << "Could not find a prefix" <<std::endl;
	      continue;
	    }
	  }

//std::cout << "is endoffset alphabetic: " << input[endOffset] << std::endl;

	  if ( (endOffset<inputLength-1) && (isalpha(sInput[endOffset])) ) {

//std::cout << "but didn't match a word ending" << std::endl;

	    /* we are at the start of a word, but not at the end, try to get to the end */
	    bool foundit = false;
	    for (ExpCompareSet::const_iterator k = suffixes.begin();
		 k != suffixes.end(); ++k) {
//std::cout << "checking " << k->word << " against [" << input + endOffset << "]" << std::endl;

	      if (regexec(k->compiled, sInput.c_str() + endOffset, 1, match, 0) == 0) {

//std::cout << "is " << match[0].rm_eo << " less than " << inputLength - endOffset << std::endl;
//std::cout << "is alpha =?= " << input[endOffset + match[0].rm_eo + 1] << std::endl;

		/* again, make sure we are now at a word end */
		if ( (match[0].rm_eo < inputLength - endOffset) &&
		     (isalpha(sInput[endOffset + match[0].rm_eo])) ) {
		  /* we matched, but we are still in the middle of a word */
		  continue;
		}

//std::cout << "matched a suffix! " << k->word << std::endl;
		if (match[0].rm_so == 0) {
		  /* push the end forward a little since we matched */
		  endOffset += match[0].rm_eo;
		  foundit = true;
		  break;
		}
	      }
	    }
	    if (!foundit) {
	      /* couldn't find a suffix, so skip this match */
//std::cout << "Could not find a suffix" <<std::endl;
	      continue;
	    }
	  }

	  int matchLength = endOffset - startOffset;

	  // make sure that longer matches actually include at least 1 alphabetic
	  if (matchLength > 3) {
	    bool foundAlpha = false;
	    for (std::string::const_iterator position = sInput.begin(); position != sInput.end(); position++) {
	      if (isalpha(*position)) {
		foundAlpha = true;
		break;
	      }
	    }
	    if (!foundAlpha) {
	      continue;
	    }
	  }

	  // add a few more slots if necessary (this should be rare/never)
	  if (matchCount * 2 + 1 >= matchPair.size()) {
	    matchPair.resize(matchCount * 2 + 201);
	  }

	  matchPair[matchCount * 2] = startOffset; /* position */
	  matchPair[(matchCount * 2) + 1] = matchLength; /* length */
	  matchCount++;
	  filtered = true;
	  matched = true;
	  // zappo! .. erase stuff that has been filtered to speed up future checks
	  // fill with some non-whitespace alpha that is not the start/end of a suffix to prevent rematch
	  std::string filler;
	  filler.assign(matchLength, 'W');
	  sInput.replace(startOffset, matchLength, filler);

	} else if ( regCode == REG_NOMATCH ) {
	  // do nothing
	  continue;

	} else {
	  char errorBuffer[256];
	  regerror(regCode, i->compiled, errorBuffer, 256);
	  std::cout << errorBuffer << std::endl;

	} /* end if regcode */

      } /* end regexec-ing */

    } /* iterate over words in a particular character bin */

  } /* iterate over characters */

  /* finally filter the input.  only filter actual alphanumerics. */
  for (unsigned int l = 0; l < matchCount; l++) {
    /* !!! debug */
#ifdef DEBUG
    char tmp[256] = {0};
    strncpy(tmp, input + matchPair[l*2], matchPair[(l*2)+1]);
    std::cout << "Matched: [" << tmp << "]" << std::endl;
#endif

    if (filterCharacters(input, matchPair[l*2], matchPair[(l*2)+1]) <= 0) {
      // XXX with multiple matching, we will be unable to filter overlapping matches
      //      std::cerr << "Unable to filter characters" << std::endl;
      continue;
    }
  }


  return filtered;

#endif /* BUILD_REGEX || HAVE_REGEX_H */
} // end aggressiveFilter


// provides a pointer to a fresh compiled expression for some given expression
#if !defined(BUILD_REGEX) && !defined(HAVE_REGEX_H)
regex_t *WordFilter::getCompiledExpression(const std::string &) const
{
  return (regex_t *)NULL;
}
#else /* BUILD_REGEX || HAVE_REGEX_H */
regex_t *WordFilter::getCompiledExpression(const std::string &word) const
{
  regex_t *compiledReg;

  /* XXX need to convert this to use new/delete */
  if ( (compiledReg = (regex_t *)calloc(1, sizeof(regex_t))) == NULL ) {

    perror("calloc failed");
    std::cerr << "Warning: unable to allocate memory for compiled regular expression";
    return (regex_t *)NULL;

  }

  if ( regcomp(compiledReg, word.c_str(), REG_EXTENDED | REG_ICASE) != 0 ) {
    std::cerr << "Warning: unable to compile regular expression for [" << word << "]" << std::endl;
    free(compiledReg);
    return (regex_t *)NULL;
  }
  return compiledReg;

}
#endif /* BUILD_REGEX || HAVE_REGEX_H */


std::string WordFilter::l33tspeakSetFromCharacter(const char c) const
{
  std::string set = "";

  if (!isalnum(c)) {
    /* escape the non-alphanumeric (punctuation or control chars) */
    set = "  ";
    set[0] = '\\';
    set[1] = c;
    return set;
  } else if (isspace(c)) {
    set = " ";
    set[0] = c;
    return set;
  }

  switch(c) {
    case 'a':
      set = "a4@";
      break;
    case 'b':
      set = "b8";
      break;
    case 'c':
      set = "c\\(";
      break;
    case 'e':
      set = "e3";
      break;
    case 'g':
      set = "g96";
      break;
    case 'i':
      set = "il1|!\\/";
      break;
    case 'l':
      set = "li1!|\\/";
      break;
    case 'o':
      set ="o0";
      break;
    case 's':
      // dollarsign $ may not be the first char..
      set = "s$z5";
      break;
    case 't':
      set = "t+7";
      break;
    case 'v':
      set = "v\\/";
      break;
    case 'w':
      set = "w\\/";
      break;
    case 'z':
      set = "zs";
      break;
    default:
      set = " ";
      set[0] = c;
      break;
  }

  return set;
}


std::string WordFilter::alphabeticSetFromCharacter(const char c) const
{
  std::string set = " ";

  /* for most punctuation, we include the actual punctuation
   * last just in case it was really intended
   */
  switch(c) {
    case '!':
      set = "il";
      break;
    case '@':
      set = "a";
      break;
    case '$':
      set =  "s";
      break;
    case '&':
      set = "s";
      break;
    case '(':
      set = "cil";
      break;
    case ')':
      set = "il";
      break;
    case '+':
      set = "t";
      break;
    case '|':
      set = "li";
      break;
    case '\\':
      set = "li";
      break;
    case '{':
      set = "c";
      break;
    case '/':
      set = "il";
      break;
    case '*':
      set = "aeiou";
      break;
    default:
      set = " ";
      set[0] = c;
      break;
  }

  return set;
}


std::string WordFilter::expressionFromString(const std::string &word) const
{
  /* create the regular expression description */
  std::string expression;
  unsigned int length = word.length();
  std::string charSet;

  /* individual characters expand into a potential set of matchable characters */
  for (unsigned int i = 0; i < length; i++) {

    // convert to lowercase for simplicity and speed
    charSet = l33tspeakSetFromCharacter(tolower(word[i]));

    /* we specifically will create a regular expression that should at least
     * match exactly the given input, including any spaces or special
     * characters.  including spaces or other characters in the input will
     * make them required to create a match.
     */

    /* append multi-letter expansions */
    if (charSet[0] == 'f') {
      /* ensure we don't capture non-printables after end of word */
      if (i != length - 1) {
	expression.append("[fp]+[^[:alpha:]]*h?[^[:alpha:]]*");
      } else {
	expression.append("[fp]+h?");
      }
    } else {

      if ( charSet.size() >= 1 ) {
	/* appends characters classes */
	expression.append("[");
	expression.append(charSet);
	expression.append("]");
      } else if (charSet.size() == 1) {
	/* append single characters */
	expression.append(charSet);
      } else {
	std::cout << "ERROR: l33t-speak returned an empty string" << std::endl;
	std::cout << "ERROR: This should never happen" << std::endl;
	exit(1);
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

  //  std::cout << "EXP: " <<  expression << std::endl;

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
  addSuffix("dom");
  addSuffix("ity");
  addSuffix("ment");
  addSuffix("sion");
  addSuffix("tion");
  addSuffix("ness");
  addSuffix("ance");
  addSuffix("ence");
  addSuffix("er");
  addSuffix("or");
  addSuffix("ist");
  // adjective
  addSuffix("ive");
  addSuffix("ic");
  addSuffix("al");
  addSuffix("able");
  addSuffix("y");
  addSuffix("ous");
  addSuffix("ful");
  addSuffix("less");
  // verb
  addSuffix("en");
  addSuffix("ize");
  addSuffix("ate");
  addSuffix("ify");
  addSuffix("fy");
  addSuffix("ed");
  // adverb
  addSuffix("ly");
  // slang
  addSuffix("a");
  addSuffix("e");
  addSuffix("i");
  addSuffix("o");
  addSuffix("u");
  addSuffix("z");
  addSuffix("r");
  addSuffix("ah");
  addSuffix("io");
  addSuffix("rs");
  addSuffix("rz");
  addSuffix("in");
  addSuffix("n");
  addSuffix("ster");
  addSuffix("meister");
  // plurality
  addSuffix("s");
  addSuffix("es");
  // imperfect verb
  addSuffix("ing");
  // diminutive
  addSuffix("let");

#endif

  /* PREFIXES */

  // bz-specific

#if 1
  /* XXX adding prefixes _significantly_ increases the expression count
   * and is rather expensive (slow, XN+N extra checks for N words)
   */
  addPrefix("bz"); // bz-specific prefix
  addPrefix("beze"); // bz-specific prefix
  addPrefix("u"); // l33t prefix
  addPrefix("you");
  addPrefix("ura"); // l33t prefix
  addPrefix("k"); // common l33t prefix

#endif

  return;
}

/** default copy constructor */
WordFilter::WordFilter(const WordFilter& _filter)
  : alphabet(_filter.alphabet),
    filterChars(_filter.filterChars),
    suffixes(_filter.suffixes),
    prefixes(_filter.prefixes)
{
  for (int i=0; i < MAX_FILTER_SETS; i++) {
    filters[i] = _filter.filters[i];
  }
}


/** destructor releases the compiled bad words */
WordFilter::~WordFilter(void)
{
  ExpCompareSet::iterator i;

  // delete compiled words
  for (int j = 0; j < MAX_FILTER_SETS; j++) {
    for (i = filters[j].begin();
	 i != filters[j].end();
	 i++) {
      if (i->compiled) {
	regfree(i->compiled);
	free(i->compiled);
      }
    }
  }
  // delete compiled prefixes
  for (i = prefixes.begin();
       i != prefixes.end();
       i++) {
    if (i->compiled) {
      regfree(i->compiled);
      free(i->compiled);
    }
  }
  // delete compiled suffixes
  for (i = suffixes.begin();
       i != suffixes.end();
       i++) {
    if (i->compiled) {
      regfree(i->compiled);
      free(i->compiled);
    }
  }

  return;
}


// adds an individual word to the filter list
bool WordFilter::addToFilter(const std::string &word, const std::string &expression)
{
  long int length = (long int)word.length();
  if (0 >= length) {
    std::cerr << "Someone tried to add an empty word to the filter" << std::endl;
    return false;
  } // end check if word is empty

  if (expression.size() == 0) {
    /* make sure to create an expression if it wasn't given */
    std::string expr = expressionFromString(word);
    return addToFilter(word, expr);

  } else {
    /* base case */
    filter_t newFilter;

    newFilter.word = word;
    newFilter.compiled = getCompiledExpression(expression);

    unsigned int firstchar = (unsigned char)tolower(word[0]);
    /* check if the word is already added */
    if (filters[firstchar].find(newFilter) != \
	filters[firstchar].end()) {
      regfree(newFilter.compiled);
      free(newFilter.compiled);
      return false;
    } else {
      filters[firstchar].insert(newFilter);
    }
    return true;
  }
} // end addToFilter


/** loads a set of bad words from a specified file */
unsigned int WordFilter::loadFromFile(const std::string &fileName, bool verbose)
{
  char buffer[2048];
  unsigned int totalAdded=0;
  std::ifstream filterStream(fileName.c_str());

  if (!filterStream)  {
    if (verbose) {
      std::cerr << "Warning: '" << fileName << "' bad word filter file not found" << std::endl;
    }
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
      static int counter=0;
      if (counter-- <= 0) {
	std::cout << ".";
	std::cout.flush();
	counter=100;
      }
    }

    bool added = addToFilter(filterWord, std::string(""));
    if ((!added) && (verbose)) {
	std::cout << std::endl << "Word is already added: " << filterWord << std::endl;
    } else {
      totalAdded++;
    }

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
#ifdef DEBUG
  TimeKeeper before = TimeKeeper::getCurrent();
#endif
  bool filtered;
  if (simple) {
    filtered = simpleFilter(input);
  } else {
    filtered = aggressiveFilter(input);
  }
#ifdef DEBUG
  TimeKeeper after = TimeKeeper::getCurrent();
  std::cout << "Time elapsed: " << after - before << " seconds" << std::endl;
#endif
  return filtered;
}

bool WordFilter::filter(std::string &input, bool simple) const
{
  char input2[512] = {0};
  bool filtered = false;
  std::string resultString = "";

  /* filter in 512 chunks.  ugly means to support large input strings,
   * but it works.  just means words that span the boundary might be
   * wrong.
   */
  for (unsigned int i = 0; i < input.size(); i+=512) {
    strncpy(input2, input.c_str() + i, 512);
    bool filteredChunk = filter(input2, simple);
    if (filteredChunk) {
      filtered = true;
    }
    resultString += input2;
  }
  if (filtered) {
    input = resultString;
  }
  return filtered;
}

void WordFilter::outputFilter(void) const
{
  for (int i=0; i < MAX_FILTER_SETS; ++i) {
    int count=0;
    for (ExpCompareSet::const_iterator j = filters[i].begin(); \
	 j != filters[i].end(); \
	 ++j) {
      std::string jword = j->word;
      std::cout << count++ << ": " << jword << std::endl;
      std::cout << "    " << expressionFromString(jword) << std::endl;
    }
  }

}
void WordFilter::outputWords(void) const
{
  //		std::cout << "size of compiled set is " << () << std::endl;
  for (int i=0; i < MAX_FILTER_SETS; ++i) {
    int count=0;
    for (ExpCompareSet::const_iterator j = filters[i].begin(); \
	 j != filters[i].end(); \
	 ++j) {
      std::cout << "[" << i << "] " << count++ << ": " << j->word << std::endl;
    }
  }

}
unsigned long int WordFilter::wordCount(void) const
{
  int count=0;
  for (int i=0; i < MAX_FILTER_SETS; ++i) {
    for (ExpCompareSet::const_iterator j = filters[i].begin(); \
	 j != filters[i].end(); \
	 ++j) {
      count += 1;
    }
  }
  return count;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
