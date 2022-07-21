#ifndef CH_CHANNEL_W_H
#define CH_CHANNEL_W_H

#include <QObject>
#include <QThread>
#include <windows.h>
#include "backend.h"
#include "config.h"

#define CH_BACKSPACE_CODE 16777219
#define CH_ESCAPE_CODE    16777216
#define CH_F1_CODE        16777264
#define CH_KEY_MIN        ('0'-1)
#define CH_KEY_MAX        ('Z'+1)

#define CH_LEFT_CLICK   0
#define CH_NO_CLICK     1
#define CH_RIGHT_CLICK  2

#define CH_PIPE_PATH "\\\\.\\pipe\\com_binaee_chess"
// The buffer size specified should be small enough that your
// process will not run out of nonpaged pool, but large enough
// to accommodate typical requests.
#define BUFFER_SIZE (1024 * 8)

class ChChannelW : public QObject
{
    Q_OBJECT
public:
    ChChannelW(QObject *ui, QObject *parent = NULL);
    ~ChChannelW();

signals:
    void show(const QString &args);
    void side(const QString &args);

private:
    void listenPipe();
    void processCommand(QString cmd, QString arg);
    void processLine(QString line);

    HANDLE hPipe;
};



#endif // CH_CHANNEL_W_H
