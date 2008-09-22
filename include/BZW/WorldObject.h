/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __BZW_WORLDOBJECT_H__
#define __BZW_WORLDOBJECT_H__

#include <string>
#include <vector>
#include <list>

/* bzflag common headers */

namespace BZW
{
  /** 
   * Class for all World Objects.
   */

  std::string peekNextString ( const std::istringstream& line )
  {
    // do something here to look at the next string, but dont' read past it
  }

  class WorldObject
  {
    public:
      virtual ~WorldObject(){};

      /// Read and parse a parameter line in BZW format
      virtual void readLine(const std::istringstream& line)
      {
        std::string param = peekNextString(line);

        if (param == "name")
        {
          line >> param >> name;
        }
        else
        {
          std::cerr << "Warning! Unrecognized parameter: " << param << std::endl;
        }
      }

      virtual void finalize ( void ){}; // some objects may need to do stuff when we end

    protected:
      std::string name;

  };

  class PostionalWorldObject : public WorldObject
  {
    public:

      PostionalWorldObject()
      {
        position[0] = position[1] = position[2] = 0;
        size[0] = size[1] = size[2] = 1;
        rotation =0;
      }

      virtual void readLine(const std::istringstream& line)
      {
        std::string param = peekNextString(line);

        if(param == "position")
          line >> param >> position[0] >> position[1] >> position[2];
        else if(param == "size")
          line >> param >> size[0] >> size[1] >> size[2];
        else if(param == "rotation")
          line >> param >> rotation;
        else
          WorldObject::readLine(line);
      }

      float position[3];
      float size[3];
      float rotation;
  };


  class CustomObjectCallback
  {
    virtual ~CustomObjectCallback(){};
    virtual void process ( const std::string & name , const std::list<std::string> &data ) = 0;
  };

  class GenericWorldObject : public WorldObject
  {
    public:
      // this object just goes and stores all the data for an object
      // so that it's not lost, we basically ingore these in the world
      virtual void readLine(const std::istringstream& line)
      {
        char t[512] = {0};

        line.getline(t,512);
        items.push_back(std::string(t));
      }

      virtual void finalize ( void )
      {
        for ( size_t i=0; i < callbacks.size(); i++)
        {
          if (callbacks[i])
            callbacks[i]->process(name,items);
        }
      }

      std::list<CustomObjectCallback*> callbacks;
      std::list<std::string> items;
      std::string className;
  };
}

#endif // __BZW_WORLDOBJECT_H__

// Local Variables: ***
// Mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

