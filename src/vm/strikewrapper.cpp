#include "strikewrapper.h"
#include "log.h"
#include "common.h"
#include "threads.h"
#include <stdio.h>
#include <util.hpp>
//#include <string>
//#include <dlfcn.h>
//#include <iostream>
//#include <lldbservices.h>
//#include <services.h>

//extern HRESULT InitCorDebugInterface();

/*void *LoadModule(const char *moduleName) {
  std::string modulePath("./");
  modulePath.append(moduleName);
  void *moduleHandle = dlopen(modulePath.c_str(), RTLD_NOW);
  if (moduleHandle == NULL)
  {
    std::cerr << "dlopen(" << modulePath.c_str() << ") failed " << dlerror() << std::endl;
  }
  return moduleHandle;
}

DECLARE_API(my_get_frame) {
  INIT_API (); 
 
  HRESULT hr;
  void * sosHandle = LoadModule(MAKEDLLNAME_A("sos"));
  HRESULT(*initCorDebugInterface)() = (HRESULT (*)())dlsym(sosHandle, "_Z21InitCorDebugInterfacev");
  if(FAILED(hr = initCorDebugInterface())) {
    LogProfilerActivity("Failed initializing CorDebugInterface");
  } else {
    LogProfilerActivity("Success initializing CorDebugInterface");
  }
}
*/

class CheckpointData {
public:
    T_CONTEXT registerContext;
    PBYTE stackBuffer;
    size_t stackBufferSize;
    ~CheckpointData() {
        delete[] stackBuffer;
    }
};

class CheckpointNode {
public:
    CheckpointNode()
        : data(nullptr), next(nullptr)
    {}
    CheckpointData* data;
    CheckpointNode* next;
    ~CheckpointNode() 
    {
        delete data;
    }
};

static CheckpointNode* g_CheckpointList = nullptr;

extern UINT_PTR ProfileGetIPFromPlatformSpecificHandle(void * handle);
typedef struct _PROFILE_PLATFORM_SPECIFIC_DATA
{
    void       *Pc;    
    union                   // Float arg registers as 32-bit (s0-s15) and 64-bit (d0-d7)
    {
        UINT32  s[16];
        UINT64  d[8];
    };
    FunctionID  functionId;
    void       *probeSp;    // stack pointer of managed function 
    void       *profiledSp; // location of arguments on stack
    LPVOID      hiddenArg;
    UINT32      flags;
    UINT32      r[13];
} PROFILE_PLATFORM_SPECIFIC_DATA, *PPROFILE_PLATFORM_SPECIFIC_DATA;

void NotifySave(void* eltInfo) {
    (void)eltInfo;
    static int counter = 0;
    ++counter;
    PPROFILE_PLATFORM_SPECIFIC_DATA platformSpecificData = *static_cast<PPROFILE_PLATFORM_SPECIFIC_DATA*>(eltInfo);
    T_CONTEXT context;
    memset(&context, 0, sizeof(T_CONTEXT));
    context.R0 = (UINT32)platformSpecificData->r[0];
    context.R1 = (UINT32)platformSpecificData->r[1];
    context.R2 = (UINT32)platformSpecificData->r[2];
    context.R3 = (UINT32)platformSpecificData->r[3];
    context.R4 = (UINT32)platformSpecificData->r[4];
    context.R5 = (UINT32)platformSpecificData->r[5];
    context.R6 = (UINT32)platformSpecificData->r[6];
    context.R7 = (UINT32)platformSpecificData->r[7];
    context.R8 = (UINT32)platformSpecificData->r[8];
    context.R9 = (UINT32)platformSpecificData->r[9];
    context.R10 = (UINT32)platformSpecificData->r[10];
    context.R11 = (UINT32)platformSpecificData->r[11];
    context.R12 = (UINT32)platformSpecificData->r[12];
    context.Pc = (UINT32)platformSpecificData->Pc;
    context.Sp = (UINT32)platformSpecificData->probeSp;

    if (g_CheckpointList != nullptr)
        Thread::VirtualUnwindCallFrame(&context, NULL, NULL);
    if (context.R11 < context.Sp)
        return;
    // now we have a valid frame, record it
    CheckpointData* data = new CheckpointData();
    data->registerContext = context;
    data->stackBufferSize = context.R11 - context.Sp;

    printf("counter: %d\n", counter);
    data->stackBuffer = (PBYTE) new BYTE[data->stackBufferSize];
    memcpy(data->stackBuffer, (void*)context.Sp, data->stackBufferSize);

    CheckpointNode* oldNode = g_CheckpointList;
    g_CheckpointList = new CheckpointNode();
    g_CheckpointList->next = oldNode;
    g_CheckpointList->data = data;

}

void NotifyPop() {
    if (g_CheckpointList == nullptr)
        return;
    CheckpointNode *next = g_CheckpointList->next;
    delete g_CheckpointList;
    g_CheckpointList = next;
}

void NotifyInitialize() {
    DACNotify::DoSaveStateNotification(&g_CheckpointList);
}