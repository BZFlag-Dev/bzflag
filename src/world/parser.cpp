#include <boost/spirit/core.hpp>
#include <boost/spirit/iterator/multi_pass.hpp>

#include <vector>
#include <map>
#include <iostream>

using namespace std;
using namespace boost;
using namespace boost::spirit;

struct Debug
{
  Debug() : s(""){}
  Debug(string _s) : s(_s){}
  template <typename T>
    void operator()( T a, T b ) const
    {
      action(string(a,b));
    }
  template <typename T>
    void operator()( T a ) const
    {
      action(a);
    }
  void action(string a) const
  {
    cout << s << a << endl;
  }
  string s;
};

//classes
class Parser
{
  public:
    Parser() : hasParsed(false), current(NULL) { }
    ~Parser(){ }
    //classes
    class Block
    {
      public:
        Block() : unique(false), current(NULL) { }
        Block(string _identifier) : unique(false), current(NULL) { identifier = _identifier;}
        ~Block(){ }

        string getIdentifier(){ return identifier; }
        void setIdentifier(string _identifier){ identifier = _identifier; }
        bool getUnique(){ return unique; }
        void setUnique(bool _unique){ unique = _unique; }
        string getExtra(){ return extra; }
        void setExtra(string _extra){ extra = _extra; }
        //classes
        class Parameter
        {
          public:
            Parameter() : unique(false) { }
            Parameter(string _identifier) : unique(false) { identifier = _identifier; }
            ~Parameter(){ }
            string getIdentifier(){ return identifier; }
            void setIdentifier(string _identifier){ identifier = _identifier; }
            void setUnique(bool _unique){ unique = _unique; }
            bool getUnique(){ return unique; }
            void pushBackValue(string s)
            {
              values.push_back(s);
            }
            //
          protected:
            struct AddValue
            {
              AddValue(Parser& _parser) : parser(_parser){}
              template <typename IteratorT>
                void operator()( IteratorT a, IteratorT b) const
                {
                  action(string(a, b));
                }
              template <typename IteratorT>
                void operator()( IteratorT a) const
                {
                  action(a);
                }

              void action(string s) const
              {
                Block* block = parser.getCurrent();
                if(block)
                {
                  Parameter* parameter = block->getCurrent();
                  if(parameter)
                    parameter->pushBackValue(s);
                }
              }

              Parser& parser;
            };

          private:
            string identifier;
            bool unique;
            vector<string> values;

        };
        //end classes

        void registerParameter(Parameter* parameter)
        {
          registeredParameters[parameter->getIdentifier()] = parameter;
        }


        Parameter* getCurrent(){ return current; }

      protected:
        struct AddExtra
        {
          AddExtra(Parser& _parser) : parser(_parser){}
          template <typename IteratorT>
            void operator()( IteratorT a, IteratorT b) const
            {
              action(string(a,b));
            }

          template <typename IteratorT>
            void operator()( IteratorT a) const
            {
              action(a);
            }

          void action(string s) const
          {
            Block* block = parser.getCurrent();
            if(block)
              block->setExtra(s);
          }

          Parser& parser;
        };

        struct AddParameter
        {
          AddParameter(Parser& _parser) : parser(_parser){}
          template <typename IteratorT>
            void operator()( IteratorT a, IteratorT b) const
            {
              action(string(a, b));
            }

          template <typename IteratorT>
            void operator()( IteratorT a) const
            {
              action(a);
            }

          void action(string s) const
          {
            Block* block = parser.getCurrent();
            if(block)
              block->addParameter(s);
          }

          Parser& parser;
        };

        bool addParameter(string identifier)
        {
          current = NULL;
          if((registeredParameters_i = registeredParameters.find(identifier)) != registeredParameters.end())
          {
            if(registeredParameters_i->second->getUnique() && (parsedParameters_i = parsedParameters.find(identifier)) != parsedParameters.end())
            {
              cerr << "Error[REUP]: Repeat entry for unique parameter." << endl;
              return false;
            }
            current = &(parsedParameters.insert( pair<string, Parameter>( identifier, Parameter( *(registeredParameters_i->second) ) ) )->second);
            return true;
          }
          else
          {
            cerr << "Error[EURP]: Entered un-registered parameter." << endl;
            return false;
          }
        }

      private:
        string identifier;
        string extra;
        bool unique;
        map<string, Parameter*>::iterator registeredParameters_i;
        map<string, Parameter*> registeredParameters;
        multimap<string, Parameter>::iterator parsedParameters_i;
        multimap<string, Parameter> parsedParameters;
        Parameter* current;
    };
    //end classes

    void registerBlock(Block* block)
    {
      registeredBlocks[block->getIdentifier()] = block;

    }

    void parseBZW(istream& is); /*ENTRY*/

    Block* getCurrent(){ return current; }
  protected:
    struct AddBlock
    {
      AddBlock(Parser& _parser) : parser(_parser){}
      template <typename IteratorT>
        void operator()( IteratorT a, IteratorT b ) const
        {
          parser.addBlock(string(a, b));
        }

      template <typename IteratorT>
        void operator()( IteratorT a ) const
        {
          parser.addBlock(a);
        }
      Parser& parser;
    };

    bool addBlock(string identifier)
    {
      current = NULL;
      if((registeredBlocks_i = registeredBlocks.find(identifier)) != registeredBlocks.end())
      {
        if(registeredBlocks_i->second->getUnique() && (parsedBlocks_i = parsedBlocks.find(identifier)) != parsedBlocks.end())
        {
          cerr << "Error[REUB]: Repeat entry for unique block." << endl;
          return false;
        }
        current = &(parsedBlocks.insert( pair<string, Block>( identifier, Block( *(registeredBlocks_i->second) ) )  )->second);
        return true;
      }
      else
      {
        cerr << "Error[EURB]: Entered un-registered block." << endl;
        return false;
      }
    }

  private:
    bool hasParsed;
    map<string, Block*>::iterator registeredBlocks_i;
    map<string, Block*> registeredBlocks;
    multimap<string, Block>::iterator parsedBlocks_i;
    multimap<string, Block> parsedBlocks;
    Block* current;

    /* spirit grammar */
    struct bzw_grammar : grammar<bzw_grammar>
    {
      bzw_grammar(Parser& _parser)
        : parser(_parser) {}

      template<typename ScannerT>
        struct definition
        {
          rule<ScannerT> identifier;
          rule<ScannerT> parameter_value;
          rule<ScannerT> string_literal;
          rule<ScannerT> line_end;
          rule<ScannerT> parameter;
          rule<ScannerT> parameter_list;
          rule<ScannerT> block;
          rule<ScannerT> block_end;
          rule<ScannerT> block_list;

          definition(bzw_grammar const& self)
          {
            identifier = lexeme_d[
              alpha_p >> *(alnum_p | ch_p('_'))
            ] - block_end;

            parameter_value = lexeme_d[
              graph_p >> *(graph_p - ch_p('#'))
            ];

            string_literal = lexeme_d[
              graph_p >> *(*blank_p >> (graph_p - ch_p('#')))
            ];

            line_end = +eol_p;

            parameter
              = identifier[Block::AddParameter(self.parser)]
              >>
                *(
                    parameter_value[Block::Parameter::AddValue(self.parser)]
                 )
              >> line_end
              ;

            parameter_list
              = *parameter
              >> block_end
              >> line_end
              ;

            block
              = identifier[AddBlock(self.parser)]
              >>
                !(
                    string_literal[Block::AddExtra(self.parser)]
                 )
              >> line_end
              >> parameter_list
              ;

            block_end
              = str_p("end")
              ;

            block_list = *( line_end | block );
          }

          rule<ScannerT> const& start() { return block_list; }
        };
      Parser& parser;
    };

    struct bzw_skip_grammar : grammar<bzw_skip_grammar>
    {
      template <typename ScannerT>
        struct definition
        {
          rule<ScannerT> skip;

          definition(bzw_skip_grammar const& self)
          {
            skip
              = blank_p
              | "#" >> *(anychar_p - eol_p)
              ;
          }

          rule<ScannerT> const& start() const { return skip; }
        };
    };
    /* end spirit grammar */
};

void Parser::parseBZW(istream& is)
{
  istreambuf_iterator<char> stream_begin(cin);
  multi_pass<istreambuf_iterator<char> > begin(make_multi_pass(stream_begin)), end;
  bzw_grammar g(*this);
  bzw_skip_grammar s;
  parse(begin, end, g, s);
}
//end classes


int main(int argc, char ** argv)
{
  Parser p;
    Parser::Block::Parameter* parameter_position = new Parser::Block::Parameter("position");
      parameter_position->setUnique(true);
    Parser::Block::Parameter* parameter_name = new Parser::Block::Parameter("name");
      parameter_name->setUnique(true);
    Parser::Block::Parameter* parameter_size = new Parser::Block::Parameter("size");
      parameter_size->setUnique(true);

    Parser::Block* block_world = new Parser::Block("world");
      block_world->setUnique(true);
      block_world->registerParameter(parameter_name);
      block_world->registerParameter(parameter_size);
    p.registerBlock(block_world);

    Parser::Block* block_box = new Parser::Block("box");
      block_box->registerParameter(parameter_name);
      block_box->registerParameter(parameter_size);
      block_box->registerParameter(parameter_position);
    p.registerBlock(block_box);

    Parser::Block* block_pyramid = new Parser::Block(*block_box);
      block_pyramid->setIdentifier("pyramid");
    p.registerBlock(block_pyramid);

    Parser::Block* block_mesh = new Parser::Block("mesh");
      block_mesh->registerParameter(new Parser::Block::Parameter("vertex"));
    p.registerBlock(block_mesh);
  p.parseBZW(cin);

  return 0;
}
/*
 * Local Variables:
 * mode: C++
 * tab-width: 8
 * c-basic-offset: 2
 * indent-tabs-mode: t
 * End:
 * ex: shiftwidth=2 tabstop=8
 */
