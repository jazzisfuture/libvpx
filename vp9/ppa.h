/*========================================================
Copyright (c) 2011 Multicoreware Inc. All rights reserved

======================================================= */

#ifndef _PPA_H_
#define _PPA_H_

#ifdef PPA_REGISTER_CPU_EVENT
#undef PPA_REGISTER_CPU_EVENT
#endif

#if __STDC_VERSION__  < 199901L
// #error "No C99 Support"
#endif

// #define PPA_CODELINK
#undef PPA_CLEANUP
#undef PPA_DISABLE
#ifdef PPA_DISABLE
# define UNDEF_PARAM(x)
# define PPA_CLEANUP(x) UNDEF_PARAM(#x)
#else /* defined(PPA_DISABLE) */

#ifdef PPA_CODELINK
#define P2STRING(x) #x
#define PSTRING(x) P2STRING(x)
#define PRAGMA_MES __pragma("message(__FILE__)")

#if defined(WINDOWS)||defined(_WIN32)
# define  PPA_APICLEANUP(x) PRAGMA_MES\
  x
# define PPA_CLEANUP(x) x
#else

#endif /* PPA_CODLELINK */

#else
# define  PPA_CLEANUP(x) x
# define  PPA_APICLEANUP(x) x
#endif /* !PPA_CODELINK */

#endif /* !defined(PPA_DISABLE) */



#if defined(WINDOWS) || defined(_WIN32)
#ifdef UNICODE
#define PPA_LIB_NAME L"ppa.dll"
#else
#define PPA_LIB_NAME "ppa.dll"
#endif /* UNICODE */
#else

#ifdef __i386__
#define PPA_LIB_NAME "libppa.so"
#elif defined(__amd64__) || (__ia64__)
#define PPA_LIB_NAME "libppa.so"
#else
#define PPA_LIB_NAME "/data/local/tmp/libppa.so"
#endif
#endif /* !WINDOWS */




/**
 * Register PPA event in a group
 * @param x event name
 * @param y group name
 */
#if !defined(PPA_DISABLE)
#define PPA_REGISTER_CPU_EVENT2GROUP(x, y) x,

#define PPA_REGISTER_CPU_EVENT(x) PPA_REGISTER_CPU_EVENT2GROUP(x, NoGroup)
enum PPACpuEventEnum {
  #include "./ppaCPUEvents.h"
  PPACpuGroupNums
};
#undef PPA_REGISTER_CPU_EVENT
#undef PPA_REGISTER_CPU_EVENT2GROUP
#endif


#ifdef __cplusplus
extern "C" {
#endif


typedef int16_t EventID;
typedef int8_t GrpID;
#ifndef __cplusplus
typedef enum {false = 0, true = 1} bool;
#endif

typedef void (FUNC_PPAInit)(char**, int);
typedef void  (FUNC_PPADel)();
typedef void  (FUNC_PPAStartCpuEvent)(EventID);
typedef void (FUNC_PPAStopCpuEvent)(EventID );
typedef void  (FUNC_PPAStartRSEvent)(EventID);
typedef void (FUNC_PPAStopRSEvent)(EventID );
typedef bool (FUNC_PPAIsEventEnable)(EventID );
typedef EventID (FUNC_PPARegisterCpuEvent)(const char* );
typedef GrpID (FUNC_PPARegisterGrpName)(const char* );
typedef void (FUNC_PPATIDCpuEvent)(EventID , unsigned int );
typedef void (FUNC_PPADebugCpuEvent)(EventID , unsigned int , unsigned int );
typedef EventID (FUNC_PPARegisterCpuEventExGrpID)(const char* , GrpID );
typedef int (FUNC_PPASetGrpCpuEventEnDis)(bool , GrpID );
typedef bool (FUNC_PPASetSingleCpuEventEnDis)(bool , EventID );

#ifndef PPA_DISABLE
// extern void*                 ppaDllHandle;
extern FUNC_PPAInit        *     PPAInitFunc;
extern FUNC_PPADel         *       PPADelFunc;
extern FUNC_PPAStartCpuEvent   *       PPAStartCpuEvent;
extern FUNC_PPAStopCpuEvent    *       PPAStopCpuEvent;
extern FUNC_PPAStartRSEvent  *       PPAStartRSEvent;
extern FUNC_PPAStopRSEvent     *       PPAStopRSEvent;
extern FUNC_PPAIsEventEnable   *     PPAIsEventEnable;
extern FUNC_PPARegisterCpuEvent  *       PPARegisterCpuEvent;
extern FUNC_PPARegisterGrpName   *       PPARegisterGrpName;
extern FUNC_PPATIDCpuEvent      *        PPATIDCpuEvent;
extern FUNC_PPADebugCpuEvent     *       PPADebugCpuEvent;
extern FUNC_PPARegisterCpuEventExGrpID * PPARegisterCpuEventExGrpID;
extern FUNC_PPASetGrpCpuEventEnDis     * PPASetGrpCpuEventEnDis;
extern FUNC_PPASetSingleCpuEventEnDis  * PPASetSingleCpuEventEnDis;
#endif


/**
 * Macro for ppaStartCpuEvent
 * @param e CPU event id
 * @see ppaStartCpuEvent
 */
#define PPAStartCpuEventFunc(e)       \
    if (PPAStartCpuEvent)                 \
      PPA_CLEANUP((PPAStartCpuEvent(e)))

/**
 * Macro for ppaStopCpuEvent
 * @param e CPU event id
 * @see ppaStopCpuEvent
 */
#define PPAStopCpuEventFunc(e)            \
    if (PPAStopCpuEvent)            \
      PPA_CLEANUP((PPAStopCpuEvent((e))))

/**
 * Macro for ppaStartRSEvent
 * @param e RS event id
 * @see ppaStartRSEvent
 */
#define PPAStartRSEventFunc(e)      \
    if (PPAStartRSEvent)                 \
      PPA_CLEANUP((PPAStartRSEvent(e)))
/**
 * Macro for ppaStopRSEvent
 * @param e RS event id
 * @see ppaStopRSEvent
 */
#define PPAStopRSEventFunc(e)             \
    if (PPAStopRSEvent)             \
      PPA_CLEANUP((PPAStopRSEvent((e))))

/**
 * Macro for ppaIsEventEnabled
 * @param e CPU event id
 * @see ppaIsEventEnabled
 */
#ifdef PPA_DISABLE
#define PPAIsEventEnabledFunc(e) dumbrt()
#else
#define PPAIsEventEnabledFunc(e)           \
    (PPAIsEventEnable ? PPAIsEventEnable((e)) : false)
#endif


/**
* Macro for ppaRegisterCpuEvent
* @param s CPU event name
* @see ppaRegisterCpuEvent
*/
#ifdef PPA_DISABLE
#define PPARegisterCpuEventFunc(s) dumbrt()
#else
#define PPARegisterCpuEventFunc(s)           \
    (PPARegisterCpuEvent ? PPARegisterCpuEvent((s)) : 0xFFFF)
#endif

/**
* Macro for ppaRegisterGrpName
* @param s group name
* @see ppaRegisterGrpName
*/
#ifdef PPA_DISABLE
#define PPARegisterGrpNameFunc(s) dumbrt()
#else
#define PPARegisterGrpNameFunc(s)             \
    (PPARegisterGrpName ? PPARegisterGrpName((s)) : 0xFF)
#endif

/**
 * Macro for ppaTIDCpuEvent
 * @param e CPU event id
 * @param data additional data
 * @see ppaTIDCpuEvent
 */
#define PPATIDCpuEventFunc(e, data)             \
    if (PPATIDCpuEvent)               \
      PPA_CLEANUP((PPATIDCpuEvent((e), (data))))

/**
 * Macro for ppaStartCpuEvent
 * @param e CPU event id
 * @param data0 additional data
 * @param data1 additional data
 * @see ppaDebugCpuEvent
 */
#define PPADebugCpuEventFunc(e, data0, data1)     \
    if (PPADebugCpuEvent)             \
      PPA_CLEANUP((PPADebugCpuEvent((e), (data0), (data1))))

/**
 * Macro for ppaRegisterCpuEventExGrpID
 * @param s CPU event name
 * @param e group id
 * @see ppaRegisterCpuEventExGrpID
 */
#ifdef PPA_DISABLE
#define PPARegisterCpuEventExGrpIDFunc(s, e) dumbrt()
#else
#define PPARegisterCpuEventExGrpIDFunc(s, e)     \
    (PPARegisterCpuEventExGrpID ? PPARegisterCpuEventExGrpID((s), (e)) : 0xFFFF)
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
#define PPASetGrpCpuEventsEnDisFunc(en_dis, e)   \
    (PPASetGrpCpuEventEnDis ? PPASetGrpCpuEventEnDis((en_dis), (e)) : 0)
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
    (PPASetSingleCpuEventEnDis ?   \
        PPASetSingleCpuEventEnDis((en_dis), (e)) : false)
#endif

 /**
  * Initialize PPA
  * @return a pointer to the PpaBase class
  */
void initializePPA();

/**
 *  Release PPA
 *  @return void
 */
void releasePPA();

/**
 * PPA_INIT  macro to initialize PPA
 */
#define PPA_INIT()   PPA_CLEANUP(initializePPA();)
 /**
  * PPA_END macro to shut down PPA
  */
#define PPA_END()  PPA_CLEANUP(releasePPA();)

#ifdef __cplusplus
}
#endif


#define PPADispatchStartEvent PPAStartCpuEventFunc
#define PPADispatchEndEvent PPAStopCpuEventFunc

#endif
