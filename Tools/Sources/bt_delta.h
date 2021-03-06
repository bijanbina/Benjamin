#ifndef BT_DELTA_H
#define BT_DELTA_H

#include "bt_cfb.h"

#define KD_DELTA_WINDOW 2 // should be  0<window<1000

class KdDelta
{
public:
    explicit KdDelta(BtCFB *feat);

    void calcCoeffs();
    void Process(uint frame, int max_frame);
    void applyCoeff(double *i_data, double coeff, double *o_data);
    void resetDelta(BtFrameBuf *buf);

    int  min_frame;

private:
    void initZeroCoeff(int i, int len);
    int  sumof2N2(int n);

    BtCFB *feature;
    QVector<float> coeffs[BT_DELTA_ORDER+1]; // coeff to compute smooth derivative
};

#endif // BT_DELTA_H
