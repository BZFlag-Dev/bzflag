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

#include "common.h"

class A
{
public:
  virtual void foo() { printf("foo\n"); }
  virtual void bar() { printf("bar\n"); }
};

class B : public A
{
public:
  void foo() { printf("foo B\n"); }
  void bar() { printf("bar B\n"); }
};

void (A::*Method)();

int main(int argc, char* argv[])
{
    Method = A::bar;
    A a; B b;
    (a.*Method)();
    (b.*Method)();
    printf("%d", sizeof(Method));
    test_gcrypt();
    //test_ldap();
    //test_net();
    getch();

	return 0;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8