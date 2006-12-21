// torBlock.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include <string>
#include <algorithm>
#include <sstream>
#include <stdarg.h>
#include <vector>
#include <stdio.h>
#include <assert.h>
#include <map>
#include <vector>

inline std::string tolower(const std::string& s)
{
	std::string trans = s;

	for (std::string::iterator i=trans.begin(), end=trans.end(); i!=end; ++i)
		*i = ::tolower(*i);
	return trans;
}

std::string format(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char	temp[2048];
	vsprintf(temp,fmt, args);
	std::string result = temp;
	va_end(args);
	return result;
}

std::vector<std::string> tokenize(const std::string& in, const std::string &delims, const int maxTokens, const bool useQuotes){
	std::vector<std::string> tokens;
	int numTokens = 0;
	bool inQuote = false;

	std::ostringstream currentToken;

	std::string::size_type pos = in.find_first_not_of(delims);
	int currentChar  = (pos == std::string::npos) ? -1 : in[pos];
	bool enoughTokens = (maxTokens && (numTokens >= (maxTokens-1)));

	while (pos != std::string::npos && !enoughTokens) {

		// get next token
		bool tokenDone = false;
		bool foundSlash = false;

		currentChar = (pos < in.size()) ? in[pos] : -1;
		while ((currentChar != -1) && !tokenDone){

			tokenDone = false;

			if (delims.find(currentChar) != std::string::npos && !inQuote) { // currentChar is a delim
				pos ++;
				break; // breaks out of while loop
			}

			if (!useQuotes){
				currentToken << char(currentChar);
			} else {

				switch (currentChar){
	  case '\\' : // found a backslash
		  if (foundSlash){
			  currentToken << char(currentChar);
			  foundSlash = false;
		  } else {
			  foundSlash = true;
		  }
		  break;
	  case '\"' : // found a quote
		  if (foundSlash){ // found \"
			  currentToken << char(currentChar);
			  foundSlash = false;
		  } else { // found unescaped "
			  if (inQuote){ // exiting a quote
				  // finish off current token
				  tokenDone = true;
				  inQuote = false;
				  //slurp off one additional delimeter if possible
				  if (pos+1 < in.size() &&
					  delims.find(in[pos+1]) != std::string::npos) {
						  pos++;
					  }

			  } else { // entering a quote
				  // finish off current token
				  tokenDone = true;
				  inQuote = true;
			  }
		  }
		  break;
	  default:
		  if (foundSlash){ // don't care about slashes except for above cases
			  currentToken << '\\';
			  foundSlash = false;
		  }
		  currentToken << char(currentChar);
		  break;
				}
			}

			pos++;
			currentChar = (pos < in.size()) ? in[pos] : -1;
		} // end of getting a Token

		if (currentToken.str().size() > 0){ // if the token is something add to list
			tokens.push_back(currentToken.str());
			currentToken.str("");
			numTokens ++;
		}

		enoughTokens = (maxTokens && (numTokens >= (maxTokens-1)));
		if (enoughTokens){
			break;
		} else{
			pos = in.find_first_not_of(delims,pos);
		}

	} // end of getting all tokens -- either EOL or max tokens reached

	if (enoughTokens && pos != std::string::npos) {
		std::string lastToken = in.substr(pos);
		if (lastToken.size() > 0)
			tokens.push_back(lastToken);
	}

	return tokens;
}

BZ_GET_PLUGIN_VERSION

std::string torMasterList("http://belegost.mit.edu/tor/status/authority");

double lastUpdateTime = -99999.0;
double updateInterval = 60.0;

std::vector<std::string>	exitNodes;

class Handler : public bz_EventHandler
{
public:
	virtual void process ( bz_EventData *eventData );
};

Handler handler;

class MyURLHandler: public bz_URLHandler
{
public:
	std::string page;
	virtual void done ( const char* URL, void * data, unsigned int size, bool complete )
	{
		char *str = (char*)malloc(size+1);
		memcpy(str,data,size);
		str[size] = 0;

		page += str;
		if (!complete)
			return;

		std::vector<std::string> tokes = tokenize(page,std::string("\n"),0,false);

		bool gotKey = false;
		for (unsigned int i = 0; i < tokes.size(); i++ )
		{
			if (!gotKey)
			{
				if ( tokes[i] == "-----END RSA PUBLIC KEY-----")
				{
					gotKey = true;
					exitNodes.clear(); // only clear when we have a list
				}
			}
			else
			{
				if ( tokes[i].size() )
				{
					std::vector<std::string> chunks = tokenize(tokes[i],std::string(" "),0,false);

					if ( chunks.size() > 1 )
					{
						if ( chunks[0] == "r" && chunks.size() > 7 )
							exitNodes.push_back(chunks[6]);
					}
				}
			}
		}
	}
};


class mySlashCommand : public bz_CustomSlashCommandHandler
{
public:
	virtual bool handle ( int playerID, bzApiString command, bzApiString message, bzAPIStringList *params )
	{
		bz_sendTextMessage(BZ_SERVER,playerID,"torBlock List");
		for ( unsigned int i = 0; i < exitNodes.size(); i++ )
			bz_sendTextMessage(BZ_SERVER,playerID,exitNodes[i].c_str());

		return true;
	}
};

mySlashCommand mySlash;
MyURLHandler myURL;

void updateTorList ( void )
{
	if ( bz_getCurrentTime() - lastUpdateTime >= updateInterval)
	{
		lastUpdateTime = bz_getCurrentTime();
		myURL.page.clear();
		bz_addURLJob(torMasterList.c_str(),&myURL);
	}
}

bool isTorAddress ( const char* addy )
{
	for ( unsigned int i = 0; i < exitNodes.size(); i++ )
	{
		if (exitNodes[i] == addy)
			return true;
	}
	return false;
}

BZF_PLUGIN_CALL int bz_Load ( const char* /*commandLine*/ )
{
	bz_debugMessage(4,"torBlock plugin loaded");
	bz_registerEvent(bz_eAllowPlayer,&handler);
	bz_registerEvent(bz_eTickEvent,&handler);
	bz_registerCustomSlashCommand("torlist",&mySlash);
	lastUpdateTime = -updateInterval * 2;
	return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
	bz_removeCustomSlashCommand("torlist");
	bz_removeEvent(bz_eTickEvent,&handler);
	bz_removeEvent(bz_eAllowPlayer,&handler);
	bz_debugMessage(4,"torBlock plugin unloaded");
	return 0;
}

void Handler::process ( bz_EventData *eventData )
{
	if (!eventData)
		return;

	switch (eventData->eventType)
	{
		case bz_eAllowPlayer:
			{
				bz_AllowPlayerEventData *data = (bz_AllowPlayerEventData*)eventData;

				if (isTorAddress(data->ipAddress.c_str()))
				{
					data->allow = false;
					data->reason = "Proxy Network Ban";
				}
			}
			break;

		case bz_eTickEvent:
			updateTorList();
			break;

		default:
			break;
	}
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

