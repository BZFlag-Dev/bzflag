/* bzflag
 * Copyright (c) 1993-2023 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "VotingArbiter.h"

/* common implementation headers */
#include "GameKeeper.h"
#include "TextUtils.h"


/* private */

/* protected */

void VotingArbiter::updatePollers(void)
{
    while (!_pollers.empty())
    {
        poller_t& p = _pollers.front();
        if ((TimeKeeper::getCurrent() - p.lastRequest) > _voteRepeatTime)
        {
            // remove pollers that expired their repoll timeout
            _pollers.pop_front();
        }
        else
            break;
    }
    return;
}

bool VotingArbiter::isPollerWaiting(const std::string &name) const
{
    for (unsigned int i = 0; i < _pollers.size(); i++)
    {
        if (TextUtils::compare_nocase(_pollers[i].name, name) == 0)
            return true;
    }
    return false;
}


/* public */

bool VotingArbiter::forgetPoll(void)
{
    if (_votingBooth != NULL)
    {
        delete _votingBooth;
        _votingBooth = NULL;
    }
    _startTime = TimeKeeper::getNullTime();
    _pollee = "nobody";
    _polleeIP = "";
    _action = "";
    _pollOrganizer = BZ_SERVER;

    /* poof */
    _suffraged.clear();

    return true;
}

bool VotingArbiter::poll(const std::string &target, int playerRequestingID, std::string action, std::string playerIP)
{
    GameKeeper::Player *pr = GameKeeper::Player::getPlayerByIndex(playerRequestingID);
    std::string playerRequesting = pr->player.getCallSign();

    poller_t p;
    std::string message;
    bool tooSoon;

    // you have to forget the current poll before another can be spawned
    if (this->isPollOpen())
        return false;

    // update the poller list (people on the pollers list cannot initiate a poll
    updatePollers();

    // see if the poller is in the list
    tooSoon = isPollerWaiting(playerRequesting);
    if (tooSoon)
        return false;

    // add this poller to the end list
    p.name = playerRequesting;
    p.lastRequest = TimeKeeper::getCurrent();
    _pollers.push_back(p);

    // create the booth to record votes
    message = target + " " + action;

    if (_votingBooth != NULL)
        delete _votingBooth;
    _votingBooth = YesNoVotingBooth(message);
    _pollee = target;
    _polleeIP = playerIP;
    _action = action;
    _pollOrganizer = playerRequestingID;

    // set timers
    _startTime = TimeKeeper::getCurrent();

    return true;
}

bool VotingArbiter::pollToKick(const std::string &victim, int playerRequestingID, const std::string &playerIP)
{
    return (this->poll(victim, playerRequestingID, std::string("kick"), playerIP));
}

bool VotingArbiter::pollToKill(const std::string &victim, int playerRequestingID, const std::string &playerIP)
{
    return (this->poll(victim, playerRequestingID, std::string("kill"), playerIP));
}

bool VotingArbiter::pollToBan(const std::string &victim, int playerRequestingID, const std::string &playerIP)
{
    return (this->poll(victim, playerRequestingID, std::string("ban"), playerIP));
}

bool VotingArbiter::pollToSet(const std::string &setting, int playerRequestingID)
{
    return (this->poll(setting, playerRequestingID, std::string("set")));
}

bool VotingArbiter::pollToResetFlags(int playerRequestingID)
{
    std::string flags=std::string("flags");
    return (this->poll(flags, playerRequestingID, std::string("reset")));
}

bool VotingArbiter::closePoll(void)
{
    if (this->isPollClosed())
        return true;
    // set starting time to exactly current time minus necessary vote time
    _startTime = TimeKeeper::getCurrent();
    _startTime += -(float)(_voteTime);

    return true;
}

bool VotingArbiter::setAvailableVoters(unsigned short int count)
{
    _maxVotes = count;
    return true;
}

bool VotingArbiter::grantSuffrage(const std::string &player)
{
    for (unsigned int i = 0; i < _suffraged.size(); i++)
    {
        if (TextUtils::compare_nocase(_suffraged[i], player) == 0)
            return true;
    }
    _suffraged.push_front(player);
    return true;
}

bool VotingArbiter::hasSuffrage(const std::string &player) const
{
    // is there a poll to vote on?
    if (!this->isPollOpen())
        return false;

    // was this player granted the right to vote?
    bool foundPlayer = false;
    for (unsigned int i = 0; i < _suffraged.size(); i++)
    {
        if (TextUtils::compare_nocase(_suffraged[i], player) == 0)
        {
            foundPlayer = true;
            break;
        }
    }
    if (!foundPlayer)
        return false;

    // has this player already voted?
    if (_votingBooth->hasVoted(TextUtils::tolower(player)))
        return false;

    // are there too many votes somehow (sanity)
    if (_votingBooth->getTotalVotes() >= _maxVotes)
        return false;

    return true;
}

bool VotingArbiter::hasVoted(const std::string &player) const
{
    return _votingBooth->hasVoted(TextUtils::tolower(player));
}

bool VotingArbiter::voteYes(const std::string &player)
{
    if (!this->knowsPoll() || this->isPollClosed())
        return false;

    // allowed to vote?
    if (!hasSuffrage(player))
        return false;

    return (_votingBooth->vote(TextUtils::tolower(player), "yes"));
}

bool VotingArbiter::voteNo(const std::string &player)
{
    if (!this->knowsPoll() || this->isPollClosed())
        return false;

    // allowed to vote?
    if (!hasSuffrage(player))
        return false;

    return (_votingBooth->vote(TextUtils::tolower(player), "no"));
}

unsigned long int VotingArbiter::getYesCount(void) const
{
    if (!this->knowsPoll())
        return 0;
    return _votingBooth->getVoteCount("yes");
}

unsigned long int VotingArbiter::getNoCount(void) const
{
    if (!this->knowsPoll())
        return 0;
    return _votingBooth->getVoteCount("no");
}

unsigned long int VotingArbiter::getAbstentionCount(void) const
{
    // cannot abstain if there is no poll
    if (!this->knowsPoll())
        return 0;
    int count = int(_suffraged.size() - this->getYesCount() - this->getNoCount());
    if (count <= 0)
        return 0;
    return count;
}

bool VotingArbiter::isPollSuccessful(void) const
{
    if (!this->knowsPoll())
        return false;
    unsigned long int yesVotes = this->getYesCount();
    unsigned long int noVotes = this->getNoCount();
    unsigned long int abstainVotes = this->getAbstentionCount();
    double total = (double)yesVotes + (double)noVotes + (double)abstainVotes;

    // ensure minimum votage (and sanity zero-div check)
    if ((yesVotes + noVotes < _votesRequired) || (yesVotes + noVotes == 0))
        return false;

    //  std::cout << "Percentage successful is " << ((double)votes * (double)100.0 / (double)_maxVotes) << " with " << votes << " votes and " << _maxVotes << "max votes required" << std::endl;

    // were there enough votes?
    if (( (double)100.0 * (double)yesVotes / total) >= (double)_votePercentage)
        return true;

    return false;
}

bool VotingArbiter::isPollSureToFail(void) const
{
    if (!this->knowsPoll())
    {
        return true; // hrm, maybe false
    }

    unsigned long int yesVotes = this->getYesCount();
    unsigned long int noVotes = this->getNoCount();
    unsigned long int abstainVotes = this->getAbstentionCount();
    double total = (double)yesVotes + (double)noVotes + (double)abstainVotes;

    // there is a poll, but nobody is allowed to vote
    if (total <= 0.1)
        return true;

    // were there enough no votes to ensure failure?
    if (( (double)100.0 * (double)noVotes / total) >= (double)100.0 - (double)_votePercentage)
        return true;

    return false;
}

unsigned short int VotingArbiter::timeRemaining(void) const
{
    if (_votingBooth == NULL)
        return 0;

    // if the poll is successful early, terminate the clock
    if (this->isPollSuccessful())
        return 0;

    float remaining = _voteTime - (float)(TimeKeeper::getCurrent() - _startTime);
    if (remaining < 0.0f)
        return 0;
    return (unsigned short int)remaining;
}

bool VotingArbiter::retractVote(const std::string &player)
{
    if (_votingBooth == NULL)
        return false;
    return _votingBooth->retractVote(TextUtils::tolower(player));
}

// Custom Poll Types

std::map<std::string, PollType> customPollTypes;

void registerCustomPollType ( const char* object, const char* parameters, bz_CustomPollTypeHandler *handler )
{
    PollType o;
    o.pollParameters = parameters;
    o.pollHandler = handler;

    std::string objectName = object;

    customPollTypes[TextUtils::tolower(objectName)] = o;
}

void removeCustomPollType ( const char* object )
{
    std::string objectName = object;
    objectName = TextUtils::tolower(objectName);

    if (customPollTypes.find(objectName) != customPollTypes.end())
        customPollTypes.erase(customPollTypes.find(objectName));
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
