/* bzflag
 * Copyright (c) 1993-2020 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __SINGLETON_H__
#define __SINGLETON_H__

/* Singleton template class
 *
 * This template is based on an implementation suggested by Martin York in
 * https://stackoverflow.com/a/1008289/9220132
 *
 * This template class pattern provides a traditional Singleton pattern.
 * Allows you to designate a single-instance global class by inheriting
 * from the template class.
 *
 * Example:
 *
 *   class Whatever : public Singleton<Whatever> ...
 *
 * The class will need to provide an accessible default constructor
 *
 */
template < typename T >
class Singleton
{
public:

    /** returns a singleton
     */
    static T& instance()
    {
        static T private_instance;
        return private_instance;
    }

    Singleton(const Singleton &) = delete;
    Singleton& operator=(const Singleton&) = delete;

protected:
    Singleton() = default;
    ~Singleton() = default;

};

#endif /* __SINGLETON_H__ */


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
