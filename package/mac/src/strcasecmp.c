#include <string.h>

#include "strcasecmp.h"


// This routine is a modified from  __ssgStringEqual()
int strcasecmp ( const char *s1, const char *s2 )
{
  char c1;
  char c2;
  int i;
  
  int l1 = (s1==NULL)? 0 : strlen ( s1 ) ;
  int l2 = (s2==NULL)? 0 : strlen ( s2 ) ;

  if ( l1 > l2 ) return  1;
  if ( l1 < l2 ) return -1;
 
  for ( i = 0 ; i < l1 ; i++ )
  {
    c1 = s1[i] ;
    c2 = s2[i] ;

    if ( c1 == c2 )
     continue ;

    if ( c1 >= 'a' && c1 <= 'z' )
      c1 = c1 - ('a'-'A') ;

    if ( c2 >= 'a' && c2 <= 'z' )
      c2 = c2 - ('a'-'A') ;

    if ( c1 != c2 )
     return 1 ;
  }

  return 0 ;
}

int strncasecmp ( const char *s1, const char *s2, int n )
{
  char c1 ;
  char c2 ;
  int i;
  
  int l1 = (s1==NULL)? 0 : strlen ( s1 ) ;
  int l2 = (s2==NULL)? 0 : strlen ( s2 ) ;

  for ( i = 0 ; i < n ; i++ )
  {
    c1 = s1[i] ;
    c2 = s2[i] ;

    if ( c1 == c2 )
     continue ;

    if ( c1 >= 'a' && c1 <= 'z' )
      c1 = c1 - ('a'-'A') ;

    if ( c2 >= 'a' && c2 <= 'z' )
      c2 = c2 - ('a'-'A') ;

    if ( c1 != c2 )
     return 1 ;
  }

  return 0 ;
}
