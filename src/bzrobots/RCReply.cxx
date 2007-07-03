#include "RCReply.h"
#include "Roster.h"

#include "RCLink.h"
//#include "RCReplys.h"

#define ADD_LOOKUP(COMMAND) messageLookup[#COMMAND] = &instantiate<COMMAND ## Req>
/* These two are static functions that manipulate and access the lookup-
 * table for request commands -> request instances. :-) */
void RCReply::initializeLookup(void)
{
}

RCReply::~RCReply() { }
RCReply::RCReply(RCLink *_link) :RCMessage<RCReply>(_link)
{
}
