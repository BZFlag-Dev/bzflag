TorBlock

TorBlock automatically bans any user that joins the game who uses the TOR
anonymity network.

Usage

When loaded TorBlock loads up the TOR master IP list from
http://belegost.mit.edu/tor/status/authority, and will from then on
disconnect all users from any of those IP addresses. The TOR network is not
intended for game play use and is generally only used in BZFlag for server
spamming or other disruptive effects. TorBlock will automatically update
its ban list every hour with new IPs. TorBlock works independently from the
built in Banfile capability of BZFS so it will automatically allow users
from blocked IPs as soon as they are removed from the TOR master list.