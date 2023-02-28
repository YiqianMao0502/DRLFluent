#ifndef __VXTHREAD_H__
#define __VXTHREAD_H__
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Project:     omniORB
%% Filename:    $Filename$
%% Author:      Guillaume/Bill ARRECKX
%%              Copyright Wavetek Wandel & Goltermann, Plymouth.
%% Description: OMNI thread implementation classes for VxWorks threads
%% Notes:
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/


///////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////
#include <vxWorks.h>
#include <semLib.h>
#include <taskLib.h>


///////////////////////////////////////////////////////////////////////////
// Externs prototypes
///////////////////////////////////////////////////////////////////////////
extern "C" void omni_thread_wrapper(void* ptr);


///////////////////////////////////////////////////////////////////////////
// Exported macros
// Note: These are added as private members in each class implementation.
///////////////////////////////////////////////////////////////////////////
#define OMNI_MUTEX_IMPLEMENTATION \
   SEM_ID mutexID;	\
   bool m_bConstructed;

#define OMNI_CONDITION_IMPLEMENTATION \
   SEM_ID sema_;

#define OMNI_SEMAPHORE_IMPLEMENTATION \
   SEM_ID semID;

#define OMNI_MUTEX_LOCK_IMPLEMENTATION                  \
	if(semTake(mutexID, WAIT_FOREVER) != OK)	\
	{	\
		throw omni_thread_fatal(errno);	\
	}

#define OMNI_MUTEX_TRYLOCK_IMPLEMENTATION               \
	return semTake(mutexID, NO_WAIT) == OK;

#define OMNI_MUTEX_UNLOCK_IMPLEMENTATION                \
	if(semGive(mutexID) != OK)	\
	{	\
		throw omni_thread_fatal(errno);	\
	}

#define OMNI_THREAD_IMPLEMENTATION \
   friend void omni_thread_wrapper(void* ptr); \
   static int vxworks_priority(priority_t); \
   omni_condition *running_cond; \
   void* return_val; \
   int tid; \
   public: \
   static void attach(void); \
   static void detach(void); \
   static void show(void);


///////////////////////////////////////////////////////////////////////////
// Porting macros
///////////////////////////////////////////////////////////////////////////
// This is a wrapper function for the 'main' function which does not exists
//  as such in VxWorks. The wrapper creates a launch function instead,
//  which spawns the application wrapped in a omni_thread.
// Argc will always be null.
///////////////////////////////////////////////////////////////////////////
#define main( discarded_argc, discarded_argv ) \
        omni_discard_retval() \
          { \
          throw; \
          } \
        int omni_main( int argc, char **argv ); \
        void launch( ) \
          { \
          omni_thread* th = new omni_thread( (void(*)(void*))omni_main );\
          th->start();\
          }\
        int omni_main( int argc, char **argv )


#endif // ndef __VXTHREAD_H__
