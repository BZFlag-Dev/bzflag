#ifndef __WORDFILTER_H__
#define __WORDFILTER_H__

#include <string>
#include <vector>
#include <set>
#include <fstream>
#include <iostream>

#include <sys/types.h>
#include <regex.h>

/* !!! temporary */
#define bzfrand()	((double)rand() / ((double)RAND_MAX + 1.0))

/* WordFilter will load a list of words and phrases from a file or one at
 * a time or manually.	
 *
 * Loading from file assumes that words/phrases are listed one per line;
 * comments are possible using the shell-style "#" delimiter.	 Words are 
 * matched case-insensitive; punctuation and non-newline white-space are 
 * always ignored with the default filter.
 *
 * By default (aggressive filtering), the filter will match many
 * additional combinations to avoid the need to list all potential spelling
 * variations and hacks in the filter file.
 *
 * e.g. "dumb ass" should match "dumbass!", "DUMB a s s", "d_u_m_b! a_S_S", etc
 *
 * Strings should also match repetitive identical letter expansions.
 *
 * e.g. "dumb ass" should match "dumb asssss", "dumbaaaass", "dumb as s", etc
 *
 * Strings should also match l33t-speak. (l=1, o=0, e=3, a=@, s=z, i=l, f=ph)
 *
 * e.g. "ass whipe" should match "@sz wh1p3", etc
 *
 * Strings should also match common word suffixes (at least for English)
 *	 (dom|ity|memt|sion|tion|ness|ance|ence|er|or|ist) for nouns
 *	 (ive|en|ic|al|able|y|ous|ful|less) for adjectives
 *	 (en|ize|ate|ify|fy|ed) for verbs
 *	 (ly) for adverbs
 *	 (a|z|r|ah|io|rs|rz|in|n|ster|meister) for slang
 *	 (s|es) for plurality
 *	 (ing|let) for imperfect verb, and diminutive
 *
 * e.g. "dumb ass" should match "dumb assness", "dumb asses", "dumb assly", etc
 *
 * Strings should also match common word prefixes (at least for English)
 *	 (bz|beze) for bzflag-centric words
 *
 * e.g. "bitch" should also match "bzbitch", "beezzeebitch", etc
 *
 * Since all of the above matchings are done for free with aggressive matching,
 * only root words need to be provided.	 For foreign languages, it may be
 * necessary to list all tenses of certain verbs, unless the rules can be
 * strictly and simply quantified.
 * 
 * There is also a simple filter mode which is not as resource intensive and
 * performs a literal match with the filter words (so you have to specify 
 * absolutely everything you want to filter and all variations).	It is still
 * case-insensitive and ignores punctuation.
 */

#endif
class WordFilter;
