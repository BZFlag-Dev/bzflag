#!/usr/bin/python3
# -*- coding: utf-8 -*-
#
# Inspired from misc/bzfquery.pl
#
# Author: Frédéric Jolliton [aka FredCods]
#         <fj@tuxee.net>
#         <fred@jolliton.com>
#
# This script can be used either as a module or
# directly from the command line.
#
# Example of use:
#
#   from bzfquery import *
#   s = Server( 'localhost' , 5154 )
#   game = s.queryGame()
#   print( game[ 'style' ] )
#   teams , players = s.queryPlayers()
#   print( teams.get( 'rogue' ) )
#   print( players[ 0 ] )
#
# output the following:
#
# TeamFFA
# {'size': 1, 'score': 0, 'won': 0, 'lost': 0}
# {'pId': 0, 'type': 0, 'team': 'observer', 'score': 0, 'won': 0, 'lost': 0, 'tks': 0, 'sign': 'FredCods', 'motto': ''}
#
#

import sys
import os
import struct
import socket
import time
import select
from functools import reduce

#
# Default hostname and port
#
defaultHostname = 'localhost'
defaultPort = 5154

#
# Default timeout is seconds.
#
# Set to None for no timeout (not a good idea)
#
defaultTimeout = 10.

#
# Throw an exception if timeout occur.
#
hardTimeout = True

class Error( Exception ) : pass

def s2n( s ) :

	return reduce( lambda a , b : 256 * a + ord( b ) , s , 0 )

gameStyles = ['TeamFFA', 'ClassicCTF', 'OpenFFA', 'RabbitChase']

# must mirror enum GameOptions in global.h
gameOptions = [
	( 'flags'        , 0x0002 ) ,
	( 'jumping'      , 0x0008 ) ,
	( 'inertia'      , 0x0010 ) ,
	( 'ricochet'     , 0x0020 ) ,
	( 'shaking'      , 0x0040 ) ,
	( 'antidote'     , 0x0080 ) ,
	( 'handicap'     , 0x0100 ) ,
	( 'no-team-kills', 0x0400 )
]

teamsName = [
	'rogue' ,
	'red' ,
	'green' ,
	'blue' ,
	'purple' ,
	'observer' ,
	'rabbit'
]

playerType = [
	'tank' ,
	'observer' ,
	'robot tank'
]

def decodeOptions( n ) :

	flags = []
	for option , bit in gameOptions :
		if n & bit :
			flags.append( option )
	return flags

def receive( sock , size , timeout = None ) :

	'''Receive up to 'size' byte from socket 'sock'. If timeout is not
	None, wait up to 'timeout' seconds for some answer, returning
	None if nothing was available in the meantime or throwing an
	exception according to hardTimeout global setting.'''

	if timeout is not None :
		#
		# First wait that something is available on socket 'sock'
		#
		timeLimit = time.time() + timeout
		while 1 :
			t = timeLimit - time.time()
			if t <= 0 :
				if hardTimeout :
					raise Error( 'Timeout' )
				else :
					return
			r , w , x = select.select( [ sock ] , [] , [] , t )
			if sock in r :
				break
	return sock.recv( size )

class Server :

	def __init__( self , host = '127.0.0.1' , port = 5154 ) :

		self.sock = socket.socket( socket.AF_INET , socket.SOCK_STREAM )
		self.sock.connect( ( host , port ) )
		self.sock.sendall( b"BZFLAG\r\n\r\n" )
		header = receive( self.sock , 9 , defaultTimeout )
		magic , self.protocol , self.id = \
			struct.unpack( '4s4sb' , header )
		if magic != b'BZFS' :
			raise Error( 'Not a bzflag server.' )
		if self.protocol not in [ b'0223' ] :
			raise Error( 'Not compatible with server.' )

	def cmd( self , command ) :

		if len( command ) != 2 :
			raise Error( 'Command must be 2 characters long.' )
		self.sock.sendall( struct.pack( '>2H' , 0 , s2n( command ) ) )
		return self.getResponse( command )

	def _getPacket( self ) :

		pktData = receive( self.sock , 4 , defaultTimeout )
		if pktData is None :
			return
		size , code = struct.unpack( '>H2s' , pktData )
		data = receive( self.sock , size , defaultTimeout )
		if data is None :
			return
		return code.decode('utf-8') , data

	#
	# If expectedCode is none, we return the first packet that is
	# not a msgGameTime packet.
	#
	# If expectedCode is not none, we return the first packet that
	# match this code, discarding all other packets.
	#
	def getResponse( self , expectedCode ) :

		timeLimit = time.time() + defaultTimeout
		code = None
		while time.time() < timeLimit :
			packet = self._getPacket()
			if packet is not None :
				code , data = packet
				if code == expectedCode :
					break
		else :
			# With 'hardTimeout = True', this may never be reached, because
			# receive() will time out with the same time limit.
			if code is not None :
				raise Error( 'Got wrong response code (got %r, expected %r)' \
					     % ( code , expectedCode ) )
			else :
				raise Error( 'No answer' )
		return data

	def queryGame( self ) :

		data = self.cmd( 'qg' )
		data = struct.unpack( '>22H' , data )
		style , options, maxPlayers , maxShots , rogueSize , \
			redSize , greenSize , blueSize , purpleSize , obsSize, \
			rogueMax , redMax , greenMax , blueMax , purpleMax , obsMax, \
			shakeWins , shakeTimeout , maxPlayerScore , maxTeamScore , \
			maxTime , elapsedTime \
			= data
		style = gameStyles[style]
		options = decodeOptions( options )
		teams = {
			'rogue'    : ( rogueSize  , rogueMax ) ,
			'red'      : ( redSize    , redMax ) ,
			'green'    : ( greenSize  , greenMax ) ,
			'blue'     : ( blueSize   , blueMax ) ,
			'purple'   : ( purpleSize , purpleMax ) ,
			'observer' : ( obsSize    , obsMax ) ,
		}
		infos = {
			'style' : style ,
			'options' : options ,
			'teams' : teams ,
			'maxPlayerScore' : maxPlayerScore ,
			'maxTeamScore' : maxTeamScore ,
			'maxPlayers' : maxPlayers ,
			'maxShots' : maxShots ,
			'maxTime' : maxTime / 10 ,
			'elapsedTime' : elapsedTime / 10 ,
		}
		if 'shaking' in options :
			infos[ 'shake' ] = { 'wins' : shakeWins , 'timeout' : shakeTimeout / 10. }
		return infos

	def queryPlayers( self ) :

		data = self.cmd( 'qp' )
		data = struct.unpack( '>2H' , data )
		numTeams_ , numPlayers = data
		data = self.getResponse( 'tu' )
		numTeamInfo , data = data[ : 1 ] , data[ 1 : ]
		numTeams , = struct.unpack( '>1B', numTeamInfo )
		#if numTeams != numTeams_ :
		#	raise Error( 'Inconsistency in numTeams (got %d and %d)' \
		#		% ( numTeams_ , numTeams ) )
		teamsInfo = {}
		for i in range( numTeams ) :
			teamInfo , data = data[ : 8 ] , data[ 8 : ]
			team , size , won , lost = struct.unpack( '>4H' , teamInfo )
			score = won - lost
			teamsInfo[ teamsName[ team ] ] = {
				'size'  : size ,
				'score' : score ,
				'won'   : won ,
				'lost'  : lost
			}
		playersInfo = []
		for i in range( numPlayers ) :
			data = self.getResponse( 'ap'  )
			pId , type , team , won , lost , tks , sign , motto = \
				struct.unpack( '>b5H32s128s' , data )
			playerInfo = {
				'pId'   : pId ,
				'type'  : type ,
				'team'  : teamsName[ team ] ,
				'score' : won - lost ,
				'won'   : won ,
				'lost'  : lost ,
				'tks'   : tks ,
				'sign'  : sign.rstrip( b'\x00' ).decode( 'utf-8' ) ,
				'motto' : motto.rstrip( b'\x00' ).decode( 'utf-8' )
			}
			playersInfo.append( playerInfo )
		return teamsInfo , playersInfo

def getAndPrintStat( hostname , port ) :

	s = Server( hostname , port )
	game = s.queryGame()

	print('Statistics of the BZFlag server %s (port %s)' % ( hostname , port ))
	print()
	print('--[ GAME ]' + '-' * 40)
	print()
	print('Type:' , game[ 'style' ])
	print('Options:' , ' '.join( game[ 'options' ] ))
	print()
	print('Max players: %s   Max shots: %s' % ( game[ 'maxPlayers' ] , game[ 'maxShots' ] ))
	print()
	print('Teams     Size   Max')
	print('-' * 20)
	for team in teamsName :
		t = game[ 'teams' ].get( team )
		if t is not None :
			print('%-8s %5d %5d' % ( team , t[ 0 ] , t[ 1 ] ))
	shaking = game.get( 'shake' )
	if shaking :
		print()
		print('Shaking bad flag: wins: %d, timeout: %g' % ( shaking[ 'wins' ] , shaking[ 'timeout' ] ))
	print()
	print('Max player score: %d' % game[ 'maxPlayerScore' ])
	print('Max team score: %d' % game[ 'maxTeamScore' ])
	print('Max time: %g' % game[ 'maxTime' ])
	print('Time elapsed: %g' % game[ 'elapsedTime' ])

	teams , players = s.queryPlayers()
	print()
	print('--[ TEAMS ]' + '-' * 39)
	print()
	print('Teams     Size  Score  Won  Lost')
	print('-' * 32)
	for team in teamsName :
		t = teams.get( team )
		if t is not None :
			print('%-8s %5d %5d %5d %5d' \
				% ( team , t[ 'size' ] , t[ 'score' ] , t[ 'won' ] , t[ 'lost' ] ))

	print()
	print('--[ PLAYERS ]' + '-' * 37)
	print()
	print('Team     Score   Won  Lost Type       Sign')
	print('-' * 60)
	players.sort(key=lambda x: x['score'], reverse=True)
	for player in players :
		sign , team , score , won , lost , motto = \
			player[ 'sign' ] , player[ 'team' ] , \
			player[ 'score' ] , player[ 'won' ] , player[ 'lost' ] , \
			player[ 'motto' ]
		try :
			type = playerType[ player[ 'type' ] ]
		except :
			type = 'Unknown player type %s' % player.get( 'tks' )
		name = sign
		if motto : name = name + ' <%s>' % motto
		print('%-8s %5d %5d %5d %-10s %s' % ( team , score , won , lost , type , name ))

def usage() :

	print('''Usage: bzfquery.py [OPTIONS] [hostname [port]]

 -h, --help  Display this help.

Report bugs on https://github.com/BZFlag-Dev/bzflag/issues''')

def main() :

	hostname , port = defaultHostname , defaultPort
	import getopt
	options , parameters = getopt.getopt( sys.argv[ 1 : ] , 'h' , ( 'help' , ) )

	for option , argument in options :
		if option in [ '-h' , '--help' ] :
			usage()
			sys.exit( 0 )

	if len( parameters ) > 2 :
		usage()
		sys.exit( 0 )
	if 1 <= len( parameters ) <= 2 :
		hostname = parameters[ 0 ]
	if len( parameters ) == 2 :
		port = int( parameters[ 1 ] )

	getAndPrintStat( hostname , port )

if __name__ == '__main__' :
	main()
