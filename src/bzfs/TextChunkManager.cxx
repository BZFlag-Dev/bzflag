#include "TextChunkManager.h"

bool TextChunkManager::parseFile(const std::string &fileName, const std::string &chunkName)
{
	char buffer[MessageLen];
	ifstream in(fileName.c_str());
	if (!in) return false;
	
	StringVector strings;
	for(int i = 0; i < 20 && in.good(); i++) {
		in.getline(buffer,MessageLen);
		if(!in.fail()){ // really read something
			strings.push_back(buffer);
		}
	}
	
	if (strings.size() != 0) {
		theChunks.insert(std::map<std::string, StringVector>::value_type(chunkName,strings));
		chunkNames.push_back(chunkName);
	}
	return true;
}

const std::vector<std::string>* TextChunkManager::getTextChunk(const std::string chunkName)
{
	std::map<std::string, StringVector>::const_iterator it;
	it =theChunks.find(chunkName);
	if (it != theChunks.end()){
		return &it->second.getVector();
	} else {
		return NULL;
	}
}

