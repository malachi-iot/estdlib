/* =============================================================================
 *
 *  Description: This is a C++ implementation for Thread MainThread
 *
 * -----------------------------------------------------------------------------
 *  Comments:
 *
 * ===========================================================================*/

#include "MainThread.h"
#include <new>

#pragma file_attr("OS_Component=Threads")
#pragma file_attr("Threads")

/******************************************************************************
 *  MainThread Run Function (MainThread's main{})
 */
 
void
MainThread::Run()
{
    // TODO - Put the thread's "main" Initialization HERE

    while (1)
    {
        // TODO - Put the thread's "main" body HERE

        // Use a "break" instruction to exit the "while (1)" loop
    }

    // TODO - Put the thread's exit from "main" HERE
	// A thread is automatically Destroyed when it exits its run function
}

/******************************************************************************
 *  MainThread Error Handler
 */
 
int
MainThread::ErrorHandler()
{
    /* TODO - Put this thread's error handling code HERE */

    /* The default ErrorHandler (called below)  raises
     * a kernel panic and stops the system */
    return (VDK::Thread::ErrorHandler());
}

/******************************************************************************
 *  MainThread Constructor
 */
 
MainThread::MainThread(VDK::Thread::ThreadCreationBlock &tcb)
    : VDK::Thread(tcb)
{
    /* This routine does NOT run in new thread's context.  Any non-static thread
     *   initialization should be performed at the beginning of "Run()."
     */

    // TODO - Put code to be executed when this thread has just been created HERE

}

/******************************************************************************
 *  MainThread Destructor
 */
 
MainThread::~MainThread()
{
    /* This routine does NOT run in the thread's context.  Any VDK API calls
     *   should be performed at the end of "Run()."
     */

    // TODO - Put code to be executed just before this thread is destroyed HERE

}

/******************************************************************************
 *  MainThread Externally Accessible, Pre-Constructor Create Function
 */
 
VDK::Thread*
MainThread::Create(VDK::Thread::ThreadCreationBlock &tcb)
{
    /* This routine does NOT run in new thread's context.  Any non-static thread
     *   initialization should be performed at the beginning of "Run()."
     */

    	return new (tcb) MainThread(tcb);
}

/* ========================================================================== */
