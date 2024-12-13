#include "tcconverter.h"

TCconverter::TCconverter() {}


timecode_t TCconverter::frames2dftc(const qint64 &frames)
{
    constexpr double framerate = 29.97;
    constexpr int dropFrames = static_cast<int>(framerate * 0.066666 + 0.5); // Round to nearlist int
    constexpr int framesPerHour = static_cast<int>(framerate * 60 * 60 + 0.5);
    constexpr int framesPer24Hours = framesPerHour * 24;
    constexpr int framesPer10Minutes = static_cast<int>(framerate * 600 + 0.5);
    constexpr int framesPerMinute = static_cast<int>(framerate * 60 + 0.5) - dropFrames; // Количество кадров в минуте с учётом дроп-фреймов

    qint64 frameNumber = frames;

    // Adjust for negative frame numbers by adding a full 24-hour frame count
    frameNumber = (frameNumber < 0) ? frameNumber + framesPer24Hours : frameNumber;
    // If frame number exceeds 24 hours, wrap it around (timecode rollover)
    frameNumber %= framesPer24Hours;

    int full10MinBlocks = frameNumber / framesPer10Minutes;
    int remainingFrames = frameNumber % framesPer10Minutes;

    // Calc drop frames
    if (remainingFrames > dropFrames) {
        frameNumber += dropFrames * (9 * full10MinBlocks + (remainingFrames - dropFrames) / framesPerMinute);
    } else {
        frameNumber += dropFrames * 9 * full10MinBlocks;
    }

    timecode_t tc;
    const int fps = static_cast<int>(framerate + 0.5); // Round to int
    tc.fps = fps;
    tc.ff = frameNumber % fps;
    frameNumber /= fps;
    tc.ss = frameNumber % 60;
    frameNumber /= 60;
    tc.mm = frameNumber % 60;
    tc.hh = frameNumber / 60;

    return tc;
}

timecode_t TCconverter::frames2ndftc(const qint64 &frames, const int &framerate)
{
    // Calculate the number of frames in an hour and in a 24-hour period
    const int framesPerHour = framerate * 60 * 60;
    const int framesPer24Hours = framesPerHour * 24;

    // Adjust for negative frame numbers by adding a full 24-hour frame count
    qint64 frameNumber = (frames < 0) ? frames + framesPer24Hours : frames;

    // If frame number exceeds 24 hours, wrap it around (timecode rollover)
    frameNumber %= framesPer24Hours;

    timecode_t tc;
    const int fps = framerate;
    tc.fps = fps;
    tc.ff = frameNumber % fps;
    frameNumber /= fps;
    tc.ss = frameNumber % 60;
    frameNumber /= 60;
    tc.mm = frameNumber % 60;
    tc.hh = frameNumber / 60;

    return tc;
}


timecode_t TCconverter::milliseconds2tc(const qint64 &ms, const double &framerate)
{
    timecode_t tc;
    quint64 frames = ms * framerate / 1000;
    int fps = static_cast<int>(framerate);
    if (fps == 29)
    {
        tc = frames2dftc(frames);
    }
    else
    {
        tc = frames2ndftc(frames, fps);
    }
    return tc;
}


qint64 TCconverter::dftc2frames(const timecode_t &tc)
{
    double framerate = 29.97;
    int dropFrames = static_cast<int>(framerate * 0.066666 + 0.5); //Number of drop frames is 6% of framerate rounded to nearest integer
    int timebase = round(framerate);
    int hourFrames = timebase * 60 * 60; //Number of frames per hour (non-drop)
    int minuteFrames = timebase * 60; //Number of frames per minute (non-drop)
    int totalMinutes = (60 * tc.hh) + tc.mm; //Total number of minuts
    int frameNumber = ((hourFrames * tc.hh) + (minuteFrames * tc.mm) + (timebase * tc.ss) + tc.ff) - (dropFrames * (totalMinutes - (totalMinutes / 10)));
    return frameNumber;
}

qint64 TCconverter::ndftc2frames(const timecode_t &tc)
{
    int timebase = static_cast<int>(tc.fps);
    int hourFrames = timebase * 60 * 60; //Number of frames per hour (non-drop)
    int minuteFrames = timebase * 60; //Number of frames per minute (non-drop)
    int frameNumber = (hourFrames * tc.hh) + (minuteFrames * tc.mm) + (timebase * tc.ss) + tc.ff;
    return frameNumber;
}

qint64 TCconverter::tc2milliseconds(const timecode_t &tc)
{
    quint64 totalMilliseconds = 0;
    int fps = static_cast<int>(tc.fps);
    if (fps == 29.97)
    {
        totalMilliseconds = (dftc2frames(tc) * 1000) / 29.97;
    }
    else
    {
        totalMilliseconds = ((tc.hh * 3600 + tc.mm * 60 + tc.ss) * 1000) + ((tc.ff * 1000) / tc.fps);
    }
    return totalMilliseconds;
}

QString TCconverter::tc2string(const timecode_t &tc)
{
    QString strTime = QString("%1:%2:%3:%4")
                                .arg(tc.hh, 2, 10, QChar('0'))
                                .arg(tc.mm, 2, 10, QChar('0'))
                                .arg(tc.ss, 2, 10, QChar('0'))
                                .arg(tc.ff, 2, 10, QChar('0'));
    return strTime;
}



