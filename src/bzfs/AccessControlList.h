/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __ACCESSCONTROLLIST_H__
#define __ACCESSCONTROLLIST_H__

#include <vector>
#include <string>
#include <string.h>

#include "TimeKeeper.h"
#include "Address.h"

/** This struct contains information about a ban - the address that was banned,
    the time the ban will expire, the callsign of the player who performed the
    ban, and the reason. It also has operators defined for checking if two
    bans are equal. */
struct BanInfo
{
  /** This constructor creates a new BanInfo with the address @c banAddr,
      the ban performer @c bannedBy, and the expiration time @c period
      minutes from now. */
  BanInfo(in_addr &banAddr, const char *_bannedBy = NULL, int period = 0,
	  bool isFromMaster = false ) {
    memcpy( &addr, &banAddr, sizeof( in_addr ));
    if (_bannedBy)
      bannedBy = _bannedBy;
    if (period == 0) {
      banEnd = TimeKeeper::getSunExplodeTime();
    } else {
      banEnd = TimeKeeper::getCurrent();
      banEnd += period * 60.0f;
    }
    fromMaster = isFromMaster;
  }
  /** BanInfos with same IP are identical. */
  bool operator==(const BanInfo &rhs) const {
    return addr.s_addr == rhs.addr.s_addr;
  }
  /** Only BanInfos with the same IP are identical. */
  bool operator!=(const BanInfo &rhs) const {
    return addr.s_addr != rhs.addr.s_addr;
  }

  in_addr	addr;
  TimeKeeper	banEnd;
  std::string	bannedBy;	// Who did perform the ban
  std::string	reason;		// reason for banning
  bool fromMaster;		// where the ban came from, local or master list.
};


/** This struct contains information about a hostban - the host pattern that
    was banned, the time the ban will expire, the callsign of the player who
    performed the ban, and the reason. It also has operators defined for
    checking if two bans are equal. */
struct HostBanInfo
{
  /** This constructor creates a new HostBanInfo with the host pattern
      @c hostpat, the ban performer @c bannedBy, and the expiration time
      @c period minutes from now. */
  HostBanInfo(std::string _hostpat, const char *_bannedBy = NULL,
	      int period = 0, bool isFromMaster = false ) {
    hostpat = _hostpat;
    if (_bannedBy)
      bannedBy = _bannedBy;
    if (period == 0) {
      banEnd = TimeKeeper::getSunExplodeTime();
    } else {
      banEnd = TimeKeeper::getCurrent();
      banEnd += period * 60.0f;
    }
    fromMaster = isFromMaster;
  }
  /** HostBanInfos with same host pattern are identical. */
  bool operator==(const HostBanInfo &rhs) const {
    return hostpat == rhs.hostpat;
  }
  /** Only HostBanInfos with same host pattern are identical. */
  bool operator!=(const HostBanInfo& rhs) const {
    return hostpat != rhs.hostpat;
  }

  std::string hostpat;
  TimeKeeper banEnd;
  std::string bannedBy;
  std::string reason;
  bool fromMaster;	// where the ban came from, local or master list.
};


/** This struct contains information about an idban - the databaseid pattern
    that was banned, the time the ban will expire, the callsign of the player
    who performed the ban, and the reason. It also has operators defined for
    checking if two bans are equal. */
struct IdBanInfo
{
  /** This constructor creates a new IdBanInfo with the id pattern
      @c idpat, the ban performer @c bannedBy, and the expiration time
      @c period minutes from now. */
  IdBanInfo(std::string _idpat, const char *_bannedBy = NULL,
	    int period = 0, bool isFromMaster = false ) {
    idpat = _idpat;
    if (_bannedBy)
      bannedBy = _bannedBy;
    if (period == 0) {
      banEnd = TimeKeeper::getSunExplodeTime();
    } else {
      banEnd = TimeKeeper::getCurrent();
      banEnd += period * 60.0f;
    }
    fromMaster = isFromMaster;
  }
  /** IdBanInfos with same id pattern are identical. */
  bool operator==(const IdBanInfo &rhs) const {
    return idpat == rhs.idpat;
  }
  /** Only IdBanInfos with same id pattern are identical. */
  bool operator!=(const IdBanInfo& rhs) const {
    return idpat != rhs.idpat;
  }

  std::string idpat;
  TimeKeeper banEnd;
  std::string bannedBy;
  std::string reason;
  bool fromMaster;	// where the ban came from, local or master list.
};


/* FIXME the AccessControlList assumes that 255 is a wildcard. it "should"
 * include a cidr mask with each address. it's still useful as is, though
 * see wildcard conversion occurs in convert().
 */

/** This class handles the lists of bans and hostbans. It has functions for
    adding and removing bans and hostbans, checking if a certain IP or host
    is banned, sending the ban lists to a player, and reading and writing the
    ban lists to a file. */
class AccessControlList
{
public:

  /** This function will add a ban for the address @c ipAddr with the given
      parameters. If that address already is banned the old ban will be
      replaced. */
  void ban(in_addr &ipAddr, const char *bannedBy, int period = 0,
	   const char *reason=NULL, bool fromMaster = false);

  /** This function takes a list of addresses as a string and tries to ban them
      using the given parameters. The string should be comma separated,
      like this: "1.2.3.4,5.6.7.8,9.10.11.12". */
  bool ban(std::string &ipList, const char *bannedBy=NULL, int period = 0,
	   const char *reason=NULL, bool fromMaster = false);

  /** This function takes a list of addresses as a <code>const char*</code>
      and tries to ban them using the given parameters. The string should be
      comma separated, like this: "1.2.3.4,5.6.7.8,9.10.11.12". */
  bool ban(const char *ipList, const char *bannedBy=NULL, int period = 0,
	   const char *reason=NULL, bool fromMaster = false);

  /** This function adds a hostban for the host pattern @c hostpat with the
      given parameters. If the host pattern already is banned the old ban will
      be replaced. */
  void hostBan(std::string hostpat, const char *bannedBy, int period = 0,
	       const char *reason = NULL, bool fromMaster = false);

  /** This function adds an idban for the id pattern @c idpat with the
      given parameters. If the idban pattern already is banned the old ban will
      be replaced. */
  void idBan(std::string hostpat, const char *bannedBy, int period = 0,
	     const char *reason = NULL, bool fromMaster = false);

  /** This function removes any ban for the address @c ipAddr.
      @returns @c true if there was a ban for that address, @c false if there
      wasn't. */
  bool unban(in_addr &ipAddr);

  /** This function unbans any addresses given in @c ipList, which should be
      a comma separated string in the same format as in the ban() functions.
      @returns @c true if there was a ban for any of the addresses, @c false
      if none of the addresses were banned. */
  bool unban(std::string &ipList);

  /** This function unbans any addresses given in @c ipList, which should be
      a comma separated string in the same format as in the ban() functions.
      @returns @c true if there was a ban for any of the addresses, @c false
      if none of the addresses were banned. */
  bool unban(const char *ipList);

  /** This function removes any ban for the host pattern @c hostpat.
      @returns @c true if there was a ban for the host pattern, @c false
      otherwise. */
  bool hostUnban(std::string hostpat);

  /** This function removes any ban for the database id @c idpat.
      @returns @c true if there was a ban for the database id, @c false
      otherwise. */
  bool idUnban(std::string idpat);

  /** This function checks if an address is "valid" or not. Valid in this case
      means that it has not been banned.
      @returns @c true if the address is valid, @c false if not. */
  bool validate(const in_addr &ipAddr, BanInfo *info = NULL);

  /** This function checks if a host matches a given host pattern.
      @returns @c true if a match is found, @c false otherwise. */
  static bool does_match(const char *targ, int targlen, const char *pat, int patlen);

  /** This function checks that a hostname is "valid". In this case valid means
      "not banned".
      @returns @c true if the hostname is valid, @c false if it isn't. */
  bool hostValidate(const char *hostname, HostBanInfo *info = NULL);

  /** This function checks that an id is "valid". In this case valid means
      "not banned".
      @returns @c true if the id is valid, @c false if it isn't. */
  bool idValidate(const char *idname, IdBanInfo *info = NULL);

  /** This function sends a textual list of the given IP ban to a player. */
  void sendBan(PlayerId id, const BanInfo&);

  /** This function sends a textual list of all IP bans to a player. */
  void sendBans(PlayerId id, const char* pattern);

  /** This function sends a textual list of all host bans to a player. */
  void sendHostBans(PlayerId id, const char* pattern);

  /** This function sends a textual list of all id bans to a player. */
  void sendIdBans(PlayerId id, const char* pattern);

  /** This function tells this object where to save the banlist, and where
      to load it from. */
  void setBanFile(const std::string& filename);

  /** This function loads a banlist from the ban file, if it has been set.
      It only returns @c false if the file exist but is not in the correct
      format, otherwise @c true is returned. */
  bool load();

  /** This function saves the banlist to the ban file, if it has been set. */
  void save();

  /** This function merges in a banlist from the master ban list ban file,
      if it has been set. It only returns @c false if the file exist but is not
      in the correct format, otherwise @c true is returned. */
  int  merge(const std::string& banData);

  /** This function purges any bans that are flagged as from the master ban list,
      presumably so it can be remerged */
  void purgeMasters(void);

  std::vector<std::pair<std::string, std::string> > listMasterBans(void) const;

private:
  /** This function converts a <code>char*</code> containing an IP mask to an
      @c in_addr. */
  bool convert(char *ip, in_addr &mask);

  /** This function checks all bans to see if any of them have expired,
      and removes those who have. */
  void expire();

  /** This function removes bans dependent on their origin
      (local or from master list) **/
  void purge(bool master);

  /** This function purges all local bans
      so the local banfile can be reloaded **/
  void purgeLocals(void);

  typedef std::vector<BanInfo> banList_t;
  banList_t banList;

  typedef std::vector<HostBanInfo> hostBanList_t;
  hostBanList_t hostBanList;

  typedef std::vector<IdBanInfo> idBanList_t;
  idBanList_t idBanList;

  std::string banFile;
};

inline void AccessControlList::setBanFile(const std::string& filename) {
  banFile = filename;
}


#endif /* __ACCESSCONTROLLIST_H__ */

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

