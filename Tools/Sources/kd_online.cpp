#include "kd_online.h"

using namespace kaldi;
using namespace fst;

AmDiagGmm        *am_gmm;
TransitionModel  *trans_model;
KdOnlineLDecoder *o_decoder;
KdOnline2Model   *o2_model; //gaussain online 2 model

KdOnline::KdOnline(QObject *parent): QObject(parent)
{
    cy_buf = new BtCyclic(BT_REC_RATE*BT_BUF_SIZE);

    ab_src = new BtRecorder(cy_buf);
    status.word_count = 0;

    std::string model_rxfilename = BT_OAMDL_PATH;
    QVector<int> silence_phones = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    trans_model = new TransitionModel;
    am_gmm = new AmDiagGmm;

    bool rx_binary;
    Input ki(model_rxfilename, &rx_binary);
    trans_model->Read(ki.Stream(), rx_binary);
    am_gmm->Read(ki.Stream(), rx_binary);

    std::string online_alimdl = KAL_NATO_DIR"exp/tri1_online/final.oalimdl";
    o2_model = new KdOnline2Model(trans_model, am_gmm, online_alimdl);

    o_decoder = new KdOnlineLDecoder(silence_phones, *trans_model);
}

KdOnline::~KdOnline()
{
    delete am_gmm;
    delete trans_model;
    delete o2_model;
    delete o_decoder;
}

void KdOnline::init()
{
    startDecode();
}

void KdOnline::startDecode()
{
    float acoustic_scale = 0.05;

    KdOnline2Decodable decodable(ab_src, o2_model,
                             acoustic_scale);

    ab_src->startStream();

    o_decoder->InitDecoding(&decodable);
    KdCompactLattice out_fst;
    QVector<BtWord> result;

    while( 1 )
    {
        decodable.features->AcceptWaveform(cy_buf);
        o_decoder->Decode();
        result = o_decoder->getResult(&out_fst);
        bt_writeBarResult(result);

        if( result.size() )
        {
            execute(result);
        }

        if( o_decoder->status.state!=KD_STATE_NORMAL )
        {
            status.word_count = 0;
        }
    }
}

void KdOnline::execute(QVector<BtWord> result)
{
    QVector<QString> buf;
    QString dbg = "result ";

    for( int i=status.word_count ; i<result.size() ; i++ )
    {
        if( i==0 )
        {
            if( result[i].conf<0.75 &&
                result.size()==1 )
            {
                continue;
            }
            if( result[i].end<0.15 )
            {
                continue;
            }
        }
        if( result[i].is_final )
        {
            status.word_count = i+1;
            buf += result[i].word;
            dbg += result[i].word;
            dbg += "(";
            dbg += QString::number(result[i].conf);
            dbg += ")";
            dbg += " ";
        }
    }

    if( buf.empty() )
    {
        return;
    }

//    qDebug() << dbg;
    QString cmd = KAL_SI_DIR"main.sh \"";

    for( int i=0 ; i<buf.size() ; i++ )
    {
        cmd += buf[i];
        cmd += " ";
    }

    cmd += "\"";
    system(cmd.toStdString().c_str());
//    qDebug() << "exec" << cmd;
}
