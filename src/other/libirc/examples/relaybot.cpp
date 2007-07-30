

#include <string>
#include <vector>
#include <map>

class relayBotEventHandler : public IRCBasicEventCallback {

  virtual bool process (IRCClient &ircClient, teIRCEventType eventType, trBaseEventInfo &info);

};

class relayBot {
 public:
  relayBot();

  bool init(); //connect, join, etc

  ~ircControl();

 private:

  IRCClient client;
  relayBotEventHandler handler;

  std::string server;
  int port;

  std::string nick; //TODO: Er, multiple nicks would probably be a good idea. Whoops...
  std::string pwd;

  bool connected;

  



};

bool relayBotEventHandler::process(IRCClient &ircClient, teIRCEventType eventType, trBaseEventInfo &info) {



}

bool relayBot::init() {

  //register events... we don't really care about most of them. This bot is simple. ;)

  //client.registerEventHandler(eIRCNoticeEvent, handler);
  client.registerEventHandler(eIRCNickNameError, handler);
  //client.registerEventHandler(eIRCNickNameChange, handler);
  //client.registerEventHandler(eIRCWelcomeEvent, handler);
  //client.registerEventHandler(eIRCEndMOTDEvent, handler);
  client.registerEventHandler(eIRCChannelJoinEvent, handler);
  client.registerEventHandler(eIRCChannelPartEvent, handler);
  client.registerEventHandler(eIRCChannelBanEvent, handler);
  client.registerEventHandler(eIRCChannelMessageEvent, handler);
  client.registerEventHandler(eIRCPrivateMessageEvent, handler);
  //client.registerEventHandler(eIRCTopicEvent, handler);
  //client.registerEventHandler(eIRCUserJoinEvent, handler);
  //client.registerEventHandler(eIRCUserPartEvent, handler);
  //client.registerEventHandler(eIRCUserKickedEvent, handler);
  //client.registerEventHandler(eIRCTopicChangeEvent, handler);
  client.registerEventHandler(eIRCChanInfoCompleteEvent, handler);
  client.registerEventHandler(eIRCChannelModeSet, handler);
  client.registerEventHandler(eIRCChannelUserModeSet, handler);
  client.registerEventHandler(eIRCUserModeSet, handler);
  client.registerEventHandler(eIRCQuitEvent, handler);

  //connect

  //join channels

}