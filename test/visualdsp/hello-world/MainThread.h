/* =============================================================================
 *
 *  Description: This is a C++ header file for Thread MainThread
 *
 * -----------------------------------------------------------------------------
 *  Comments:
 *
 * ===========================================================================*/

#ifndef _MainThread_H_    
#define _MainThread_H_

#pragma diag(push)
#pragma diag(suppress: 177,401,451,826,831,1462)

#include "VDK.h"

#pragma diag(pop)

class MainThread : public VDK::Thread 
{
public:
    MainThread(VDK::Thread::ThreadCreationBlock&);
    virtual ~MainThread();
    virtual void Run();
    virtual int ErrorHandler();
    static VDK::Thread* Create(VDK::Thread::ThreadCreationBlock&);
};

#endif /* _MainThread_H_ */

/* ========================================================================== */
