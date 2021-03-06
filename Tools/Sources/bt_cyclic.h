#ifndef BT_CYCLIC_H
#define BT_CYCLIC_H

#include <QObject>
#include <QFile>
#include <QTimer>
#include <QVector>

#include "config.h"

class BtCyclic : public QObject
{
    Q_OBJECT
public:
    explicit BtCyclic(int size, QObject *parent = nullptr);

    int  read(int16_t *data, int size, int *fake=NULL);
    int  read(float   *data, int size, int *fake=NULL);

    void write(int16_t *data, int size);
    void write(QVector<int16_t> *data);

    void clear();

    void constWrite(int16_t data, int size);
    int  rewind(int count);
    int  getDataSize(int *fake=NULL); // Get Data size that are in Buf
    int  getFreeSize(); // Get Availible Byte to Write in Buf

private:
    int16_t *buffer;

    int read_p;
    int write_p;
    int buff_size;
};

#endif // BT_CYCLIC_H
