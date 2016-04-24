/*****************************************************************************
 *  gaston - We pay homage to Gaston, an Africa-born brown fur seal who
 *    escaped the Prague Zoo during the floods in 2002 and made a heroic
 *    journey for freedom of over 300km all the way to Dresden. There he
 *    was caught and subsequently died due to exhaustion and infection.
 *    Rest In Piece, brave soldier.
 *
 *  Copyright (c) 2016  Tomas Fiedor <ifiedortom@fit.vutbr.cz>
 *      Notable mentions:   Ondrej Lengal <ondra.lengal@gmail.com>
 *                              (author of VATA)
 *                          Petr Janku <ijanku@fit.vutbr.cz>
 *                              (MTBDD and automata optimizations)
 *
 *  File: Timer.h
 *  Description:
 *      Implementation of high-resolution timer
 *****************************************************************************/

#ifndef WSKS_TIMER_H
#define WSKS_TIMER_H

#include <chrono>

struct ChronoTimer {
private:
    std::uint64_t _startTime;
    std::uint64_t _elapsed;
    bool _running = false;

    const unsigned long hour = 360000;
    const unsigned long min  = 6000;
    const unsigned long sec  = 100;

public:
    ChronoTimer() : _startTime(0), _elapsed(0) {}

public:
    void Start() {
        _startTime = _takeTimeStamp();
        _running = true;
    }

    void Stop() {
        if(_running) {
            _elapsed += GetTimeElapsed();
            _running = false;
        }
    }

    std::uint64_t GetTimeElapsed() {
        return (_takeTimeStamp() - _startTime)*1e-7;
    }

    void PrintElapsed() {
        std::uint64_t t = _elapsed, hours, mins, secs;

        hours = t / hour;
        t -= hours * hour;

        mins = t / min;
        t -= mins * min;

        secs = t / sec;
        t -= secs * sec;

        printf("%02lu:%02lu:%02lu.%02lu\n", hours, mins, secs, t);
    }

protected:
    static std::uint64_t _takeTimeStamp() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }
};


#endif //WSKS_TIMER_H
