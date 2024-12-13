#ifndef TCCONVERTER_H
#define TCCONVERTER_H

#include <QObject>
#include "struct.h"


class TCconverter
{

public:
    TCconverter();
    timecode_t frames2dftc(const qint64 &frames); // for 29.97 drop frames fps
    timecode_t frames2ndftc(const qint64 &frames, const int &framerate);  // for 24, 25, 30 no drop frames fps
    timecode_t milliseconds2tc(const qint64 &ms, const double &framerate);

    qint64 dftc2frames(const timecode_t &tc);
    qint64 ndftc2frames(const timecode_t &tc);
    qint64 tc2milliseconds(const timecode_t &tc);

    QString tc2string(const timecode_t &tc);

};

#endif // TCCONVERTER_H
