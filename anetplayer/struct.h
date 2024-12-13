#ifndef STRUCT_H
#define STRUCT_H
#include <QMainWindow>

typedef struct
{
    uint8_t rows;
    uint8_t columns;
    QString fps;
    QString slectedInterfaceName;
    QString ip;
    uint16_t port;
    bool tcOut;
} settings_t;

typedef struct
{
    uint8_t hh;
    uint8_t mm;
    uint8_t ss;
    uint8_t ff;
    uint8_t fps;
} timecode_t;


#endif // STRUCT_H
