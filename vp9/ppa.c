#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ppa.h"

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
#include "ppaCPUEvents.h"
    ""
};

#undef PPA_REGISTER_CPU_EVENT
#undef PPA_REGISTER_CPU_EVENT2GROUP

#include <dlfcn.h>
static void* ppaDllHandle;
static int init_count = 0;

FUNC_PPAInit* PPAInitFunc = NULL;
FUNC_PPADel* PPADelFunc = NULL;
FUNC_PPAStartCpuEvent * PPAStartCpuEvent = NULL;
FUNC_PPAStopCpuEvent * PPAStopCpuEvent = NULL;
FUNC_PPAStartRSEvent * PPAStartRSEvent = NULL;
FUNC_PPAStopRSEvent * PPAStopRSEvent = NULL;
FUNC_PPAStartSubSession * PPAStartSubSession = NULL;
FUNC_PPAStopSubSession * PPAStopSubSession = NULL;
FUNC_PPAIsEventEnable * PPAIsEventEnable = NULL;
FUNC_PPARegisterCpuEvent * PPARegisterCpuEvent = NULL;
FUNC_PPARegisterGrpName * PPARegisterGrpName = NULL;
FUNC_PPARegisterSubSession * PPARegisterSubSession = NULL;
FUNC_PPATIDCpuEvent * PPATIDCpuEvent = NULL;
FUNC_PPADebugCpuEvent * PPADebugCpuEvent = NULL;
FUNC_PPARegisterCpuEventExGrpID * PPARegisterCpuEventExGrpID = NULL;
FUNC_PPASetGrpCpuEventEnDis * PPASetGrpCpuEventEnDis = NULL;
FUNC_PPASetSingleCpuEventEnDis * PPASetSingleCpuEventEnDis = NULL;

//< opencl cl agent path
static const char* g_ppa_cl_agent_path = "/data/local/tmp/libOCLAgent.so";

#define MCW_PPA_MAX_PATH 256
//< opencl cl path
static char g_ppa_cl_path[MCW_PPA_MAX_PATH];

void initializePPA()
{
    if(ppaDllHandle)
    {
        ++init_count;
        return;
    }

    ppaDllHandle = dlopen(PPA_LIB_NAME, RTLD_LAZY|RTLD_GLOBAL);
    if(!ppaDllHandle)
    {
        return;
    }

    /** get the function pointers to the event src api */
    PPAInitFunc = (FUNC_PPAInit*)dlsym(ppaDllHandle, "InitPpaUtil");
    PPADelFunc = (FUNC_PPADel*)dlsym(ppaDllHandle, "DeletePpa");
    PPAStartCpuEvent = (FUNC_PPAStartCpuEvent *)dlsym(ppaDllHandle, "mcw_ppaStartCpuEvent");
    PPAStopCpuEvent = (FUNC_PPAStopCpuEvent *)dlsym(ppaDllHandle, "mcw_ppaStopCpuEvent");
    PPAStartRSEvent = (FUNC_PPAStartRSEvent *)dlsym(ppaDllHandle, "mcw_ppaStartRSEvent");
    PPAStopRSEvent = (FUNC_PPAStopRSEvent *)dlsym(ppaDllHandle, "mcw_ppaStopRSEvent");
    PPAStartSubSession = (FUNC_PPAStartSubSession*)dlsym(ppaDllHandle, "mcw_ppaStartSubSession");
    PPAStopSubSession = (FUNC_PPAStopSubSession*)dlsym(ppaDllHandle, "mcw_ppaStopSubSession");
    PPAIsEventEnable = (FUNC_PPAIsEventEnable *)dlsym(ppaDllHandle, "mcw_ppaIsEventEnable");
    PPARegisterCpuEvent = (FUNC_PPARegisterCpuEvent *)dlsym(ppaDllHandle, "mcw_ppaRegisterCpuEvent");
    PPARegisterGrpName = (FUNC_PPARegisterGrpName *)dlsym(ppaDllHandle, "mcw_ppaRegisterGrpName");
    PPARegisterSubSession = ( FUNC_PPARegisterSubSession* )dlsym( ppaDllHandle, "mcw_ppaRegisterSubSession" );
    PPATIDCpuEvent = (FUNC_PPATIDCpuEvent *)dlsym(ppaDllHandle, "mcw_ppaTIDCpuEvent");
    PPADebugCpuEvent = (FUNC_PPADebugCpuEvent *)dlsym(ppaDllHandle, "mcw_ppaDebugCpuEvent");
    PPARegisterCpuEventExGrpID = (FUNC_PPARegisterCpuEventExGrpID *)dlsym(ppaDllHandle, "mcw_ppaRegisterCpuEventExGrpID");
    PPASetGrpCpuEventEnDis = (FUNC_PPASetGrpCpuEventEnDis *)dlsym(ppaDllHandle, "mcw_ppaSetGrpCpuEventEnDis");
    PPASetSingleCpuEventEnDis = (FUNC_PPASetSingleCpuEventEnDis *)dlsym(ppaDllHandle, "mcw_ppaSetSingleCpuEventEnDis");

    if(!PPAInitFunc  || !PPADelFunc || !PPAStartCpuEvent || !PPAStopCpuEvent
            || !PPARegisterCpuEvent
            || !PPARegisterGrpName || !PPATIDCpuEvent || !PPADebugCpuEvent
            || !PPARegisterCpuEventExGrpID 
            || !PPAStartRSEvent || !PPAStopRSEvent )
    {
        dlclose(ppaDllHandle);
        ppaDllHandle = 0;
        return;
    }

    PPAInitFunc((char**)PPACpuAndGroup, PPACpuGroupNums);
    ++init_count;
}

void releasePPA()
{
    if(--init_count == 0)
    {
        PPADelFunc();
        dlclose(ppaDllHandle);
        ppaDllHandle = 0;
    }
}

bool IsPPAInstalled()
{
    return access(PPA_LIB_NAME, 0) == 0;
}

const char* GetCLLibraryPath(const char* ocl_path)
{
    const char* ret = NULL;
    if(IsPPAInstalled())
    {
        ret = g_ppa_cl_agent_path;
    }
    else
    {
        strncpy(g_ppa_cl_path, ocl_path, MCW_PPA_MAX_PATH);
        ret = g_ppa_cl_path;
    }
    return ret;
}

#endif //< PPA_DISABLE
