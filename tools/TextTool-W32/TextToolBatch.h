#include <string>
#include <vector>

struct BatchItem {
  std::string font;
  int size;
  bool bold;
  bool italic;
  std::string filename;
};

class TextToolBatch {
public:
  TextToolBatch(std::string file);
  TextToolBatch();
  ~TextToolBatch();
  
  void loadFile(std::string file);
  
  bool getNext(BatchItem& item);

  void error(std::string msg);

private:
  unsigned int position;
  std::vector<BatchItem> items;
};

