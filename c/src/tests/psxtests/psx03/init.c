/* 
 *  COPYRIGHT (c) 1989, 1990, 1991, 1992, 1993, 1994.
 *  On-Line Applications Research Corporation (OAR).
 *  All rights assigned to U.S. Government, 1994.
 *
 *  This material may be reproduced by or for the U.S. Government pursuant
 *  to the copyright license under the clause at DFARS 252.227-7013.  This
 *  notice must appear in all copies of this file and its derivatives.
 *
 *  $Id$
 */

#define CONFIGURE_INIT
#include "system.h"
#include <signal.h>
#include <errno.h>

volatile int Signal_occurred;
volatile int Signal_count;

void Signal_handler(
  int signo
)
{
  Signal_count++;
  printf(
    "Signal: %d caught by 0x%x (%d)\n",
    signo,
    pthread_self(),
    Signal_count
  );
  Signal_occurred = 1;
}

void *POSIX_Init(
  void *argument
)
{
  int               status;
  struct timespec   timeout;
  struct sigaction  act;
  sigset_t          mask;
  sigset_t          waitset;
  int               signo;
  siginfo_t         siginfo;

  puts( "\n\n*** POSIX TEST 3 ***" );

  /* set the time of day, and print our buffer in multiple ways */

  set_time( TM_FRIDAY, TM_MAY, 24, 96, 11, 5, 0 );

  /* get id of this thread */

  Init_id = pthread_self();
  printf( "Init's ID is 0x%08x\n", Init_id );

  /* install a signal handler */

  status = sigemptyset( &act.sa_mask );
  assert( !status );

  act.sa_handler = Signal_handler;
  act.sa_flags   = 0;
 
  sigaction( SIGUSR1, &act, NULL );

  /* initialize signal handler variables */

  Signal_count = 0;
  Signal_occurred = 0;

  /*
   *  wait on SIGUSR1 for 3 seconds, will timeout 
   */

  /* initialize the signal set we will wait for to SIGUSR1 */

  status = sigemptyset( &waitset );
  assert( !status );

  status = sigaddset( &waitset, SIGUSR1 );
  assert( !status );

  timeout.tv_sec = 3;
  timeout.tv_nsec = 0;

  puts( "Init: waiting on any signal for 3 seconds." );
  signo = sigtimedwait( &waitset, &siginfo, &timeout );
  assert( signo == -1 );

  if ( errno == EAGAIN ) 
    puts( "Init: correctly timed out waiting for SIGUSR1." );
  else
    printf( "sigtimedwait returned wrong errno - %d\n", errno );

  Signal_occurred = 0;

  /*
   *  wait on SIGUSR1 for 3 seconds, will timeout because Task_1 sends SIGUSR2
   */

  empty_line();

  /* initialize a mask to block SIGUSR2 */

  status = sigemptyset( &mask );
  assert( !status );

  status = sigaddset( &mask, SIGUSR2 );
  assert( !status );

  printf( "Init: Block SIGUSR2\n" );
  status = sigprocmask( SIG_BLOCK, &mask, NULL );
  assert( !status );

  /* create a thread */

  status = pthread_create( &Task_id, NULL, Task_1, NULL );
  assert( !status );

  /* signal handler is still installed, waitset is still set for SIGUSR1 */
   
  timeout.tv_sec = 3;
  timeout.tv_nsec = 0;
 
  puts( "Init: waiting on any signal for 3 seconds." );
  signo = sigtimedwait( &waitset, &siginfo, &timeout );

     /* switch to Task 1 */

  if ( errno == EAGAIN )
    puts( "Init: correctly timed out waiting for SIGUSR1." );
  else
    printf( "sigtimedwait returned wrong errno - %d\n", errno );
  assert( signo == -1 );
 
  /*
   *  wait on SIGUSR1 for 3 seconds, Task_2 will send it to us
   */

  /* create a thread */

  status = pthread_create( &Task_id, NULL, Task_2, NULL );
  assert( !status );

  /* signal handler is still installed, mask is still set for SIGUSR1 */
 
  /* wait on SIGUSR1 for 3 seconds, will receive SIGUSR1 from Task_2 */
 
  timeout.tv_sec = 3;
  timeout.tv_nsec = 0;
 
  /* just so we can check that these were altered */

  siginfo.si_code = -1;
  siginfo.si_signo = -1;
  siginfo.si_value.sival_int = -1;

  puts( "Init: waiting on any signal for 3 seconds." );
  signo = sigtimedwait( &mask, &siginfo, &timeout );
  printf( "Init: correctly received SIGUSR1 - %d\n", siginfo.si_signo );
  assert( signo == SIGUSR1 );
  assert( siginfo.si_signo == SIGUSR1 );
  assert( siginfo.si_code == SI_USER );
  assert( siginfo.si_value.sival_int != -1 );   /* rtems does always set this */
 

/*
  status = sigemptyset( &mask );
  assert( !status );

  status = sigaddset( &mask, SIGUSR1 );
  assert( !status );

  printf( "Init: Block SIGUSR1\n" );
  status = sigprocmask( SIG_BLOCK, &mask, NULL );
  assert( !status );

  status = sigpending( &pending_set );
  assert( !status );
  printf( "Init: Signals pending 0x%08x\n", pending_set );
  
  printf( "Init: send SIGUSR1 to self\n" );
  status = pthread_kill( Init_id, SIGUSR1 );
  assert( !status );

  status = sigpending( &pending_set );
  assert( !status );
  printf( "Init: Signals pending 0x%08x\n", pending_set );
  
  printf( "Init: Unblock SIGUSR1\n" );
  status = sigprocmask( SIG_UNBLOCK, &mask, NULL );
  assert( !status );

*/

  /*
   *  Loop for 5 seconds seeing how many signals we catch 
   */

/*
  tr.tv_sec = 5;
  tr.tv_nsec = 0;
 
  do {
    tv = tr;

    Signal_occurred = 0;

    status = nanosleep ( &tv, &tr );
    assert( !status );

    printf(
      "Init: signal was %sprocessed with %d:%d time remaining\n",
      (Signal_occurred) ? "" : "not ",
      (int) tr.tv_sec,
      (int) tr.tv_nsec
   );

  } while ( tr.tv_sec || tr.tv_nsec );

*/

  /* exit this thread */

  puts( "*** END OF POSIX TEST 3 ***" );
  exit( 0 );

  return NULL; /* just so the compiler thinks we returned something */
}
