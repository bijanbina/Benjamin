#ifndef KD_DECODABLE_H
#define KD_DECODABLE_H

#include "config.h"

#include "kd_model.h"
#include "kd_a_model.h"
#include "bt_feinput.h"
#include "kd_t_model.h"

#define MAX_FRAME_CNT 400
typedef struct KdPDF
{
    int pdf_id;
    int phone_id;
    float val;

    bool operator<(const KdPDF &b)
    {
        return val<b.val;
    }
} KdPDF;

// KdOnline2Decodable
class KdDecodable
{
public:
    KdDecodable(BtCyclic *buf, KdAModel *a_mdl,
                KdTransitionModel *t_mdl, BtState *state);
    ~KdDecodable();

    /// Returns the scaled log likelihood
    float LogLikelihood(uint frame, int index);
    uint NumFramesReady();

    int NumIndices();
    BtFeInput *features;


private:
    void CacheFeature(uint frame);

    KdAModel *ac_model;
    float ac_scale_;
    KdTransitionModel *trans_model;
    int feat_dim;  // dimensionality of the input features
    BtFrameBuf *feat_buf;
    int cur_frame_;
    std::vector<std::pair<int, float> > cache_;
};


#endif // KD_DECODABLE_H
