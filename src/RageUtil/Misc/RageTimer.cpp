/*
 * This can be used in two ways: as a timestamp or as a timer.
 *
 * As a timer,
 * RageTimer Timer;
 * for(;;) {
 *   printf( "Will be approximately: %f", Timer.PeekDeltaTime()) ;
 *   float fDeltaTime = Timer.GetDeltaTime();
 * }
 *
 * or as a timestamp:
 * void foo( RageTimer &timestamp ) {
 *     if( timestamp.IsZero() )
 *         printf( "The timestamp isn't set." );
 *     else
 *         printf( "The timestamp happened %f ago", timestamp.Ago() );
 *     timestamp.Touch();
 *     printf( "Near zero: %f", timestamp.Age() );
 * }
 */

#include "Etterna/Globals/global.h"

#include "RageTimer.h"
#include "Core/Misc/Timer.hpp"
#include "RageUtil/Utils/RageUtil.h"

#include "arch/ArchHooks/ArchHooks.h"

#include <chrono>

// let this henceforth be referred to as the difference between a second and a
// microsecond *kingly fanfare*
#define TIMESTAMP_RESOLUTION 1000000

const RageTimer RageZeroTimer(0);
static std::chrono::microseconds g_iStartTime =
  ArchHooks::GetChronoDurationSinceStart();

static uint64_t
GetTime()
{
	return ArchHooks::GetMicrosecondsSinceStart();
}

static std::chrono::microseconds
GetChronoTime()
{
	return ArchHooks::GetChronoDurationSinceStart();
}

void
RageTimer::Touch()
{
	this->c_dur = GetChronoTime();
}

float
RageTimer::Ago() const
{
	const RageTimer Now;
	return Now - *this;
}

float
RageTimer::GetDeltaTime()
{
	const RageTimer Now;
	const float diff = Difference(Now, *this);
	*this = Now;
	return diff;
}

RageTimer
RageTimer::operator+(float tm) const
{
	return Sum(*this, tm);
}

float
RageTimer::operator-(const RageTimer& rhs) const
{
	return Difference(*this, rhs);
}

bool
RageTimer::operator<(const RageTimer& rhs) const
{
	return c_dur < rhs.c_dur;
}

RageTimer
RageTimer::Sum(const RageTimer& lhs, float tm)
{
	const uint64_t usecs = static_cast<uint64_t>(tm * TIMESTAMP_RESOLUTION);
	const std::chrono::microseconds period(usecs);

	RageTimer ret(0); // Prevent unnecessarily checking the time
	ret.c_dur = period + lhs.c_dur;

	return ret;
}

float
RageTimer::Difference(const RageTimer& lhs, const RageTimer& rhs)
{
	const std::chrono::microseconds diff = lhs.c_dur - rhs.c_dur;

	return static_cast<float>(diff.count()) / TIMESTAMP_RESOLUTION;
}

#include "Etterna/Singletons/LuaManager.h"
LuaFunction(GetTimeSinceStart, static_cast<float>(Core::Timer::getDeltaSinceStart<std::chrono::milliseconds>().count()) / 1000.0f)
