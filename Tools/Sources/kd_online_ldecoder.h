#ifndef KD_ONLINE_LDECODER_H
#define KD_ONLINE_LDECODER_H

#include <QVector>
#include "bt_config.h"
#include "backend.h"

// This class provide a lattice online gmm decoder
// Kaldi only offer an online gmm decoder with no
// Lattice Support. Lattices Make it possible to
// get multi recognition output.

#include "util/stl-utils.h"
#include "kd_decoder.h"
#include "hmm/transition-model.h"
#include "lat/lattice-functions.h"
#include "kd_lattice.h"
#include "kd_lattice_functions.h"
#include "kd_mbr.h"

struct KdOnlineLDecoderOpts: public KdDecoderConfig
{
    int batch_size = 27;       // number of features decoded in one go
    int inter_utt_sil = 50;    // minimum silence (#frames) to trigger end of utterance
    int max_utt_len_  = 1500;  // if utt. is longer, we accept shorter silence as utt. separators
    int update_interval = 3;   // beam update period in # of frames
    float beam_update = 0.01;    // rate of adjustment of the beam
};

struct KdOnlineStatus
{
    int state = KD_STATE_NORMAL;
    uint max_frame = 0;
    uint min_frame = 0;
};

class KdOnlineLDecoder : public KdDecoder
{
public:
    KdOnlineLDecoder(kaldi::TransitionModel *trans_model);

    int Decode();

    void createStates(KdLattice *ofst);
    void RawLattice(KdLattice *ofst);
    void MakeLattice(KdCompactLattice *ofst);

    QVector<BtWord> getResult(KdCompactLattice *out_fst);

    void HaveSilence();
    void CalcFinal();
    KdOnlineStatus status;

    int wav_id = 0; //used for cyclyic test mode

private:
    void checkReset();
    bool GetiSymbol(KdLattice *fst, std::vector<int> *isymbols_out);
    void MakeGraph(int frame);
    void makeNodes();
    void makeEdge();
    QVector<BtWord> result;
    KdMBR *mbr;

    // Returns a linear fst by tracing back the last N frames, beginning
    // from the best current token
    void TracebackNFrames(int nframes, KdLattice *out_fst);
    KdToken* getBestTok();

    // Searches for the last token, ancestor of all currently active tokens
    void UpdateImmortalToken();

    // cache FSTs
    int        last_cache_f; // last cache frame
    KdLattice  cache_fst1; //used for createStates
    KdLattice  cache_fst2; //used for addArcs
    QFile     *gd_file; // graph debug

    KdOnlineLDecoderOpts opts;
    kaldi::TransitionModel *t_model; // needed for trans-id -> phone conversion
    float effective_beam_; // the currently used beam
    clock_t start_t;
};

#endif // KD_ONLINE_LDECODER_H
