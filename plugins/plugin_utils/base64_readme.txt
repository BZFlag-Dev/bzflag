If you've never heard of it, base64 is a standard for storing binary data in a 7-bit clean format.  It's commonly used where only text is appropriate but binary files are desired, such as email and usenet.

I know it is a bit of a stretch to call this game-related, so here's my story.  I've been using <a href="http://tinyxml.sourceforge.net/">TinyXML</a> for data storage in my game project, and I wanted to move some binary data into the XML files.  The only problem is that TinyXML does not and will not support CDATA sections, which is XML's binary storage method.  Base64 fits the need nicely, and I decided to implement it myself just for kicks.  The result is this piece of work -- short and sweet.

The two functions, base64::encode and base64::decode, operate very similarly to std::copy.  Using the STL's iterator library, you can easily encode and decode to and from straight buffers, containers, or files via streams.  The functions take as parameters an input iterator range and an output iterator, and just as std::copy you can throw anything at it, from raw pointers to container iterators to istream_ and ostream_iterators.  

Here's some example usage:
<cpp>
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <cstdlib>
#include <cstring>

#include "base64.hpp"


using namespace std;

int main()
{
  const int size = 439;
  char source[size], source2[size];
  vector<char> dest;
  int i;

  // initialize
  for(i=0; i < size; ++i) source[i] = (rand() & 0xFF);

  // convert to base64
  base64::encode((char*)source, source+size, back_inserter(dest));

  // spit out the result
  copy(dest.begin(), dest.end(), ostream_iterator<char>(cout));
  cout << endl;

  // convert back to binary
  base64::decode(dest.begin(), dest.end(), source2);

  // make sure we got the same thing
  if (memcmp(source, source2, size) == 0)
    cout <<"They're the same!" << endl;
    else cout << "They're different!" << endl;


  // encode from memory to a file
  {
    ofstream file("b64.txt");
    base64::encode((char*)source, source+size, ostream_iterator<char>(file));
  }

  // decode from file to file
  {
    ifstream in("b64.txt");
    ofstream out("b64.bin", ios::binary);

    base64::decode(istream_iterator<char>(in), istream_iterator<char>(), 
                   ostream_iterator<char>(out)
                  );
  }
 
  return 0;
}
</cpp>

I'm releasing it under the zlib license -- basically, do whatever you want with it, just don't blame me for the results.  :)  Comments and feedback are welcome.  I had fun writing it, and if it's instructive or useful to anyone else, all the better.

Ryan Petrie (aka hurri)