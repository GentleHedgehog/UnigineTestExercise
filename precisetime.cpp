#include <windef.h>
#include "precisetime.h"

#include <stdio.h>
#include <winbase.h>

#define MS_MLTPL  1e-3
#define MCS_MLTPL 1e-6

#define TO_LIT(f) #f

#define CHECK_DLL(dll) (!dll ? printf("%s not load\n", TO_LIT(dll)), 0 : (int)dll)
#define CHECK_FUNC_CAPTURE(func) (!func ? printf("%s not capture\n", TO_LIT(func)), 0 : (int)func)
#define NT_DLL CHECK_DLL(_nt_dll_)

static LARGE_INTEGER _startTick_ ;
static LARGE_INTEGER _freq_      ;
static LARGE_INTEGER _tick_      ;
static double        _diff_      ;

/*=========== NtDll.dll ===========*/
static HINSTANCE     _nt_dll_ =  LoadLibrary(L"C:/Windows/System32/ntdll.dll");
typedef void __stdcall (*prototypeSetTimerResolution)(unsigned int DesiredResolution, bool SetTimer, unsigned int &CurrentResolution);
typedef void __stdcall (*prototypeQueryTimerResolution)(unsigned long &MinRes, unsigned long &MaxRes, unsigned long &CurRes);

static prototypeQueryTimerResolution  NtQueryTimerResolution = (prototypeQueryTimerResolution)GetProcAddress(_nt_dll_, "NtQueryTimerResolution");
static prototypeSetTimerResolution    NtSetTimerResolution   = (prototypeSetTimerResolution)GetProcAddress(_nt_dll_,   "NtSetTimerResolution");
/*=================================*/

/*=========== WinMM.dll ===========*/
static HINSTANCE  _winmm_dll_ =  LoadLibrary(L"C:/Windows/System32/winmm.dll");

typedef unsigned int __stdcall (*prototypeTimeBeginPeriod) (unsigned int Period);
typedef unsigned int __stdcall (*prototypeTimeEndPeriod)   (unsigned int period);

static prototypeTimeBeginPeriod _timeBeginPeriod   = (prototypeTimeBeginPeriod)GetProcAddress(_winmm_dll_, "timeBeginPeriod");
static prototypeTimeEndPeriod   _timeEndPeriod     = (prototypeTimeEndPeriod)  GetProcAddress(_winmm_dll_, "timeEndPeriod");
/*==================================*/


void nPrecTime::initPrecTime()
{
    QueryPerformanceFrequency(&_freq_);
    QueryPerformanceCounter(&_startTick_);
}

void nPrecTime::resetPrecTime()
{
    _startTick_.QuadPart = 0;
    _freq_.QuadPart      = 0;
    _tick_.QuadPart      = 0;
    _diff_               = 0;
}

double nPrecTime::getElapsedPrecTime_ms()
{
    QueryPerformanceCounter(&_tick_);
    QueryPerformanceFrequency(&_freq_);

    _diff_ = (double)(_tick_.QuadPart - _startTick_.QuadPart)/(double)(_freq_.QuadPart);
    return (_diff_/MS_MLTPL);
}

double nPrecTime::getElapsedPrecTime_mcs()
{
    QueryPerformanceCounter(&_tick_);
    QueryPerformanceFrequency(&_freq_);

    _diff_ = (double)(_tick_.QuadPart - _startTick_.QuadPart)/(double)(_freq_.QuadPart);
    return (_diff_/MCS_MLTPL);
}

LONGLONG nPrecTime::getFrequency()
{
    QueryPerformanceFrequency(&_freq_);
    return _freq_.QuadPart;
}

nPrecTime::sTime nPrecTime::getCurrentSystemTime()
{
    sTime st;
    GetSystemTime(&st);
    return st;
}

nPrecTime::sTime nPrecTime::getCurrentLocalTime()
{
    sTime st;
    GetLocalTime(&st);
    return st;
}

nPrecTime::TimerResolution nPrecTime::setTimerResolution(TimerResolution _tr)
{
    if (!NT_DLL) return TimerError;
    if (!CHECK_FUNC_CAPTURE(NtSetTimerResolution))
        return TimerError;

    unsigned int currentResolution;
    NtSetTimerResolution((unsigned int)_tr, true, currentResolution);
    return (TimerResolution)currentResolution;
}

nPrecTime::TimerResolution nPrecTime::resetTimerResolution()
{
    if (!NT_DLL) return TimerError;
    if (!CHECK_FUNC_CAPTURE(NtSetTimerResolution))
        return TimerError;

    unsigned int currentResolution;
    NtSetTimerResolution( (unsigned int)TimerResolution_15_600 , false, currentResolution);
    return (TimerResolution)currentResolution;
}

nPrecTime::queryTimerResolution nPrecTime::getQueryTimerResolution()
{
    queryTimerResolution qtr;
    unsigned long min = 0, max = 0, cur = 0;
    if ( NT_DLL && CHECK_FUNC_CAPTURE(NtQueryTimerResolution) )
    {
        NtQueryTimerResolution(min, max, cur);
        qtr.minimum = min / 10000.0;
        qtr.maximum = max / 10000.0;
        qtr.current = cur / 10000.0;
    }
    return qtr;
}

bool nPrecTime::timeBeginPeriod(TimerResolution tr)
{

    if (!CHECK_DLL(_winmm_dll_)) return 0;
    if (!CHECK_FUNC_CAPTURE(_timeBeginPeriod))
        return 0;

    unsigned int currentResolution;
    switch (tr)
    {
        case TimerResolution_0_500: currentResolution = 1;  break;
        case TimerResolution_1_000: currentResolution = 1;  break;
        case TimerResolution_1_250: currentResolution = 2;  break;
        case TimerResolution_2_500: currentResolution = 3;  break;
        case TimerResolution_5_000: currentResolution = 5;  break;
        case TimerResolution_10_000:currentResolution = 10; break;
        case TimerResolution_15_600:currentResolution = 16; break;
        default: currentResolution = 16; break;
    }

    _timeBeginPeriod(currentResolution);
    return 1;
}


bool nPrecTime::timeEndPeriod(TimerResolution tr)
{
    if (!CHECK_DLL(_winmm_dll_)) return 0;
    if (!CHECK_FUNC_CAPTURE(_timeEndPeriod))
        return 0;

    unsigned int currentResolution;
    switch (tr)
    {
        case TimerResolution_0_500: currentResolution = 1;  break;
        case TimerResolution_1_000: currentResolution = 1;  break;
        case TimerResolution_1_250: currentResolution = 2;  break;
        case TimerResolution_2_500: currentResolution = 3;  break;
        case TimerResolution_5_000: currentResolution = 5;  break;
        case TimerResolution_10_000:currentResolution = 10; break;
        case TimerResolution_15_600:currentResolution = 16; break;
        default: currentResolution = 16; break;
    }

    _timeEndPeriod(currentResolution);
    return 1;

}
