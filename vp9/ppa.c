#include <stdlib.h>
#include <stdio.h>

#if defined(WINDOWS) || defined(_WIN32)
#include <Windows.h>
#endif

#if defined(__ANDROID__)
#include <dlfcn.h>
#endif

#include "vp9/ppa.h"

#if !defined(PPA_DISABLE)

#ifdef PPA_REGISTER_CPU_EVENT
#undef PPA_REGISTER_CPU_EVENT
#endif

#ifdef PPA_REGISTER_CPU_EVENT2GROUP
#undef PPA_REGISTER_CPU_EVENT2GROUP
#endif

#define PPA_REGISTER_CPU_EVENT2GROUP(x, y) #x, #y,
#define PPA_REGISTER_CPU_EVENT(x) PPA_REGISTER_CPU_EVENT2GROUP(x, NoGroup)
char* PPACpuAndGroup[] = {
    #include "vp9/ppaCPUEvents.h"
    ""
  };
#undef PPA_REGISTER_CPU_EVENT
#undef PPA_REGISTER_CPU_EVENT2GROUP

#if defined(WINDOWS) || defined(_WIN32)

static int init_count = 0;

bool dumbrt() {
  return 0;
}

HMODULE ppaHandle;
FUNC_PPAInit*       PPAInitFunc;
FUNC_PPADel*                PPADelFunc;
FUNC_PPAStartCpuEvent      *       PPAStartCpuEvent;
FUNC_PPAStopCpuEvent     *       PPAStopCpuEvent;
FUNC_PPAIsEventEnable      *     PPAIsEventEnable;
FUNC_PPARegisterCpuEvent     *       PPARegisterCpuEvent;
FUNC_PPARegisterGrpName      *       PPARegisterGrpName;
FUNC_PPATIDCpuEvent         *       PPATIDCpuEvent;
FUNC_PPADebugCpuEvent        *       PPADebugCpuEvent;
FUNC_PPARegisterCpuEventExGrpID * PPARegisterCpuEventExGrpID;
FUNC_PPASetGrpCpuEventEnDis     * PPASetGrpCpuEventEnDis;
FUNC_PPASetSingleCpuEventEnDis  * PPASetSingleCpuEventEnDis;

void ErrorManage() {
}

void initializePPA() {
  if (ppaHandle) {
    ++init_count;
    return;
  }

  ppaHandle = LoadLibrary(PPA_LIB_NAME);
  if (!ppaHandle) {
    return;
  }


  /** get the function pointers to the event src api */
  PPAInitFunc = (FUNC_PPAInit*)GetProcAddress(ppaHandle, "InitPpaUtil");
  PPADelFunc = (FUNC_PPADel*)GetProcAddress(ppaHandle, "DeletePpa");
  PPAStartCpuEvent = (FUNC_PPAStartCpuEvent *)
      GetProcAddress(ppaHandle, "mcw_ppaStartCpuEvent");
  PPAStopCpuEvent = (FUNC_PPAStopCpuEvent *)
      GetProcAddress(ppaHandle, "mcw_ppaStopCpuEvent");
  PPAIsEventEnable = (FUNC_PPAIsEventEnable *)
      GetProcAddress(ppaHandle, "mcw_ppaIsEventEnable");
  PPARegisterCpuEvent = (FUNC_PPARegisterCpuEvent *)
      GetProcAddress(ppaHandle, "mcw_ppaRegisterCpuEvent");
  PPARegisterGrpName = (FUNC_PPARegisterGrpName *)
      GetProcAddress(ppaHandle, "mcw_ppaRegisterGrpName");
  PPATIDCpuEvent = (FUNC_PPATIDCpuEvent *)
      GetProcAddress(ppaHandle, "mcw_ppaTIDCpuEvent");
  PPADebugCpuEvent = (FUNC_PPADebugCpuEvent *)
      GetProcAddress(ppaHandle, "mcw_ppaDebugCpuEvent");
  PPARegisterCpuEventExGrpID = (FUNC_PPARegisterCpuEventExGrpID *)
      GetProcAddress(ppaHandle, "mcw_ppaRegisterCpuEventExGrpID");
  PPASetGrpCpuEventEnDis = (FUNC_PPASetGrpCpuEventEnDis *)
      GetProcAddress(ppaHandle, "mcw_ppaSetGrpCpuEventEnDis");
  PPASetSingleCpuEventEnDis = (FUNC_PPASetSingleCpuEventEnDis *)
      GetProcAddress(ppaHandle, "mcw_ppaSetSingleCpuEventEnDis");


  if (!PPAInitFunc  || !PPADelFunc || !PPAStartCpuEvent
    || !PPAStopCpuEvent || !PPAIsEventEnable
    || !PPASetGrpCpuEventEnDis|| !PPARegisterCpuEvent
    || !PPARegisterGrpName || !PPATIDCpuEvent || !PPADebugCpuEvent
    || !PPARegisterCpuEventExGrpID
    || !PPASetSingleCpuEventEnDis) {
    FreeLibrary(ppaHandle);
    ppaHandle = 0;
    printf("Load function fails\n");
    exit(0);
    return;
  }

  ++init_count;
}

void releasePPA() {
  if (--init_count == 0) {
    PPADelFunc();
  }
}

#elif defined(__ANDROID__)
static void *ppaHandle;
static int init_count = 0;

FUNC_PPAInit *PPAInitFunc;
FUNC_PPADel *PPADelFunc;
FUNC_PPAStartCpuEvent *PPAStartCpuEvent;
FUNC_PPAStopCpuEvent *PPAStopCpuEvent;
FUNC_PPAStartRSEvent *PPAStartRSEvent;
FUNC_PPAStopRSEvent *PPAStopRSEvent;
FUNC_PPAIsEventEnable *PPAIsEventEnable;
FUNC_PPARegisterCpuEvent *PPARegisterCpuEvent;
FUNC_PPARegisterGrpName *PPARegisterGrpName;
FUNC_PPATIDCpuEvent *PPATIDCpuEvent;
FUNC_PPADebugCpuEvent *PPADebugCpuEvent;
FUNC_PPARegisterCpuEventExGrpID *PPARegisterCpuEventExGrpID;
FUNC_PPASetGrpCpuEventEnDis     *PPASetGrpCpuEventEnDis;
FUNC_PPASetSingleCpuEventEnDis  *PPASetSingleCpuEventEnDis;

void initializePPA() {
  if (ppaHandle) {
    ++init_count;
    return;
  }

  ppaHandle = dlopen(PPA_LIB_NAME, RTLD_LAZY | RTLD_GLOBAL);
  if (!ppaHandle) {
    return;
  }

  /** get the function pointers to the event src api */
  PPAInitFunc = (FUNC_PPAInit*)dlsym(ppaHandle, "InitPpaUtil");
  PPADelFunc = (FUNC_PPADel*)dlsym(ppaHandle, "DeletePpa");
  PPAStartCpuEvent = (FUNC_PPAStartCpuEvent *)
      dlsym(ppaHandle, "mcw_ppaStartCpuEvent");
  PPAStopCpuEvent = (FUNC_PPAStopCpuEvent *)
      dlsym(ppaHandle, "mcw_ppaStopCpuEvent");
  PPAStartRSEvent = (FUNC_PPAStartRSEvent *)
      dlsym(ppaHandle, "mcw_ppaStartRSEvent");
  PPAStopRSEvent  = (FUNC_PPAStopRSEvent *)
      dlsym(ppaHandle, "mcw_ppaStopRSEvent");
  PPAIsEventEnable = (FUNC_PPAIsEventEnable *)
      dlsym(ppaHandle, "mcw_ppaIsEventEnable");
  PPARegisterCpuEvent = (FUNC_PPARegisterCpuEvent *)
      dlsym(ppaHandle, "mcw_ppaRegisterCpuEvent");
  PPARegisterGrpName = (FUNC_PPARegisterGrpName *)
      dlsym(ppaHandle, "mcw_ppaRegisterGrpName");
  PPATIDCpuEvent = (FUNC_PPATIDCpuEvent *)
      dlsym(ppaHandle, "mcw_ppaTIDCpuEvent");
  PPADebugCpuEvent = (FUNC_PPADebugCpuEvent *)
      dlsym(ppaHandle, "mcw_ppaDebugCpuEvent");
  PPARegisterCpuEventExGrpID = (FUNC_PPARegisterCpuEventExGrpID *)
      dlsym(ppaHandle, "mcw_ppaRegisterCpuEventExGrpID");
  PPASetGrpCpuEventEnDis = (FUNC_PPASetGrpCpuEventEnDis *)
      dlsym(ppaHandle, "mcw_ppaSetGrpCpuEventEnDis");
  PPASetSingleCpuEventEnDis = (FUNC_PPASetSingleCpuEventEnDis *)
      dlsym(ppaHandle, "mcw_ppaSetSingleCpuEventEnDis");


  if (!PPAInitFunc || !PPADelFunc || !PPAStartCpuEvent
    || !PPAStopCpuEvent || !PPARegisterCpuEvent
    || !PPARegisterGrpName || !PPATIDCpuEvent || !PPADebugCpuEvent
    || !PPARegisterCpuEventExGrpID
    || !PPAStartRSEvent || !PPAStopRSEvent
    /*|| !PPASetSingleCpuEventEnDis*/) {
    dlclose(ppaHandle);
    ppaHandle = 0;
    printf("Load function fails\n");
    abort();
    return;
  }
  PPAInitFunc((char**)PPACpuAndGroup, PPACpuGroupNums);
  ++init_count;
}

void releasePPA() {
  if (--init_count == 0) {
    PPADelFunc();
    dlclose(ppaHandle);
    ppaHandle = 0;
  }
}
#else

FUNC_PPAInit *PPAInitFunc;
FUNC_PPADel *PPADelFunc;
FUNC_PPAStartCpuEvent *PPAStartCpuEvent;
FUNC_PPAStopCpuEvent *PPAStopCpuEvent;
FUNC_PPAStartRSEvent *PPAStartRSEvent;
FUNC_PPAStopRSEvent *PPAStopRSEvent;
FUNC_PPAIsEventEnable *PPAIsEventEnable;
FUNC_PPARegisterCpuEvent *PPARegisterCpuEvent;
FUNC_PPARegisterGrpName *PPARegisterGrpName;
FUNC_PPATIDCpuEvent *PPATIDCpuEvent;
FUNC_PPADebugCpuEvent *PPADebugCpuEvent;
FUNC_PPARegisterCpuEventExGrpID * PPARegisterCpuEventExGrpID;
FUNC_PPASetGrpCpuEventEnDis *PPASetGrpCpuEventEnDis;
FUNC_PPASetSingleCpuEventEnDis *PPASetSingleCpuEventEnDis;

void initializePPA() {
}

void releasePPA() {
}

#endif
#endif
