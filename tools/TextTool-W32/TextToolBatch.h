#include <string>
#include <vector>

struct BatchItem {
  std::string font;
  int size;
  bool bold;
  bool italic;
  std::string filename;
}

class TextToolBatch {
public:
  TextToolBatch(std::string file);
  TextToolBatch();
  ~TextToolBatch();
  
  loadFile(std::string file);
  
  bool getNext(BatchItem& item);

private:
  int position;
  std::vector<BatchItem> items;
}

