#include "kd_online_decoder.h"

#ifndef BT_LAT_ONLINE

#include "base/timer.h"
#include "fstext/fstext-utils.h"
#include "hmm/hmm-utils.h"
#include "QDebug"

using namespace kaldi;

KdOnlineDecoder::KdOnlineDecoder(fst::Fst<fst::StdArc> &fst,
                                 KdOnlineDecoderOpts &opts, QVector<int> sil_phones,
                                 const kaldi::TransitionModel &trans_model):
    KdFasterDecoder(fst, opts), opts_(opts),
    trans_model_(trans_model),
    max_beam_(opts.beam), effective_beam_(opts.beam),
    state_(KD_EndFeats), frame_(0), utt_frames_(0)
{
    silence_set = sil_phones;
}

void KdOnlineDecoder::ResetDecoder(bool full)
{
    ClearToks(toks_.Clear());

    StateId start_state = fst_.Start();
    KALDI_ASSERT(start_state != fst::kNoStateId);
    Arc dummy_arc(0, 0, Weight::One(), start_state);
    KdFToken *dummy_token = new KdFToken(dummy_arc, NULL);
    toks_.Insert(start_state, dummy_token);
    prev_immortal_tok_ = immortal_tok_ = dummy_token;
    utt_frames_ = 0;

    if( full )
    {
        frame_ = 0;
    }
}


void KdOnlineDecoder::MakeLattice(KdFToken *start, KdFToken *end,
                                   fst::MutableFst<LatticeArc> *out_fst)
{
    out_fst->DeleteStates();
    if (start == NULL)
    {
        return;
    }

    bool is_final = true;
    double this_cost = start->cost + fst_.Final(start->arc_.nextstate).Value();

    if( this_cost==KD_INFINITY )
    {
        is_final = false;
    }

    std::vector<LatticeArc> arcs_reverse;  // arcs in reverse order.

    for( KdFToken *tok=start ; tok!=end ; tok=tok->prev_ )
    {
        float last_cost = 0;
        if( tok->prev_ )
        {
            last_cost = tok->prev_->cost;
        }

        float tot_cost   = tok->cost - last_cost;
        float graph_cost = tok->arc_.weight.Value();
        float ac_cost    = tot_cost - graph_cost;

        LatticeWeight arc_weight(graph_cost, ac_cost);
        LatticeArc l_arc(tok->arc_.ilabel, tok->arc_.olabel,
                         arc_weight, tok->arc_.nextstate);

        arcs_reverse.push_back(l_arc);
    }

    if(arcs_reverse.back().nextstate == fst_.Start())
    {
        arcs_reverse.pop_back();  // that was a "fake" token... gives no info.
    }

    StateId cur_state = out_fst->AddState();
    out_fst->SetStart(cur_state);
    for( int i=arcs_reverse.size()-1; i>=0 ; i-- )
    {
        LatticeArc arc = arcs_reverse[i];
        arc.nextstate = out_fst->AddState();
        out_fst->AddArc(cur_state, arc);
        cur_state = arc.nextstate;
    }

    if (is_final)
    {
        Weight final_weight = fst_.Final(start->arc_.nextstate);
        out_fst->SetFinal(cur_state, LatticeWeight(final_weight.Value(), 0.0));
    }
    else
    {
        out_fst->SetFinal(cur_state, LatticeWeight::One());
    }

    RemoveEpsLocal(out_fst);
}

void KdOnlineDecoder::UpdateImmortalToken()
{
    QVector<KdFToken*> emitting;
    for( const Elem *e=toks_.GetList() ; e != NULL; e = e->tail )
    {
        KdFToken* tok = e->val;
        while( tok!=NULL && tok->arc_.ilabel==0 ) //deal with non-emitting ones ...
        {
            tok = tok->prev_;
        }
        if ( tok==NULL )
        {
            continue;
        }
        if( emitting.contains(tok) )
        {
            continue;
        }
        emitting.push_back(tok);
    }

    QVector<KdFToken*> p_emitting;

    while( emitting.size()>1 )
    {
        p_emitting.clear();
        for( int i=0 ; i<emitting.size() ; i++ )
        {
            KdFToken* p_token = emitting[i]->prev_;
            while ((p_token!=NULL) && (p_token->arc_.ilabel == 0)) //deal with non-emitting ones
            {
                p_token = p_token->prev_;
            }
            if (p_token == NULL)
            {
                continue;
            }

            if( p_emitting.contains(p_token) )
            {
                continue;
            }
            p_emitting.push_back(p_token);
        }
        emitting = p_emitting;
    }

    if( emitting.size()==1 )
    {
        prev_immortal_tok_ = immortal_tok_;
        immortal_tok_ = emitting[0];
        return;
    }
}

int KdOnlineDecoder::PartialTraceback(fst::MutableFst<LatticeArc> *out_fst)
{
    UpdateImmortalToken();
    if( immortal_tok_==prev_immortal_tok_ )
    {
        return 0; //no new word
    }

    MakeLattice(immortal_tok_, prev_immortal_tok_, out_fst);
    return 1;
}

int KdOnlineDecoder::FinishTraceBack(fst::MutableFst<LatticeArc> *out_fst)
{
    KdFToken *best_tok = NULL;

    if( ReachedFinal() )
    {
        best_tok = getBestTok();
    }
    else //this should not happen;
    {
        qDebug() << "FinishTraceBack: false on ReachedFinal";
        return -1;
    }

    MakeLattice(best_tok, immortal_tok_, out_fst);
    return 0;
}

KdFToken *KdOnlineDecoder::getBestTok()
{
    KdFToken *best_tok = NULL;
    for( const Elem *e=toks_.GetList() ; e != NULL ; e=e->tail )
    {
        if( best_tok==NULL || *best_tok<*(e->val) )
        {
            best_tok = e->val;
        }
    }

    return best_tok;
}

double KdOnlineDecoder::updateBeam(double tstart)
{
    // adjust the beam if needed
    Timer timer;
    float tend = timer.Elapsed();
    float elapsed = (tend - tstart) * 1000;
    // warning: hardcoded 10ms frames assumption!
    float factor = elapsed / (opts_.rt_max * opts_.update_interval * 10);
    float min_factor = (opts_.rt_min / opts_.rt_max);

    if( factor>1 || factor<min_factor )
    {
        float update_factor = 0;

        if( factor>1 )
        {
            update_factor = -std::min(opts_.beam_update * factor, opts_.max_beam_update);
        }
        else
        {
            update_factor = std::min(opts_.beam_update / factor, opts_.max_beam_update);
        }

        effective_beam_ += effective_beam_ * update_factor;
        effective_beam_ = std::min(effective_beam_, max_beam_);
    }

    return tend;
}

// Used only in EndOfUtterance
void KdOnlineDecoder::TracebackNFrames(int32 nframes,
                                   fst::MutableFst<LatticeArc> *out_fst)
{
    KdFToken *best_tok = getBestTok();
    if( best_tok==NULL )
    {
        out_fst->DeleteStates();
        return;
    }

    bool is_final = false;
    double this_cost = best_tok->cost +
            fst_.Final(best_tok->arc_.nextstate).Value();

    if( this_cost!=KD_INFINITY )
    {
        is_final = true;
    }

    std::vector<LatticeArc> arcs_reverse;  // arcs in reverse order.

    KdFToken *tok = best_tok;
    while( nframes>0 )
    {
        if( tok==NULL )
        {
            break;
        }

        if( tok->arc_.ilabel!=0 ) // count only the non-epsilon arcs
        {
            nframes--;
        }

        float tot_cost = tok->cost;

        if( tok->prev_ )
        {
            tot_cost -= tok->prev_->cost;
        }

        float graph_cost = tok->arc_.weight.Value();
        float ac_cost = tot_cost - graph_cost;
        LatticeWeight lat_w(graph_cost, ac_cost);

        LatticeArc l_arc(tok->arc_.ilabel, tok->arc_.olabel,
                        lat_w, tok->arc_.nextstate);
        arcs_reverse.push_back(l_arc);

        tok = tok->prev_;
    }

    if( arcs_reverse.back().nextstate==fst_.Start() )
    {
        arcs_reverse.pop_back();  // remove first "fake" token
    }

    StateId cur_state = out_fst->AddState();
    out_fst->SetStart(cur_state);

    for( int i = arcs_reverse.size()-1 ; i>=0; i-- )
    {
        LatticeArc arc = arcs_reverse[i];
        arc.nextstate = out_fst->AddState();
        out_fst->AddArc(cur_state, arc);
        cur_state = arc.nextstate;
    }
    if( is_final )
    {
        Weight final_weight = fst_.Final(best_tok->arc_.nextstate);
        out_fst->SetFinal(cur_state, LatticeWeight(final_weight.Value(), 0.0));
    }
    else
    {
        out_fst->SetFinal(cur_state, LatticeWeight::One());
    }
    RemoveEpsLocal(out_fst);
}

// Detect End of Utterance
bool KdOnlineDecoder::HaveSilence()
{
    fst::VectorFst<LatticeArc> trace;
    int32 sil_frm = opts_.inter_utt_sil / (1 + utt_frames_ / opts_.max_utt_len_); //50
    TracebackNFrames(sil_frm, &trace);
    std::vector<int32> isymbols;
    std::vector<int32> *osymbols = NULL;
    LatticeArc::Weight *l_weight = NULL;
    fst::GetLinearSymbolSequence(trace, &isymbols, osymbols, l_weight);
    std::vector<std::vector<int32> > split;
    SplitToPhones(trans_model_, isymbols, &split);
    for( int i=0 ; i<split.size() ; i++ )
    {
        int32 tid = split[i][0];
        int32 phone = trans_model_.TransitionIdToPhone(tid);
        if( !silence_set.contains(phone) )
        {
            return false;
        }
    }
    return true;
}

KdDecodeState KdOnlineDecoder::Decode(DecodableInterface *decodable)
{
    if( state_==KD_EndFeats || state_==KD_EndUtt ) // new utterance
    {
        ResetDecoder(state_ == KD_EndFeats);
    }

    ProcessNonemitting(std::numeric_limits<float>::max());
    int batch_frame;
    Timer timer;
    double64 tstart = timer.Elapsed();
    for ( batch_frame=0 ; batch_frame<opts_.batch_size ; batch_frame++ )
    {
        if( batch_frame>0 && (batch_frame%opts_.update_interval)==0 )
        {
            tstart = updateBeam(tstart);
        }

        if( decodable->IsLastFrame(frame_ - 1) )
        {
            break;
        }

        float weight_cutoff = ProcessEmitting(decodable);
        ProcessNonemitting(weight_cutoff);
        ++frame_, ++utt_frames_;
    }

    if( decodable->IsLastFrame(frame_-1) )
    {
        qDebug() << "-----------";//THIS LINE NEVER RUN!
        state_ = KdDecodeState::KD_EndFeats;
    }
    else
    {
        if( HaveSilence() )
        {
            state_ = KdDecodeState::KD_EndUtt;
        }
        else
        {
            state_ = KdDecodeState::KD_EndBatch;
        }
    }

    return state_;
}

#endif // BT_LAT_ONLINE
