#ifndef BT_RECORDER_H
#define BT_RECORDER_H

#include <QObject>
#include <QTimer>
#include <QDebug>
#include <QThread>

#include "bt_cyclic.h"
#include <portaudio.h>

class BtRecorder: public QObject
{
    Q_OBJECT
public:
    explicit BtRecorder(BtCyclic *buffer, QObject *parent = nullptr);

    // The real PortAudio callback delegates to this one
    int Callback(int16_t *data, int size);

    ~BtRecorder();
    BtCyclic *cy_buf;

public slots:
    void startStream();

private:
    PaStream *pa_stream;
    bool pa_started_; // becomes "true" after "pa_stream_" is started
    uint report_interval_; // interval (in Read() calls) to report PA rb overflows
    uint nread_calls_; // number of Read() calls so far
    uint noverflows_; // number of the ringbuf overflows since the last report
    uint samples_lost_; // samples lost, due to PA ring buffer overflow
    int    read_pf; //fake pointer for only OnlineDecodable(multicore)
};

// The actual PortAudio callback - delegates to OnlinePaSource->Callback()
int PaCallback(const void *input, void *output,
               long unsigned frame_count,
               const PaStreamCallbackTimeInfo *time_info,
               PaStreamCallbackFlags status_flags,
               void *user_data);



#endif // BT_RECORDER_H
