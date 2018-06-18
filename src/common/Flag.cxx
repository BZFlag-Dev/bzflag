/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
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

/* interface header */
#include "Flag.h"

/* system implementation headers */
#include <math.h>
#include <string>
#include <assert.h>
#include <string.h>

/* common implementation headers */
#include "Team.h"
#include "Pack.h"
#include "TextUtils.h"


std::vector<FlagType::Set> FlagType::FlagType::Sets;
const int FlagType::packSize = FlagPackSize;
FlagType::Set FlagType::customFlags;

// Initialize flag description singletons in our Flags namespace
namespace Flags
{
    FlagType::Ptr Null;
    FlagType::Ptr RedTeam;
    FlagType::Ptr GreenTeam;
    FlagType::Ptr BlueTeam;
    FlagType::Ptr PurpleTeam;
    FlagType::Ptr Velocity;
    FlagType::Ptr QuickTurn;
    FlagType::Ptr OscillationOverthruster;
    FlagType::Ptr RapidFire;
    FlagType::Ptr MachineGun;
    FlagType::Ptr GuidedMissile;
    FlagType::Ptr Laser;
    FlagType::Ptr Ricochet;
    FlagType::Ptr SuperBullet;
    FlagType::Ptr InvisibleBullet;
    FlagType::Ptr Stealth;
    FlagType::Ptr Tiny;
    FlagType::Ptr Narrow;
    FlagType::Ptr Shield;
    FlagType::Ptr Steamroller;
    FlagType::Ptr ShockWave;
    FlagType::Ptr PhantomZone;
    FlagType::Ptr Jumping;
    FlagType::Ptr Identify;
    FlagType::Ptr Cloaking;
    FlagType::Ptr Useless;
    FlagType::Ptr Masquerade;
    FlagType::Ptr Seer;
    FlagType::Ptr Thief;
    FlagType::Ptr Burrow;
    FlagType::Ptr Wings;
    FlagType::Ptr Agility;
    FlagType::Ptr Colorblindness;
    FlagType::Ptr Obesity;
    FlagType::Ptr LeftTurnOnly;
    FlagType::Ptr RightTurnOnly;
    FlagType::Ptr ForwardOnly;
    FlagType::Ptr ReverseOnly;
    FlagType::Ptr Momentum;
    FlagType::Ptr Blindness;
    FlagType::Ptr Jamming;
    FlagType::Ptr WideAngle;
    FlagType::Ptr NoJumping;
    FlagType::Ptr TriggerHappy;
    FlagType::Ptr ReverseControls;
    FlagType::Ptr Bouncy;
    FlagType::Ptr Unknown;

    FlagType::Ptr AddStdFlag(FlagType::Ptr ptr)
    {
        if (FlagType::FlagType::Sets.size() == 0)
            FlagType::FlagType::Sets = std::vector<FlagType::Set>((int)FlagQuality::Last);

        if (ptr->custom)
            FlagType::customFlags.insert(ptr);

        FlagType::FlagType::Sets[(int)ptr->flagQuality].insert(ptr);
        FlagType::getFlagMap()[ptr->flagAbbv] = ptr;

        return ptr;
    }

    FlagType::Ptr AddCustomFlag(FlagType::Ptr ptr)
    {
        return AddStdFlag(ptr);
    }

    void init()
    {
        Null    = AddStdFlag(std::make_shared<FlagType>( "", "", FlagEndurance::Normal, ShotType::Normal, FlagQuality::Good, NoTeam, FlagEffect::Normal, "" ));

        Unknown = AddStdFlag(std::make_shared<FlagType>("Unknown", "--", FlagEndurance::Normal, ShotType::Normal, FlagQuality::Good, NoTeam, FlagEffect::Normal,
                                    "A flag of an unknown type, pick it up and see what it is."));
        RedTeam = AddStdFlag(std::make_shared<FlagType>( "Red Team", "R*", FlagEndurance::Normal, ShotType::Normal, FlagQuality::Good, ::RedTeam, FlagEffect::Normal,
                                    "If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!" ));
        GreenTeam   = AddStdFlag(std::make_shared<FlagType>( "Green Team", "G*", FlagEndurance::Normal, ShotType::Normal, FlagQuality::Good, ::GreenTeam, FlagEffect::Normal,
                                    "If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!" ));
        BlueTeam    = AddStdFlag(std::make_shared<FlagType>( "Blue Team", "B*", FlagEndurance::Normal, ShotType::Normal, FlagQuality::Good, ::BlueTeam, FlagEffect::Normal,
                                    "If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!" ));
        PurpleTeam  = AddStdFlag(std::make_shared<FlagType>( "Purple Team", "P*", FlagEndurance::Normal, ShotType::Normal, FlagQuality::Good, ::PurpleTeam, FlagEffect::Normal,
                                    "If it's yours, prevent other teams from taking it.  If it's not take it to your base to capture it!" ));
        Velocity    = AddStdFlag(std::make_shared<FlagType>( "High Speed", "V", FlagEndurance::Unstable, ShotType::Normal, FlagQuality::Good, NoTeam, FlagEffect::Velocity,
                                    "Tank moves faster.  Outrun bad guys." ));
        QuickTurn   = AddStdFlag(std::make_shared<FlagType>( "Quick Turn", "QT", FlagEndurance::Unstable, ShotType::Normal, FlagQuality::Good, NoTeam, FlagEffect::QuickTurn,
                                    "Tank turns faster.  Good for dodging." ));
        OscillationOverthruster = AddStdFlag(std::make_shared<FlagType>( "Oscillation Overthruster", "OO", FlagEndurance::Unstable, ShotType::Normal, FlagQuality::Good, NoTeam, FlagEffect::OscillationOverthruster,
                                    "Can drive through buildings.  Can't back up or shoot while inside." ));
        RapidFire   = AddStdFlag(std::make_shared<FlagType>( "Rapid Fire", "F", FlagEndurance::Unstable, ShotType::Special, FlagQuality::Good, NoTeam, FlagEffect::RapidFire,
                                    "Shoots more often.  Shells go faster but not as far." ));
        MachineGun  = AddStdFlag(std::make_shared<FlagType>( "Machine Gun", "MG", FlagEndurance::Unstable, ShotType::Special, FlagQuality::Good, NoTeam, FlagEffect::MachineGun,
                                    "Very fast reload and very short range." ));
        GuidedMissile   = AddStdFlag(std::make_shared<FlagType>( "Guided Missile", "GM", FlagEndurance::Unstable, ShotType::Special, FlagQuality::Good, NoTeam, FlagEffect::GuidedMissile,
                                    "Shots track a target.  Lock on with right button.  Can lock on or retarget after firing." ));
        Laser   = AddStdFlag(std::make_shared<FlagType>( "Laser", "L", FlagEndurance::Unstable, ShotType::Special, FlagQuality::Good, NoTeam, FlagEffect::Laser,
                                    "Shoots a laser.  Infinite speed and range but long reload time."));
        Ricochet    = AddStdFlag(std::make_shared<FlagType>( "Ricochet", "R", FlagEndurance::Unstable, ShotType::Special, FlagQuality::Good, NoTeam, FlagEffect::Ricochet,
                                    "Shots bounce off walls.  Don't shoot yourself!" ));
        SuperBullet = AddStdFlag(std::make_shared<FlagType>( "Super Bullet", "SB", FlagEndurance::Unstable, ShotType::Special, FlagQuality::Good, NoTeam, FlagEffect::SuperBullet,
                                    "Shoots through buildings.  Can kill Phantom Zone." ));
        InvisibleBullet = AddStdFlag(std::make_shared<FlagType>( "Invisible Bullet", "IB", FlagEndurance::Unstable, ShotType::Normal, FlagQuality::Good, NoTeam, FlagEffect::InvisibleBullet,
                                    "Your shots don't appear on other radars.  Can still see them out window."));
        Stealth = AddStdFlag(std::make_shared<FlagType>( "Stealth", "ST", FlagEndurance::Unstable, ShotType::Normal, FlagQuality::Good, NoTeam, FlagEffect::Stealth,
                                    "Tank is invisible on radar.  Shots are still visible.  Sneak up behind enemies!"));
        Tiny    = AddStdFlag(std::make_shared<FlagType>( "Tiny", "T", FlagEndurance::Unstable, ShotType::Normal, FlagQuality::Good, NoTeam, FlagEffect::Tiny,
                                    "Tank is small and can get through small openings.  Very hard to hit." ));
        Narrow  = AddStdFlag(std::make_shared<FlagType>( "Narrow", "N", FlagEndurance::Unstable, ShotType::Normal, FlagQuality::Good, NoTeam, FlagEffect::Narrow,
                                    "Tank is super thin.  Very hard to hit from front but is normal size from side.  Can get through small openings."));
        Shield  = AddStdFlag(std::make_shared<FlagType>( "Shield", "SH", FlagEndurance::Unstable, ShotType::Normal, FlagQuality::Good, NoTeam, FlagEffect::Shield,
                                    "Getting hit only drops flag.  Flag flies an extra-long time."));
        Steamroller = AddStdFlag(std::make_shared<FlagType>( "Steamroller", "SR", FlagEndurance::Unstable, ShotType::Normal, FlagQuality::Good, NoTeam, FlagEffect::Steamroller,
                                    "Destroys tanks you touch but you have to get really close."));
        ShockWave   = AddStdFlag(std::make_shared<FlagType>( "Shock Wave", "SW", FlagEndurance::Unstable, ShotType::Special, FlagQuality::Good, NoTeam, FlagEffect::ShockWave,
                                    "Firing destroys all tanks nearby.  Don't kill teammates!  Can kill tanks on/in buildings."));
        PhantomZone = AddStdFlag(std::make_shared<FlagType>( "Phantom Zone", "PZ", FlagEndurance::Unstable, ShotType::Special, FlagQuality::Good, NoTeam, FlagEffect::PhantomZone,
                                    "Teleporting toggles Zoned effect.  Zoned tank can drive through buildings.  Zoned tank shoots Zoned bullets and can't be shot (except by superbullet, shock wave, and other Zoned tanks)."));
        Jumping = AddStdFlag(std::make_shared<FlagType>( "Jumping", "JP", FlagEndurance::Unstable, ShotType::Normal, FlagQuality::Good, NoTeam, FlagEffect::Jumping,
                                    "Tank can jump.  Use Tab key.  Can't steer in the air."));
        Identify    = AddStdFlag(std::make_shared<FlagType>( "Identify", "ID", FlagEndurance::Unstable, ShotType::Normal, FlagQuality::Good, NoTeam, FlagEffect::Identify,
                                    "Identifies type of nearest flag."));
        Cloaking    = AddStdFlag(std::make_shared<FlagType>( "Cloaking", "CL", FlagEndurance::Unstable, ShotType::Normal, FlagQuality::Good, NoTeam, FlagEffect::Cloaking,
                                    "Makes your tank invisible out-the-window.  Still visible on radar."));
        Useless = AddStdFlag(std::make_shared<FlagType>( "Useless", "US", FlagEndurance::Unstable, ShotType::Normal, FlagQuality::Good, NoTeam, FlagEffect::Useless,
                                    "You have found the useless flag. Use it wisely."));
        Masquerade  = AddStdFlag(std::make_shared<FlagType>( "Masquerade", "MQ", FlagEndurance::Unstable, ShotType::Normal, FlagQuality::Good, NoTeam, FlagEffect::Masquerade,
                                    "In opponent's hud, you appear as a teammate."));
        Seer    = AddStdFlag(std::make_shared<FlagType>( "Seer", "SE", FlagEndurance::Unstable, ShotType::Normal, FlagQuality::Good, NoTeam, FlagEffect::Seer,
                                    "See stealthed, cloaked and masquerading tanks as normal."));
        Thief   = AddStdFlag(std::make_shared<FlagType>( "Thief", "TH", FlagEndurance::Unstable, ShotType::Special, FlagQuality::Good, NoTeam, FlagEffect::Thief,
                                    "Steal flags.  Small and fast but can't kill."));
        Burrow  = AddStdFlag(std::make_shared<FlagType>( "Burrow", "BU", FlagEndurance::Unstable, ShotType::Normal, FlagQuality::Good, NoTeam, FlagEffect::Burrow,
                                    "Tank burrows underground, impervious to normal shots, but can be steamrolled by anyone!"));
        Wings   = AddStdFlag(std::make_shared<FlagType>( "Wings", "WG", FlagEndurance::Unstable, ShotType::Normal, FlagQuality::Good, NoTeam, FlagEffect::Wings,
                                    "Tank can drive in air."));
        Agility = AddStdFlag(std::make_shared<FlagType>( "Agility", "A", FlagEndurance::Unstable, ShotType::Normal, FlagQuality::Good, NoTeam, FlagEffect::Agility,
                                    "Tank is quick and nimble making it easier to dodge."));
        ReverseControls = AddStdFlag(std::make_shared<FlagType>( "ReverseControls", "RC", FlagEndurance::Sticky, ShotType::Normal, FlagQuality::Bad, NoTeam, FlagEffect::ReverseControls,
                                    "Tank driving controls are reversed."));
        Colorblindness  = AddStdFlag(std::make_shared<FlagType>( "Colorblindness", "CB", FlagEndurance::Sticky, ShotType::Normal, FlagQuality::Bad, NoTeam, FlagEffect::Colorblindness,
                                    "Can't tell team colors.  Don't shoot teammates!"));
        Obesity = AddStdFlag(std::make_shared<FlagType>( "Obesity", "O", FlagEndurance::Sticky, ShotType::Normal, FlagQuality::Bad, NoTeam, FlagEffect::Obesity,
                                    "Tank becomes very large.  Can't fit through teleporters."));
        LeftTurnOnly    = AddStdFlag(std::make_shared<FlagType>( "Left Turn Only", "LT", FlagEndurance::Sticky, ShotType::Normal, FlagQuality::Bad, NoTeam, FlagEffect::LeftTurnOnly,
                                    "Can't turn right."));
        RightTurnOnly   = AddStdFlag(std::make_shared<FlagType>( "Right Turn Only", "RT", FlagEndurance::Sticky, ShotType::Normal, FlagQuality::Bad, NoTeam, FlagEffect::RightTurnOnly,
                                    "Can't turn left."));
        ForwardOnly = AddStdFlag(std::make_shared<FlagType>( "Forward Only", "FO", FlagEndurance::Sticky, ShotType::Normal, FlagQuality::Bad, NoTeam, FlagEffect::ForwardOnly,
                                    "Can't drive in reverse."));
        ReverseOnly = AddStdFlag(std::make_shared<FlagType>( "ReverseOnly", "RO", FlagEndurance::Sticky, ShotType::Normal, FlagQuality::Bad, NoTeam, FlagEffect::ReverseOnly,
                                    "Can't drive forward."));
        Momentum    = AddStdFlag(std::make_shared<FlagType>( "Momentum", "M", FlagEndurance::Sticky, ShotType::Normal, FlagQuality::Bad, NoTeam, FlagEffect::Momentum,
                                    "Tank has inertia.  Acceleration is limited."));
        Blindness   = AddStdFlag(std::make_shared<FlagType>( "Blindness", "B", FlagEndurance::Sticky, ShotType::Normal, FlagQuality::Bad, NoTeam, FlagEffect::Blindness,
                                    "Can't see out window.  Radar still works."));
        Jamming = AddStdFlag(std::make_shared<FlagType>( "Jamming", "JM", FlagEndurance::Sticky, ShotType::Normal, FlagQuality::Bad, NoTeam, FlagEffect::Jamming,
                                    "Radar doesn't work.  Can still see."));
        WideAngle   = AddStdFlag(std::make_shared<FlagType>( "Wide Angle", "WA", FlagEndurance::Sticky, ShotType::Normal, FlagQuality::Bad, NoTeam, FlagEffect::WideAngle,
                                    "Fish-eye lens distorts view."));
        NoJumping   = AddStdFlag(std::make_shared<FlagType>( "No Jumping", "NJ", FlagEndurance::Sticky, ShotType::Normal, FlagQuality::Bad, NoTeam, FlagEffect::NoJumping,
                                    "Tank can't jump."));
        TriggerHappy    = AddStdFlag(std::make_shared<FlagType>( "Trigger Happy", "TR", FlagEndurance::Sticky, ShotType::Normal, FlagQuality::Bad, NoTeam, FlagEffect::TriggerHappy,
                                    "Tank can't stop firing."));
        Bouncy  = AddStdFlag(std::make_shared<FlagType>( "Bouncy", "BY", FlagEndurance::Sticky, ShotType::Normal, FlagQuality::Bad, NoTeam, FlagEffect::Bouncy,
                                    "Tank can't stop bouncing."));
    }

    void kill()
    {
        clearCustomFlags();

        RedTeam = nullptr;
        GreenTeam = nullptr;
        BlueTeam = nullptr;
        PurpleTeam = nullptr;
        Velocity = nullptr;
        QuickTurn = nullptr;
        OscillationOverthruster = nullptr;
        RapidFire = nullptr;
        MachineGun = nullptr;
        GuidedMissile = nullptr;
        Laser = nullptr;
        Ricochet = nullptr;
        SuperBullet = nullptr;
        InvisibleBullet = nullptr;
        Stealth = nullptr;
        Tiny = nullptr;
        Narrow = nullptr;
        Shield = nullptr;
        Steamroller = nullptr;
        ShockWave = nullptr;
        PhantomZone = nullptr;
        Jumping = nullptr;
        Identify = nullptr;
        Cloaking = nullptr;
        Useless = nullptr;
        Masquerade = nullptr;
        Seer = nullptr;
        Thief = nullptr;
        Burrow = nullptr;
        Wings = nullptr;
        Agility = nullptr;
        ReverseControls = nullptr;
        Colorblindness = nullptr;
        Obesity = nullptr;
        LeftTurnOnly = nullptr;
        RightTurnOnly = nullptr;
        ForwardOnly = nullptr;
        ReverseOnly = nullptr;
        Momentum = nullptr;
        Blindness = nullptr;
        Jamming = nullptr;
        WideAngle = nullptr;
        NoJumping = nullptr;
        TriggerHappy = nullptr;
        Bouncy = nullptr;

        FlagType::FlagType::Sets.clear();
        FlagType::customFlags.clear();
    }

    void clearCustomFlags()
    {
        FlagType::Set::iterator itr, nitr;
        for (int q = 0; q < (int)FlagQuality::Last; ++q)
        {
            for (itr = FlagType::FlagType::Sets[q].begin(); itr != FlagType::FlagType::Sets[q].end(); ++itr)
            {
                if ((*itr)->custom)
                {
                    FlagType::getFlagMap().erase((*itr)->flagAbbv);
                    nitr = itr;
                    ++nitr;
                    FlagType::FlagType::Sets[q].erase(itr);
                    itr = nitr;
                    if (itr == FlagType::FlagType::Sets[q].end()) break;
                }
            }
        }
        FlagType::customFlags.clear();
    }
}

void* FlagType::pack(void* buf) const
{
    buf = nboPackUByte(buf, (flagAbbv.size() > 0) ? flagAbbv[0] : 0);
    buf = nboPackUByte(buf, (flagAbbv.size() > 1) ? flagAbbv[1] : 0);
    return buf;
}

void* FlagType::fakePack(void* buf) const
{
    buf = nboPackUByte(buf, '-');
    buf = nboPackUByte(buf, '-');
    return buf;
}

const void* FlagType::unpack(const void* buf, FlagType::Ptr &type)
{
    unsigned char abbv[3] = {0, 0, 0};
    buf = nboUnpackUByte(buf, abbv[0]);
    buf = nboUnpackUByte(buf, abbv[1]);
    type = FlagType::getDescFromAbbreviation((const char *)abbv);
    return buf;
}

void* FlagType::packCustom(void* buf) const
{
    buf = pack(buf);
    buf = nboPackUByte(buf, uint8_t(flagQuality));
    buf = nboPackUByte(buf, uint8_t(flagShot));
    buf = nboPackUByte(buf, uint8_t(flagEffect));
    buf = nboPackStdString(buf, flagName);
    buf = nboPackStdString(buf, flagHelp);
    return buf;
}

const void* FlagType::unpackCustom(const void* buf, FlagType::Ptr &type)
{
    uint8_t *abbv = new uint8_t[3];
    abbv[0]=abbv[1]=abbv[2]=0;
    buf = nboUnpackUByte(buf, abbv[0]);
    buf = nboUnpackUByte(buf, abbv[1]);

    uint8_t quality, shot, effect;
    buf = nboUnpackUByte(buf, quality);
    buf = nboUnpackUByte(buf, shot);
    buf = nboUnpackUByte(buf, effect);
    // make copies to keep - note that these will need to be deleted.
    std::string sName, sHelp;
    buf = nboUnpackStdString(buf, sName);
    buf = nboUnpackStdString(buf, sHelp);

    FlagEndurance e = FlagEndurance::Unstable;
    switch ((FlagQuality)quality)
    {
    case FlagQuality::Good:
        e = FlagEndurance::Unstable;
        break;
    case FlagQuality::Bad:
        e = FlagEndurance::Sticky;
        break;
    default:
        assert(false); // shouldn't happen
    }

    type = Flags::AddCustomFlag(std::make_shared<FlagType>(sName, reinterpret_cast<const char*>(&abbv[0]), e, (ShotType)shot, (FlagQuality)quality, NoTeam, (FlagEffect)effect, sHelp, true));
    return buf;
}

static FlagType::TypeMap flagMap;
FlagType::TypeMap& FlagType::getFlagMap()
{
    return flagMap;
}

void* FlagInstance::pack(void* buf) const
{
    buf = type->pack(buf);
    buf = nboPackUShort(buf, uint16_t(status));
    buf = nboPackUShort(buf, uint16_t(endurance));
    buf = nboPackUByte(buf, owner);
    buf = nboPackVector(buf, position);
    buf = nboPackVector(buf, launchPosition);
    buf = nboPackVector(buf, landingPosition);
    buf = nboPackFloat(buf, flightTime);
    buf = nboPackFloat(buf, flightEnd);
    buf = nboPackFloat(buf, initialVelocity);
    return buf;
}

void* FlagInstance::fakePack(void* buf) const
{
    buf = type->fakePack(buf);
    buf = nboPackUShort(buf, uint16_t(status));
    buf = nboPackUShort(buf, uint16_t(endurance));
    buf = nboPackUByte(buf, owner);
    buf = nboPackVector(buf, position);
    buf = nboPackVector(buf, launchPosition);
    buf = nboPackVector(buf, landingPosition);
    buf = nboPackFloat(buf, flightTime);
    buf = nboPackFloat(buf, flightEnd);
    buf = nboPackFloat(buf, initialVelocity);
    return buf;
}

const void* FlagInstance::unpack(const void* buf)
{
    uint16_t data;

    buf = FlagType::unpack(buf, type);
    buf = nboUnpackUShort(buf, data);
    status = FlagStatus(data);
    buf = nboUnpackUShort(buf, data);
    endurance = FlagEndurance(data);
    buf = nboUnpackUByte(buf, owner);
    buf = nboUnpackVector(buf, position);
    buf = nboUnpackVector(buf, launchPosition);
    buf = nboUnpackVector(buf, landingPosition);
    buf = nboUnpackFloat(buf, flightTime);
    buf = nboUnpackFloat(buf, flightEnd);
    buf = nboUnpackFloat(buf, initialVelocity);
    return buf;
}

FlagType::Ptr FlagType::getDescFromAbbreviation(const char* abbreviation)
{
    FlagType::TypeMap::iterator i;
    std::string abbvString;

    /* Uppercase the abbreviation */
    while (*abbreviation)
    {
        abbvString += toupper(*abbreviation);
        abbreviation++;
    }

    i = FlagType::getFlagMap().find(abbvString);
    if (i == FlagType::getFlagMap().end())
        /* Not found, return the Null flag */
        return Flags::Null;
    else
        return i->second;
}

FlagType::Set& FlagType::getGoodFlags()
{
    return FlagType::FlagType::Sets[(int)FlagQuality::Good];
}

FlagType::Set& FlagType::getBadFlags()
{
    return FlagType::FlagType::Sets[(int)FlagQuality::Bad];
}

const float* FlagType::getColor() const
{
    static const float superColor[3] = { 1.0, 1.0, 1.0 };

    if (flagTeam == NoTeam)
        return superColor;
    else
        return Team::getTankColor(flagTeam);
}

const float* FlagType::getRadarColor() const
{
    static const float superColor[3] = { 1.0, 1.0, 1.0 };

    if (flagTeam == NoTeam)
        return superColor;
    else
        return Team::getRadarColor(flagTeam);
}


const std::string FlagType::label() const
{
    unsigned int i;

    /* convert to lowercase so we can uppercase the abbreviation later */
    std::string caseName = "";
    for (i = 0; i < flagName.size(); i++)
        caseName += tolower(flagName.c_str()[i]);

    /* modify the flag name to exemplify the abbreviation */
    int charPosition;
    for (i = 0; i < flagAbbv.size(); i++)
    {

        charPosition = caseName.find_first_of(tolower(flagAbbv[i]), 0);

        if (charPosition > 0)
        {
            /* if we can match an abbreviation on a word boundary -- prefer it */
            int alternateCharPosition = 1;
            while (alternateCharPosition > 0)
            {
                if (caseName[alternateCharPosition - 1] == ' ')
                {
                    charPosition = alternateCharPosition;
                    break;
                }
                alternateCharPosition = caseName.find_first_of(tolower(flagAbbv[i]), alternateCharPosition+1);
            }
        }

        if (charPosition >= 0)
            caseName[charPosition] = toupper(caseName[charPosition]);
    }

    if (flagTeam != NoTeam)
    {
        /* team flag info is more simple than non-team flag info */
        caseName += " flag";
    }
    else
    {
        /* non-team flags */
        caseName += TextUtils::format(" (%c%s)",
                                      flagQuality==FlagQuality::Good?'+':'-',
                                      flagAbbv.c_str());
    }

    return caseName;
}


const std::string FlagType::information() const
{
    return TextUtils::format("%s: %s",
                             label().c_str(),
                             flagHelp.c_str());
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
