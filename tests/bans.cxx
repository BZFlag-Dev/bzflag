/* bzflag
 * Copyright (c) 1993-2015 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "CppUTest/TestHarness.h"

#include "AccessControlList.h"

TEST_GROUP(Bans)
{

};

TEST(Bans, IPBanComparisons)
{
  in_addr ip1, ip2, ip3;
  ip1.s_addr = inet_addr("127.0.0.1");
  ip2.s_addr = inet_addr("127.0.0.1");
  ip3.s_addr = inet_addr("127.0.0.2");

  BanInfo a1(ip1, "nobody", 0, 32);
  BanInfo a2(ip2, "nobody", 0, 32);
  BanInfo a3(ip3, "nobody", 0, 32);
  BanInfo a4(ip2, "nobody", 0, 8);

  // 127.0.0.1/32 should equal 127.0.0.1/32
  CHECK_TEXT(a1 == a2, "Equality: Two equivilent bans were not equal.");
  CHECK_TEXT(!(a1 != a2), "Inequality: Two equivilent bans were not equal.");

  // 127.0.0.1/32 should not equal 127.0.0.2/32
  CHECK_TEXT(!(a1 == a3), "Equality: Two different bans were equal.");
  CHECK_TEXT(a1 != a3, "Inequality: Two different bans were equal.");

  // 127.0.0.1/32 should not equal 127.0.0.1/8
  CHECK_TEXT(!(a1 == a4), "Equality: Two different bans with the same IP but different CIDR were equal.");
  CHECK_TEXT(a1 != a4, "Inequality: Two different bans with the same IP but different CIDR were equal.");
}

TEST(Bans, IPBanContains)
{
  in_addr ip1, ip2, ip3, ip4, ip5;
  ip1.s_addr = inet_addr("127.0.0.1");
  ip2.s_addr = inet_addr("127.0.0.5");
  ip3.s_addr = inet_addr("127.0.5.5");
  ip4.s_addr = inet_addr("127.5.5.5");
  ip5.s_addr = inet_addr("128.1.2.3");

  BanInfo exact1(ip1, "nobody", 0, 32);
  BanInfo exact2(ip2, "nobody", 0, 32);

  BanInfo classC1(ip1, "nobody", 0, 24);
  BanInfo classB1(ip1, "nobody", 0, 16);
  BanInfo classA1(ip1, "nobody", 0, 8);

  BanInfo CIDR30(ip1, "nobody", 0, 30);
  BanInfo CIDR29(ip1, "nobody", 0, 30);

  // Exact matches should be contained
  CHECK_TEXT(exact1.contains(ip1), "First exact IP false negative.");
  CHECK_TEXT(exact2.contains(ip2), "Second exact IP false negative.");

  // But these should not (since we're using the opposite IPs)
  CHECK_TEXT(!exact1.contains(ip2), "First exact IP false positive.");
  CHECK_TEXT(!exact2.contains(ip1), "Second exact IP false positive.");

  // Check IPs within the subnet
  CHECK_TEXT(classC1.contains(ip2), "Class C positive match test failure.");
  CHECK_TEXT(classB1.contains(ip3), "Class B positive match test failure.");
  CHECK_TEXT(classA1.contains(ip4), "Class A positive match test failure.");

  // Check IPs outside of the subnet
  CHECK_TEXT(!classC1.contains(ip3), "Class C negative match test failure.");
  CHECK_TEXT(!classC1.contains(ip4), "Class B negative match test failure.");
  CHECK_TEXT(!classC1.contains(ip5), "Class A negative match test failure.");
}

TEST(Bans, BanMaskString)
{
  in_addr ip1;
  ip1.s_addr = inet_addr("127.5.35.135");

  AccessControlList acl;

  std::string exact = acl.getBanMaskString(ip1, 32);
  std::string cidr24 = acl.getBanMaskString(ip1, 24);
  std::string cidr16 = acl.getBanMaskString(ip1, 16);
  std::string cidr8 = acl.getBanMaskString(ip1, 8);
  std::string cidr25 = acl.getBanMaskString(ip1, 25);
  std::string cidr19 = acl.getBanMaskString(ip1, 19);
  
  CHECK_TEXT(exact == "127.5.35.135", std::string("Exact IP mask generation failed: " + exact + " != 127.5.7.135").c_str());
  CHECK_TEXT(cidr24 == "127.5.35.*", std::string("CIDR /24 mask generation failed: " + cidr24 + " != 127.5.7.*").c_str());
  CHECK_TEXT(cidr16 == "127.5.*.*", std::string("CIDR /16 mask generation failed: " + cidr16 + " != 127.5.*.*").c_str());
  CHECK_TEXT(cidr8 == "127.*.*.*", std::string("CIDR /8 mask generation failed: " + cidr8 + " != 127.*.*.*").c_str());
  CHECK_TEXT(cidr25 == "127.5.35.128/25", std::string("CIDR /25 mask generation failed: " + cidr25 + " != 127.5.7.128/25").c_str());
  CHECK_TEXT(cidr19 == "127.5.32.0/19", std::string("CIDR /19 mask generation failed: " + cidr19 + " != 127.5.32.0/19").c_str());
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
