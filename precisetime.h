#ifndef PRECISETIME_H
#define PRECISETIME_H

//#include <windows.h>
//#include <sysinfoapi.h>
#include <timezoneapi.h>

/*
 * Для замера времени выполнения функции предпочтительнее
 * перед выполнением нужного участка вызвать initPrecTime()
 * и по завершении getElapsedPrecTime.
 * Возвращенное значение метода getElapsedPrecTime
 * будет содержать прошедшее время между вызовами методов
 * initPrecTime и getElapsedPrecTime
 */


namespace nPrecTime
{
    void initPrecTime();
    void resetPrecTime();

    double getElapsedPrecTime_ms();
    double getElapsedPrecTime_mcs();

    typedef SYSTEMTIME sTime;

    sTime getCurrentSystemTime();
    sTime getCurrentLocalTime();

    LONGLONG getFrequency();

    enum TimerResolution
    {
        TimerResolution_0_500  = 5000,  // 0.5 ms
        TimerResolution_1_000  = 10000, // 1ms
        TimerResolution_1_250  = 12500, // 1.25ms
        TimerResolution_2_500  = 25000, // 2.5 ms
        TimerResolution_5_000  = 50000, // 5 ms
        TimerResolution_10_000 = 100000,// 10 ms
        TimerResolution_15_600 = 156000,// 15.6 ms
        TimerError             = 0      // error
    };

    TimerResolution setTimerResolution(TimerResolution tr); // возвращает текущее значение точности в случае успеха,
                                                            // иначе TimerError = 0
    TimerResolution resetTimerResolution();                 // возвращает значение по умолчанию (TimerResolution_15_600)

    // минимальное, максимальное и текущее значение системного таймера
    struct queryTimerResolution
    {
        queryTimerResolution(): minimum(0), maximum(0), current(0) {}
        float minimum;
        float maximum;
        float current;
    };
    queryTimerResolution getQueryTimerResolution();

    bool timeBeginPeriod(TimerResolution tr);
    bool timeEndPeriod(TimerResolution tr);
}

#define START_TIMING() \
    nPrecTime::resetPrecTime();\
    nPrecTime::initPrecTime()

#define STOP_TIMING(name) \
    do{\
    double mks = nPrecTime::getElapsedPrecTime_mcs();\
    qDebug() << "stop timing for "#name" (mks): " << mks;\
    }while(0)

#endif // PRECISETIME_H
