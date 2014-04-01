/*========================================================
Copyright (c) 2011 Multicoreware Inc. All rights reserved

======================================================= */

#ifndef _PPA_H_
 #define _PPA_H_

#ifdef PPA_REGISTER_CPU_EVENT
 #undef PPA_REGISTER_CPU_EVENT
#endif //< PPA_REGISTER_CPU_EVENT

#if __STDC_VERSION__  < 199901L
#endif

#undef PPA_CLEANUP
#ifdef PPA_DISABLE
 #define UNDEF_PARAM(x) 
 #define PPA_CLEANUP(x) UNDEF_PARAM(#x)
#else /* defined(PPA_DISABLE) */

#ifdef PPA_CODELINK
 #define P2STRING(x) #x
 #define PSTRING(x) P2STRING(x)
#define PRAGMA_MES __pragma("message(__FILE__)")

#if defined(WINDOWS)||defined(_WIN32)
 #define PPA_APICLEANUP(x) PRAGMA_MES \
  x
 #define PPA_CLEANUP(x) x
#else

#endif /* PPA_CODLELINK */

#else
#   define  PPA_CLEANUP(x) x
#   define  PPA_APICLEANUP(x) x
#endif /* !PPA_CODELINK */

#endif /* !defined(PPA_DISABLE) */

#if defined(WINDOWS) || defined(_WIN32)
#ifdef UNICODE
 #define PPA_LIB_NAME L"ppa.dll"
#else
 #define PPA_LIB_NAME "ppa.dll"
#endif /* UNICODE */
#else
 #define PPA_LIB_NAME "/data/local/tmp/libppa.so"
#endif /* !WINDOWS */

#ifndef NULL
 #define NULL 0
#endif //< NULL

/**
 * Register PPA event in a group
 * @param x event name
 * @param y group name
 */
#if !defined(PPA_DISABLE)
#define PPA_REGISTER_CPU_EVENT2GROUP(x, y) x,

#define PPA_REGISTER_CPU_EVENT(x) PPA_REGISTER_CPU_EVENT2GROUP(x, NoGroup)
enum PPACpuEventEnum {
    #include "ppaCPUEvents.h"
    PPACpuGroupNums
};
#undef PPA_REGISTER_CPU_EVENT
#undef PPA_REGISTER_CPU_EVENT2GROUP
#endif //< PPA_DISABLE

#ifdef __cplusplus
extern "C"{
    #endif //< __cplusplus

    typedef unsigned short SessionID;
    typedef unsigned short EventID;
    typedef unsigned char GrpID;
    #ifndef __cplusplus
    typedef enum {false = 0, true = 1} bool;
    #endif //< __cplusplus

    typedef void (FUNC_PPAInit)(char**, int);
    typedef void (FUNC_PPADel)();
    typedef void (FUNC_PPAStartCpuEvent)(EventID);
    typedef void (FUNC_PPAStopCpuEvent)(EventID);
    typedef void (FUNC_PPAStartRSEvent)(EventID);
    typedef void (FUNC_PPAStopRSEvent)(EventID);
    typedef void (FUNC_PPAStartSubSession)(SessionID);
    typedef void (FUNC_PPAStopSubSession)(SessionID);
    typedef bool (FUNC_PPAIsEventEnable)(EventID);
    typedef EventID (FUNC_PPARegisterCpuEvent)(const char*);
    typedef GrpID (FUNC_PPARegisterGrpName)(const char*);
    typedef SessionID (FUNC_PPARegisterSubSession)(const char*);
    typedef void (FUNC_PPATIDCpuEvent)(EventID, unsigned int);
    typedef void (FUNC_PPADebugCpuEvent)(EventID, unsigned int, unsigned int);
    typedef EventID (FUNC_PPARegisterCpuEventExGrpID)(const char*, GrpID);
    typedef int (FUNC_PPASetGrpCpuEventEnDis)(bool, GrpID);
    typedef bool (FUNC_PPASetSingleCpuEventEnDis)(bool, EventID);

    #ifndef PPA_DISABLE
    extern FUNC_PPAInit *  PPAInitFunc;
    extern FUNC_PPADel * PPADelFunc;
    extern FUNC_PPAStartCpuEvent * PPAStartCpuEvent;
    extern FUNC_PPAStopCpuEvent * PPAStopCpuEvent;
    extern FUNC_PPAStartRSEvent * PPAStartRSEvent;
    extern FUNC_PPAStopRSEvent * PPAStopRSEvent;
    extern FUNC_PPAStartSubSession * PPAStartSubSession;
    extern FUNC_PPAStopSubSession * PPAStopSubSession;
    extern FUNC_PPAIsEventEnable * PPAIsEventEnable;
    extern FUNC_PPARegisterCpuEvent * PPARegisterCpuEvent;
    extern FUNC_PPARegisterGrpName *	 PPARegisterGrpName;
    extern FUNC_PPARegisterSubSession* PPARegisterSubSession;
    extern FUNC_PPATIDCpuEvent * PPATIDCpuEvent;
    extern FUNC_PPADebugCpuEvent * PPADebugCpuEvent;
    extern FUNC_PPARegisterCpuEventExGrpID * PPARegisterCpuEventExGrpID;
    extern FUNC_PPASetGrpCpuEventEnDis * PPASetGrpCpuEventEnDis;
    extern FUNC_PPASetSingleCpuEventEnDis * PPASetSingleCpuEventEnDis;

    #endif  //< PPA_DISABLE


    /**
     * Macro for ppaStartCpuEvent
     * @param e CPU event id
     * @see ppaStartCpuEvent
     */
    #ifndef PPA_DISABLE
     #define PPAStartCpuEventFunc(e) \
        if(PPAStartCpuEvent !=NULL) \
            PPAStartCpuEvent(e)
    #else //< PPA_DISABLE
        #define PPAStartCpuEventFunc(e)
    #endif //< PPA_DISABLE

    /**
     * Macro for ppaStopCpuEvent
     * @param e CPU event id
     * @see ppaStopCpuEvent
     */
    #ifndef PPA_DISABLE
     #define PPAStopCpuEventFunc(e) \
        if(PPAStopCpuEvent != NULL) \
            PPAStopCpuEvent(e)
    #else //< PPA_DISABLE
        #define PPAStopCpuEventFunc(e)
    #endif //< PPA_DISABLE

    /**
     * Macro for ppaStartRSEvent
     * @param e RS event id
     * @see ppaStartRSEvent
     */
    #ifndef PPA_DISABLE
     #define PPAStartRSEventFunc(e) \
        if(PPAStartRSEvent != NULL) \
            PPAStartRSEvent(e)
    #else //< PPA_DISABLE
        #define PPAStartRSEventFunc(e)
    #endif //< PPA_DISABLE
    /**
     * Macro for ppaStopRSEvent
     * @param e RS event id
     * @see ppaStopRSEvent
     */
    #ifndef PPA_DISABLE
     #define PPAStopRSEventFunc(e) \
        if(PPAStopRSEvent != NULL) \
            PPAStopRSEvent(e)
    #else //< PPA_DISABLE
        #define PPAStopRSEventFunc(e)
    #endif //< PPA_DISABLE

    /**
     * Macro for ppaStartSubSession
     * @param e session id
     * @see ppaStartSubSession
     */
    #ifndef PPA_DISABLE
     #define PPAStartSubSessionFunc(e) \
        if(PPAStartSubSession != NULL) \
            PPAStartSubSession(e)
    #else //< PPA_DISABLE
        #define PPAStartSubSessionFunc(e)
    #endif //< PPA_DISABLE
    /**
     * Macro for ppaStopSubSession
     * @param e session id
     * @see ppaStopSubSession
     */
    #ifndef PPA_DISABLE
     #define PPAStopSubSessionFunc(e) \
        if(PPAStopSubSession != NULL) \
            PPAStopSubSession(e)
    #else //< PPA_DISABLE
        #define PPAStopSubSessionFunc(e)
    #endif //< PPA_DISABLE

    /**
     * Macro for ppaIsEventEnabled
     * @param e CPU event id
     * @see ppaIsEventEnabled
     */
    #ifdef PPA_DISABLE
     #define PPAIsEventEnabledFunc(e) dumbrt()
    #else
     #define PPAIsEventEnabledFunc(e) \
         (PPAIsEventEnable != NULL ? PPAIsEventEnable( (e) ) : false)
    #endif
     

    /**
    * Macro for ppaRegisterCpuEvent
    * @param s CPU event name
    * @see ppaRegisterCpuEvent
    */
    #ifdef PPA_DISABLE
     #define PPARegisterCpuEventFunc(s) dumbrt()
    #else
     #define PPARegisterCpuEventFunc(s) \
         (PPARegisterCpuEvent != NULL ? PPARegisterCpuEvent( (s) ) : 0xFFFF)
    #endif
              
    /**
    * Macro for ppaRegisterSubSession
    * @param s Sub-Session name
    * @see ppaRegisterSubSession
    */
    #ifdef PPA_DISABLE
     #define PPARegisterSubSessionFunc(s) dumbrt()
    #else
     #define PPARegisterSubSessionFunc(s) \
         (PPARegisterSubSession != NULL ? PPARegisterSubSession( (s) ) : 0xFFFF)
    #endif

    /**
    * Macro for ppaRegisterGrpName
    * @param s group name
    * @see ppaRegisterGrpName
    */
    #ifdef PPA_DISABLE
     #define PPARegisterGrpNameFunc(s) dumbrt()
    #else
     #define PPARegisterGrpNameFunc(s) \
         (PPARegisterGrpName != NULL ? PPARegisterGrpName( (s) ) : 0xFF)
    #endif
               
    /**
     * Macro for ppaTIDCpuEvent
     * @param e CPU event id
     * @param data additional data
     * @see ppaTIDCpuEvent
     */
    #ifndef PPA_DISABLE
     #define PPATIDCpuEventFunc(e, data) \
         if(PPATIDCpuEvent != NULL) \
             PPATIDCpuEvent(e, data)
    #else //< PPA_DISABLE
     #define PPATIDCpuEventFunc(e, data)
    #endif //< PPA_DISABLE

    /**
     * Macro for ppaStartCpuEvent
     * @param e CPU event id
     * @param data0 additional data
     * @param data1 additional data
     * @see ppaDebugCpuEvent
     */
    #ifndef PPA_DISABLE
     #define PPADebugCpuEventFunc(e, data0,data1) \
         if(PPADebugCpuEvent != NULL) \
             PPADebugCpuEvent(e, data0, data1)
    #else //< PPA_DISABLE
     #define #define PPADebugCpuEventFunc(e, data0, data1) 
    #endif //< PPA_DISABLE

    /**
     * Macro for ppaRegisterCpuEventExGrpID
     * @param s CPU event name
     * @param e group id
     * @see ppaRegisterCpuEventExGrpID
     */
    #ifdef PPA_DISABLE
     #define PPARegisterCpuEventExGrpIDFunc(s, e) dumbrt()
    #else
     #define PPARegisterCpuEventExGrpIDFunc(s, e) \
         (PPARegisterCpuEventExGrpID != NULL ? PPARegisterCpuEventExGrpID((s), (e)) : 0xFFFF)
    #endif


    /**
     * Macro for ppaSetGrpCpuEventsEnDis
     * @param en_dis enable or disable, TRUE or 1 is enable, FALSE or 0 is disable
     * @param e group id
     * @see ppaSetGrpCpuEventsEnDis
     */
    #ifdef PPA_DISABLE
     #define PPASetGrpCpuEventsEnDisFunc(en_dis, e) dumbrt()
    #else
     #define PPASetGrpCpuEventsEnDisFunc(en_dis, e) \
         (PPASetGrpCpuEventEnDis != NULL ? PPASetGrpCpuEventEnDis((en_dis), (e)) : 0)
    #endif
    /**
     * Macro for ppaSetSingleCpuEventEnDis
     * @param en_dis enable or disable, TRUE or 1 is enable, FALSE or 0 is disable
     * @param e CPU event id
     * @see ppaSetSingleCpuEventEnDis
     */
    #ifdef PPA_DISABLE
     #define PPASetSingleCpuEventEnDisFunc(en_dis, e) dumbrt()
    #else
     #define PPASetSingleCpuEventEnDisFunc(en_dis, e) \
         (PPASetSingleCpuEventEnDis != NULL ? PPASetSingleCpuEventEnDis((en_dis), (e)) : false)
    #endif
     

    #ifndef PPA_DISABLE
     /**
      * Initialize PPA
      * @return a pointer to the PpaBase class
      */
    void initializePPA();

    /**
     *	Release PPA
     *  @return void 
     */
     void releasePPA();


    //< check if the ppa has been installed
    bool IsPPAInstalled(void);

    //< get the opencl library path when you used PPA to capture the opencl
    //< ocl_path : the real opencl library path you want call
    const char* GetCLLibraryPath(const char* ocl_path);
    #endif //< PPA_DISABLE

    /**
     * PPA_INIT  macro to initialize PPA
     */
     #define PPA_INIT()  PPA_CLEANUP(initializePPA())
    /**
     * PPA_END macro to shut down PPA
     */
     #define PPA_END()  PPA_CLEANUP(releasePPA()) 

    //< macro to check if the ppa has been installed
    #ifndef PPA_DISABLE
     #define IS_PPA_INSTALLED()	IsPPAInstalled()
    #else //< PPA_DISABLE
     #define IS_PPA_INSTALLED()	false
    #endif //< PPA_DISABLE

    //< macro to get the opencl library path
    #ifndef PPA_DISABLE
     #define PPA_OCL_LIB_PATH(path)  GetCLLibraryPath(path)
    #else //< PPA_DISABLE
     #define PPA_OCL_LIB_PATH(path)  path
    #endif //< PPA_DISABLE

#ifdef __cplusplus
}
#endif //< __cplusplus


#define PPADispatchStartEvent PPAStartCpuEventFunc
#define PPADispatchEndEvent PPAStopCpuEventFunc

#endif //< _PPA_H_
