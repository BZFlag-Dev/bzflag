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

/* no header other than VotingBooth.h should be included here */

#ifdef _WIN32
#pragma warning( 4:4786)
#endif

#include "VotingBooth.h"


/* private */

/* protected */

/* public: */

VotingBooth::VotingBooth(void)
{
  return;
}

VotingBooth::~VotingBooth(void) 
{
  return;
}

vote_t VotingBooth::addOption(std::string option)
{
  vote_t id;

  return id;
}

vote_t VotingBooth::getOptionIDFromString(std::string name)
{
  vote_t id;

  return id;
}

std::string VotingBooth::getStringFromOptionID(vote_t id)
{
  std::string option;

  return option;
}

bool VotingBooth::vote(std::string name, vote_t id)
{
  return false;
}


#if UNIT_TEST
int main (int argc, char *argv[])
{
  VotingBooth poll;

  poll.vote("blah1", 0);
  poll.vote("blah2", 0);
  poll.vote("blah3", 0);
  poll.vote("blah1", 1);
  poll.vote("blah2", 0);
  poll.vote("blah3", 1);
  poll.vote("blah1", 2);
  poll.vote("blah2", 3);
  vote_t newOption = poll.addOption("maybe");
  std::cout << "new option maybe has id: " << newOption << std::endl;
  poll.vote("blah", 2);
  poll.vote("blah", 3);

  std::cout << "optionID for no is " << getOptionIDFromString("no") << std::endl;
  std::cout << "optionID for yes is " << getOptionIDFromString("yes") << std::endl;
  std::cout << "optionID for maybe is " << getOptionIDFromString("maybe") << std::endl;
  
  std::string *option;
  for (int i=0; i < 9; i++) {
    std::cout << "option " << i << " is " << poll.getStringFromOptionID(i) << std::endl;    
  }

  return 0;
}
#endif
// ex: shiftwidth=2 tabstop=8
