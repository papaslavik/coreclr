#include "strikewrapper.h"
#include "log.h"
#include "unsafe.h"
#include "util.hpp"
//#include <util.hpp>
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

void InitCorProfiler() {
//  LoadModule(MAKEDLLNAME_A("coreclr"));
//  LoadModule(MAKEDLLNAME_A("mscordaccore"));
//  my_get_frame(new LLDBServices(), ""); 
//  InitCorDebugInterface();
    DACNotify::DoSaveStateNotification();
}
