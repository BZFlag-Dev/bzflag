#include "WordFilter.h"

class WordFilter
{

private:

	/* used by the simple filter */
	std::string alphabet;

	/* used by the agressive filter */
	std::set<std::string> suffixes;
	std::set<std::string> prefixes;

	std::string filterChars;

	/* structure containing the filter words and optional compiled version */
	typedef struct badWord {
		std::string word;
		bool compiled;
		std::string expression;
		regex_t *compiledWord;
	} badWord_t;

	/* word expressions to be filtered including compiled regexp versions */
	struct expressionCompare {
		bool operator () (const badWord_t& word1, const badWord_t& word2) const {
			return strncasecmp(word1.word.c_str(), word2.word.c_str(), 1024) < 0;
		}
	};
	std::set<badWord_t, expressionCompare> badWords;


protected:

	/* This filter does a simple case-insensitive word comparison that compares
	 * all filter words with all alphabetic string sets in the input string.
	 * If test is a filter word, then input strings "test", "testy", and "test;"
	 * will get filtered to "****", "testy", and "****;" respectively.
	 */
	bool simpleFilter(char *input)
	{
		bool filtered = false;
		badWord_t findWord;

		int randomCharPos=0;
		int previousCharPos=0;
		int maxFilterChar=filterChars.length();

		// all words allowed -> skip processing
		if (badWords.size() == 0) {
			return false;
		}

		std::string line = input;
		int startPosition = line.find_first_of(alphabet);
		while (startPosition >= 0) {
			int endPosition = line.find_first_not_of(alphabet, startPosition+1);
			if (endPosition < 0)
				endPosition = line.length();
			std::string word = line.substr(startPosition, endPosition-startPosition);
			findWord.word = word;
			if (badWords.find(findWord) != badWords.end()) {
				 memset(input+startPosition,'*', endPosition-startPosition);

				 for (int i=startPosition; i < endPosition; i++) {
					 do {
						 randomCharPos = (int)((float)maxFilterChar * (float)bzfrand());
					 } while (randomCharPos == previousCharPos);
					 previousCharPos = randomCharPos;
					 input[i] = filterChars.c_str()[randomCharPos];
				 }

				 filtered=true;
			}
			startPosition = line.find_first_of(alphabet, endPosition);
		}
		return filtered;
	} // end simpleFilter


	/* This filter will take a filter word and create a rather complex regular
	 * expression that catches a variety of variations on a word.		Variations
	 * include automatic internal expansion, optional punctuation and 
	 * whitespace, suffix matching (including plurality), leet-speak conversion
	 * and more.	See the header above for more details.	This filter should be
	 * the default.
	 */
	bool aggressiveFilter(char *input)
	{
		bool filtered = false;
		regex_t *compiledReg;
		static regmatch_t match[1];
		char errorBuffer[512];

		/* only match up to 128 words per input */
		int matchPair[128 * 2];
		int matchCount = 0;

		int regCode;

		int randomCharPos;
		int previousCharPos;
		int maxFilterChar=filterChars.length();

		for (std::set<badWord_t, expressionCompare>::iterator i = badWords.begin(); i != badWords.end(); ++i) {

			if (! i->compiled) {
				// perform at least a literal comparison using the simple filter
				continue;
			}
			regCode = regexec(i->compiledWord, input, 1, match, NULL);

			if ( regCode == 0 ) {
				//				std::cout << "Matched [" << i->word << "] with substring [" << input + match[0].rm_so << "] for " << match[0].rm_eo << " position; so was " << match[0].rm_so << std::endl;

				matchPair[matchCount * 2] = match[0].rm_so; /* position */
				matchPair[(matchCount * 2) + 1] = match[0].rm_eo - match[0].rm_so; /* length */
				matchCount++;
				
				filtered = true;
			} else if ( regCode == REG_NOMATCH ) {

				// do nothing
				//			continue;
			} else {
				regerror(regCode, i->compiledWord, errorBuffer, 512);
				std::cout << errorBuffer << std::endl;
			}

		}

		/* finally filter the input.  only filter actual alphanumerics. */
		for (int i=0; i < matchCount; i++) {
			//			std::cout << "i: " << i << "  " << matchPair[i*2] << " for " << matchPair[(i*2)+1] << std::endl;
			
			//			memset(input + matchPair[i*2], '*', matchPair[(i*2)+1]);

			randomCharPos = 0;
			previousCharPos = 0;
			for (int j=0; j < matchPair[(i*2)+1]; j++) {
				char c = input[matchPair[i*2] + j];

				// don't repeat random chars
				do {
					randomCharPos = (int)((float)maxFilterChar * (float)bzfrand());
				} while (randomCharPos == previousCharPos);
				previousCharPos = randomCharPos;

				/* these are the ascii character code ranges for a-z, A-Z, and 0-9 */
				if (  ( c > 96 && c < 123 ) || ( c > 64  && c < 91 ) || ( c > 47 && c < 58 )) {
						input[matchPair[i*2] + j] = filterChars.c_str()[randomCharPos];
				}

			}
		}
		

		return filtered;
	} // end aggressiveFilter


	// provides a pointer to a fresh compiled expression for some given expression
	regex_t *getCompiledExpression(const std::string &word)
	{
		regex_t *compiledReg;

		if ( (compiledReg = (regex_t *)calloc(1, sizeof(regex_t))) == NULL ) {

			perror("calloc failed");
			std::cerr << "Warning: unable to allocate memory for compiled regular expression";
			return (regex_t *)NULL;
			
		}
					
		// ??? need to test performance of REG_NOSUB until we get a match
		if ( regcomp(compiledReg, word.c_str(), REG_EXTENDED | REG_ICASE) != 0 ) {
			std::cerr << "Warning: unable to compile regular expression for [" << word << "]" << std::endl;
			return (regex_t *)NULL;
		}
		return compiledReg;
	}


public:

	WordFilter() 
	{
		/* set up the alphabet for simple filtering */
		alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

		/* filter characters are randomly used to replace filtered text */
		filterChars = "!@#$%^&*";

		/* SUFFIXES */

		// noun
		suffixes.insert("d+[^[:alpha:]]*o+[^[:alpha:]]*m+");
		suffixes.insert("i+[^[:alpha:]]*t+[^[:alpha:]]*y+");
		suffixes.insert("m+[^[:alpha:]]*e+[^[:alpha:]]*n+[^[:alpha:]]*t+");
		suffixes.insert("s+[^[:alpha:]]*i+[^[:alpha:]]*o+[^[:alpha:]]*n+");
		suffixes.insert("t+[^[:alpha:]]*i+[^[:alpha:]]*o+[^[:alpha:]]*n+");
		suffixes.insert("n+[^[:alpha:]]*e+[^[:alpha:]]*s+[^[:alpha:]]*s+");
		suffixes.insert("a+[^[:alpha:]]*n+[^[:alpha:]]*c+[^[:alpha:]]*e+");
		suffixes.insert("e+[^[:alpha:]]*n+[^[:alpha:]]*c+[^[:alpha:]]*e+");
		suffixes.insert("e+[^[:alpha:]]*r+");
		suffixes.insert("o+[^[:alpha:]]*r+");
		suffixes.insert("i+[^[:alpha:]]*s+[^[:alpha:]]*t+");

		// adjective
		suffixes.insert("i+[^[:alpha:]]*v+[^[:alpha:]]*e+");
		suffixes.insert("e+[^[:alpha:]]*n+");
		suffixes.insert("i+[^[:alpha:]]*c+");
		suffixes.insert("a+[^[:alpha:]]*l+");
		suffixes.insert("a+[^[:alpha:]]*b+[^[:alpha:]]*l+[^[:alpha:]]*e+");
		suffixes.insert("y+");
		suffixes.insert("o+[^[:alpha:]]*u+[^[:alpha:]]*s+");
		suffixes.insert("f+[^[:alpha:]]*u+[^[:alpha:]]*l+");
		suffixes.insert("l+[^[:alpha:]]*e+[^[:alpha:]]*s+[^[:alpha:]]*s+");

		// verb
		suffixes.insert("e+[^[:alpha:]]*n+");
		suffixes.insert("i+[^[:alpha:]]*z+[^[:alpha:]]*e+");
		suffixes.insert("a+[^[:alpha:]]*t+[^[:alpha:]]*e+");
		suffixes.insert("i+[^[:alpha:]]*f+[^[:alpha:]]*y+");
		suffixes.insert("f+[^[:alpha:]]*y+");
		suffixes.insert("e+[^[:alpha:]]*d+");

		// adverb
		suffixes.insert("l+[^[:alpha:]]*y+");

		// slang
		suffixes.insert("a+");
		suffixes.insert("z+");
		suffixes.insert("r+");
		suffixes.insert("a+[^[:alpha:]]*h+");
		suffixes.insert("i+[^[:alpha:]]*o+");
		suffixes.insert("r+[^[:alpha:]]*s+");
		suffixes.insert("r+[^[:alpha:]]*z+");
		suffixes.insert("i+[^[:alpha:]]*n+");
		suffixes.insert("n+");
		suffixes.insert("s+[^[:alpha:]]*t+[^[:alpha:]]*e+[^[:alpha:]]*r+");
		suffixes.insert("m+[^[:alpha:]]*e+[^[:alpha:]]*i+[^[:alpha:]]*s+[^[:alpha:]]*t+[^[:alpha:]]*e+[^[:alpha:]]*r+");
		// plurality
		suffixes.insert("s+");
		suffixes.insert("e+[^[:alpha:]]*s+");
		// imperfect verb
		suffixes.insert("i+[^[:alpha:]]*n+[^[:alpha:]]*g+");
		// diminutive
		suffixes.insert("l+[^[:alpha:]]*e+[^[:alpha:]]*t+");

		/* PREFIXES */

		// bz-specific
		prefixes.insert("b+[^[:alpha:]]*z+");
		prefixes.insert("b+[^[:alpha:]]*e+[^[:alpha:]]*z+");
		prefixes.insert("b+[^[:alpha:]]*e+[^[:alpha:]]*z+[^[:alpha:]]*e+");

		return;
	}

	/* destructor */
	~WordFilter(void) {
		int count = 0;
		for (std::set<badWord_t, expressionCompare>::iterator i = badWords.begin(); i != badWords.end(); ++i) {
			free(i->compiledWord);
		}

		return;
	}
	// and call regfree()


	// loads a set of bad words from a specified file
	void loadFromFile(const std::string &fileName) 
	{
		char buffer[2048];
		std::ifstream badWordStream(fileName.c_str());
		
		if (!badWordStream) {
			std::cerr << "Warning: '" << fileName << "' bad word filter file not found" << std::endl;
			return;
		}		
		
		while (badWordStream.good()) {
			badWordStream.getline(buffer,2048);
			
			std::string badWord = buffer;

			int position = badWord.find_first_not_of("\r\n\t ");

			// trim leading whitespace
			if (position > 0) {
				badWord = badWord.substr(position);
			} 

			position = badWord.find_first_of("#\r\n");

			// trim trailing comments
			if ((position >= 0) && (position < (int)badWord.length())) {
				badWord = badWord.substr(0, position);
			}

			position = badWord.find_last_not_of(" \t\n\r");
			// first whitespace is at next character position
			position += 1;

			// trim trailing whitespace
			if ((position >=0) && (position < (int)badWord.length())) {
				badWord = badWord.substr(0, position);
			}

#if 0
			std::cout << "[[[" <<	 badWord << "]]]" << std::endl;
#endif

			this->addWord(badWord);

		} // end iteration over input file

	} // end loadFromFile


	// adds an individual word to the filter list
	bool addWord(const std::string &word)
	{
		if ((word.c_str() == NULL)) {
			return false;
		} // end check if word non null
			
		long int length = (long int)word.length();

		if (0 >= length) {
			return false;
		} // end check if word is empty

		register regex_t *compiledReg;

		badWord_t newWord;
		newWord.word = word;
		
		/* create the regular expression description */
		const char * cword = word.c_str();
		char c[8], character;
		c[0] = c[1] = c[2] = c[3] = c[4] = c[5] = c[6] = c[7] = 0;

		/* match prefixes	 */
		//			newWord.expression.append("[(b+[^[:alpha:]]*z+[^[:alpha:]]*)|(b+[^[:alpha:]]*[e3]+[^[:alpha:]]*z+[^[:alpha:]]*[e3]+[^[:alpha:]]*)]*");


		/* beginning of a word */
		//		newWord.expression.append("\\<");

		for (int i = 0; i < length; i++) {
			// convert to lowercase for simplicity and speed
			character = c[0] = tolower(cword[i]);
			
			/* character classes for l33t-speak */
			if ( character == 'l' ) {
				c[1] = 'i';
				c[2] = '1';
				c[3] = '|';
				c[4] = '/';
				c[5] = '\\';
			} else if ( character == 'i' ) {
				c[1] = 'l';
				c[2] = '|';
				c[3] = '!';
				c[4] = '/';
				c[5] = '\\';
			} else if ( character == 'e' ) {
				c[1] = '3';
			} else if ( character == 's' ) {
				c[1] = '$';
				c[2] = 'z';
				c[3] = '5';
			} else if ( character == 't' ) {
				c[1] = '+';
			} else if ( character == 'c' ) {
				c[1] = '(';
			} else if ( character == 'a' ) {
				c[1] = '4';
				c[2] = '@';
			} else if ( character == 'b' ) {
				c[1] = '8';
			} else if ( character == 'o' ) {
				c[1] = '0';
			} else if ( character == 'g' ) {
				c[1] = '9';
			} else if ( character == 'z' ) {
				c[1] = 's';
			}
			// need to handle 'f' special for f=>(f|ph)
			
			/* ignore whitespace and non-alphanumerics */
			// ??? we might not want to ignore numbers (l33t in filter file)
			if ( ( character < 97 ) || ( character > 122 ) ) {
				continue;
			}
			
			/* append multi-letter expansions */
			if (character == 'f') {
				/* ensure we don't capture garbage at the end */
				if (i != length - 1) {
					newWord.expression.append("[fp]+[^[:alpha:]]*h?[^[:alpha:]]*");
				} else {
					newWord.expression.append("[fp]+h?");
				}
			} else {

				if ( c[1] != 0 ) {
					newWord.expression.append("[");
				}

				newWord.expression.append(c);
				if ( c[1] != 0 ) {
					newWord.expression.append("]");
				}
				/* ensure we don't capture garbage at the end */
				if (i != length - 1) {
					/* allow internal spacing/garbage expansion */
					newWord.expression.append("+[^[:alpha:]]*");
				} else {
					newWord.expression.append("+");
				}

			} // end test for multi-letter expansions
			
		} // end iteration over work letters
			
		//		std::cout << "[" <<	 newWord.expression << "]" << std::endl;

		/* !!! add all the suffixes & prefixes */
		/* end of a word */

		/* 1) change this to what it is "supposed" to be for suffixes and prefixes
		 * 2) check for duplication and in addWord()
		 * 3) check on performance on doing regex with no matching
		 */
		newWord.expression.append("(dom|ity|memt|sion|tion|ness|ance|ence|er|or|ist|ive|en|ic|al|able|y|ous|ful|less|en|ize|ate|ify|fy|ed|ly|ing|let|a|z|r|ah|io|rs|rz|in|n|ster|meister)?");
		//		newWord.expression.append("[^[:alpha:]]");
		newWord.expression = "(b+z+|b+e+z+e+)?" + newWord.expression;

		compiledReg = getCompiledExpression(newWord.expression);
		
		if (compiledReg == NULL) {
				std::cerr << "Warning: unable to add regular expression [" << word << "]" << std::endl;
				newWord.compiled = false;
		}

		newWord.compiledWord = compiledReg;
		newWord.compiled = true;


		// finally add the sucker
		badWords.insert(newWord);
		
		return true;
		
	} // end addWord


	

	// filters an input message either a complex regular expression-based
	// pattern match (default) catching hundreds of variations per filter 
	// word or using a simple exact word match technique (original).
	bool filter(char *input, bool simple=false) {
		if (simple) {
			std::cout << "NOT aggressive" << std::endl;
			return simpleFilter(input);
		}
		std::cout << "aggressive" << std::endl;
		return aggressiveFilter(input);
	}

	void outputWords(void) {
		std::cout << "size of badwords set is " << badWords.size() << std::endl;
		//		std::cout << "size of compiled set is " << () << std::endl;
		int count=0;
		for (std::set<badWord_t, expressionCompare>::iterator i = badWords.begin(); i != badWords.end(); ++i) {
			std::cout << count++ << ": " << i->word << std::endl;
		}

	}
};


#if UNIT_TEST
int main (int argc, char *argv[]) 
{

#if 0
	cout << "Checking callsign: " << player[playerIndex].callSign << endl;
	if ( clOptions.badwords.contains(player[playerIndex].callSign) ) {
		// this is a hack; would be better to add a new reject type
		player[playerIndex].team = NoTeam;
	}
	clOptions.badwords.filter(message);
#endif

	if (argc < 2) {
		std::cerr << "missing filename" << std::endl;
		return -1;
	}

	std::cout << "Loading file" << std::endl; 
	WordFilter badwords;
	badwords.loadFromFile(argv[1]);
	badwords.addWord("test");

	char message[1024] = " This test is a fucKing simple test; you're NOT a beezeebitch!! ";
	std::cout << message << std::endl;
	badwords.filter(message, true);
	std::cout << message << std::endl;
	//	badwords.outputWords();

	char message2[1024] = "f  u  c  k  !  fuuukking test ; you're NOT a beezeecun+y!!  Phuck you b1tch! ";
	char message3[1024] = "f  u  c  k  !  fuuukking test ; you're NOT a beezeecun+y!!  Phuck you b1tch! ";
	char message4[1024] = "f  u  c  k  !  fuuukking test ; you're NOT a beezeecun+y!!  Phuck you b1tch! ";
	char message5[1024] = "f  u  c  k  !  fuuukking test ; you're NOT a beezeecun+y!!  Phuck you b1tch! ";
	char message6[1024] = "f  u  c  k  !  fuuukking test ; you're NOT a beezeecun+y!!  Phuck you b1tch! ";
	char message7[1024] = "f  u  c  k  !  fuuukking test ; you're NOT a beezeecun+y!!  Phuck you b1tch! ";
	char message8[1024] = "f  u  c  k  !  fuuukking test ; you're NOT a beezeecun+y!!  Phuck you b1tch! ";
	char message9[1024] = "f  u  c  k  !  fuuukking test ; you're NOT a beezeecun+y!!  Phuck you b1tch! ";
	char message10[1024] = "f  u  c  k  !  fuuukking test ; you're NOT a beezeecun+y!!  Phuck you b1tch! ";
	char message11[1024] = "f  u  c  k  !  fuuukking test ; you're NOT a beezeecun+y!!  Phuck you b1tch! ";
	std::cout << message2 << std::endl;
	badwords.filter(message2);
	badwords.filter(message3);
	badwords.filter(message4);
	badwords.filter(message5);
	badwords.filter(message6);
	badwords.filter(message7);
	badwords.filter(message8);
	badwords.filter(message9);
	badwords.filter(message10);
	badwords.filter(message11);
	std::cout << message2 << std::endl;

	return 0;
}
#endif
