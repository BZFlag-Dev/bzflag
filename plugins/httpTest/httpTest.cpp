// httpTest.cpp : Defines the entry point for the DLL application.
//

#include <map>
#include "bzfsAPI.h"
#include "plugin_utils.h"

BZ_GET_PLUGIN_VERSION

class HTTPHandler : public bz_NonPlayerConnectionHandler
{
public:
	virtual void pending ( int connectionID, void *data, unsigned int size )
	{
		std::string oldData;

		if ( pendingData.find(connectionID) != pendingData.end() )
			oldData = pendingData[connectionID];

		char *str = (char*)malloc(size+1);
		memcpy(str,data,size);
		str[size] = 0;

		oldData += str;
		free(str);

		pendingData[connectionID] = oldData;

		if ( strstr(oldData.c_str(),"\r\n\r\n") )
		{
			// it's done, lets parse it

			std::vector<std::string> items = tokenize(oldData,std::string ("\r\n"),0,false);
			for (unsigned int i = 0; i < items.size(); i++ )
			{
				std::string item = items[i];

				std::vector<std::string> chunks = tokenize(item,std::string(" "),0,true);

				if (chunks.size() && tolower(chunks[0]) == "get")
				{
					std::string returnPage = "This Is data from HTTP via BZFS\r\n\r\n";
					bz_sendNonPlayerData(connectionID,returnPage.c_str(),(unsigned int)returnPage.size());
				}
			}
			// we are done with it
			bz_removeNonPlayerConnectionHandler(connectionID,this);
			//bz_disconectNonPlayerConnection(connectionID);

			pendingData.erase(pendingData.find(connectionID));
		}
	}

	std::map<int,std::string>	pendingData;
};

HTTPHandler http;

class NewConnectionHandler : public bz_EventHandler
{
public:
	virtual void process ( bz_EventData *eventData )
	{
		bz_NewNonPlayerConnectionEventData_V1 *data = (bz_NewNonPlayerConnectionEventData_V1*)eventData;

		bz_registerNonPlayerConnectionHandler(data->connectionID,&http);
		http.pending(data->connectionID,data->data,data->size);
	}
};

NewConnectionHandler newConnect; 

BZF_PLUGIN_CALL int bz_Load ( const char* /*commandLine*/ )
{
  bz_debugMessage(4,"httpTest plugin loaded");
  bz_registerEvent(bz_eNewNonPlayerConnection,&newConnect);
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  bz_debugMessage(4,"httpTest plugin unloaded");
  bz_removeEvent(bz_eNewNonPlayerConnection,&newConnect);
 return 0;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
