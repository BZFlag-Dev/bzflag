#include <string>
#include <vector>
#include <fstream>
#include <iostream>

TextToolBatch::TextToolBatch() : position(0)
{
}

TextToolBatch::TextToolBatch(std::string file) : position(0)
{
  loadFile(file);
}

TextToolBatch::loadFile(std::string file)
{
  std::ifstream input = new std::ifstream(file.c_str(), std::ios::in);

  char* buffer[256];
  bool group = false;

  std::string font;
  std::string flags;
  std::vector<int> sizes;
  std::string filename;

  while (input.isGood()) { 
    buffer[0] = 0;    

    // trim leading ws
    input.eatwhite();

    // read a line
    input.readline(buffer);

    // ignore and read next line if comment or blank
    if (buffer[0] == 0) continue;
    if (buffer[0] == '#') continue;

    if (strcmp(buffer, "group") == 0) {
      // error if "group" and already in group
      if (group) error("group: Already in group");
      // set flag and read next line if "group" and not already in group
      else group = true;
    } else if (strcmp(buffer, "end") == 0) {
      // error if "end" and not in group
      if (!group) error("end: Not in group");
      // unset flag, create items, and read next line if "end" and in group
      else {
        group = false;
        for (i = 0; i < sizes.count(); i++) {
	  BatchItem temp = new BatchItem;
	  // set font face
          temp.font = font;
	  // set font flags (just bold and italic; underline and strikethrough are just lines)
	  if (flags.find(0, "bold")) temp.bold = true else temp.bold = false;
	  if (flags.find(0, "italic")) temp.italic = true else temp.italic = false;
	  // set font size
          temp.size = sizes[i];
	  // set filename (with $s -> size transform)
	  char* buf = new char[5];
	  sprintf(buf, "%d", sizes[i]);
	  std::string tmp = filename;
	  tmp.replace("$s", buf);
	  temp.filename = tmp;
	  items.push_back(temp);
	}
      }  
    }
  
    // error if not in group
    if (!group) error("command: Not in group");

    // parse "font", "flags", "sizes", "filename" into the structure
    if (strcmp(buffer, "font") == 0) {
      font = std::string(buffer + 5);
    } else if (strcmp(buffer, "flags") == 0) {
      flags = std::string(buffer + 6);
    } else if (strcmp(buffer, "sizes") == 0) {
      std::string tmp = std::string(buffer + 6);
      int x = tmp.find(0, " ");
      while (x < tmp.size()) {
        y = tmp.find(x + 1, " ");
	sizes.push_back(atoi(tmp.substr(x, y)));
	x = y;
      }
    } else if (strcmp(buffer, "filename") == 0) {
      filename = std::string(buffer + 9);
    } else {
      error("command: unrecognized");
    }

  }
}

bool TextToolBatch::getNext(BatchItem& item)
{
  if (position >= items.count()) 
    return false;

  item = items[position++];
  return true;
}

TextToolBatch::error(std::string msg)
{
  std::cerr << msg << std::endl;
  exit(-2);
}

