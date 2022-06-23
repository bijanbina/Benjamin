#include "bt_enn.h"
#include <QDir>
#include <qmath.h>

BtEnn::BtEnn(QString dir_name, QObject *parent): QObject(parent)
{
    cy_buf = new BtCyclic(BT_REC_RATE*BT_BUF_SIZE);

    QDir p_dir(dir_name);
    QStringList fmt;
    fmt.append("*.wav");
    file_list = p_dir.entryList(fmt, QDir::Files);

    for( int i = 0 ; i<file_list.size() ; i++ )
    {
        file_list[i] = dir_name + file_list[i];
    }

    for( int i=0 ; i<BT_DELTA_ORDER+1 ; i++)
    {
        max_delta[i] = 0;
        min_delta[i] = 10000;
    }

    bt_mkDir(KAL_AU_DIR"enn/");

    QString cmd = "find " KAL_AU_DIR "enn/ -type f";
    wav_w = new BtWavWriter(cy_buf);
    exist_list = getStrCommand(cmd).split("\n");
}

BtEnn::~BtEnn()
{
    delete t_model;
    delete oa_model;
    delete o_decoder;
    delete cy_buf;
}

void BtEnn::init(QString dir)
{
    cat_dir = dir;
    std::string model_filename = BT_OAMDL_PATH;

    oa_model = new KdAModel;
    t_model = new KdTransitionModel;

    bool binary;
    kaldi::Input ki(model_filename, &binary);
    t_model->Read(ki.Stream());
    oa_model->Read(ki.Stream());

    o_decoder = new KdOnlineLDecoder(t_model);
    o_decoder->status.min_sil = 300;

    startDecode();
}

void BtEnn::startDecode()
{
    float acoustic_scale = 0.05;

    KdDecodable decodable(cy_buf, oa_model,
                          t_model, acoustic_scale);
    decodable.features->enableENN();

    o_decoder->InitDecoding(&decodable);
    KdCompactLattice out_fst;

//    int len = 50;
    int len = file_list.size();
//    qDebug() << file_list.size() << cat_dir;
    for( int i=0 ; i<len ; i++ )
    {
        if( checkExist(file_list[i]) )
        {
            continue;
        }
        qDebug() << "Info: wave [" << i << "/" << len << "].";
        openWave(file_list[i]);
        readWav(cy_buf);
        decodable.features->ComputeFeatures();
        o_decoder->Decode();
        last_r = o_decoder->getResult(&out_fst);
        if( !sanityCheck(file_list[i]) )
        {
            shit_counter++;
            qDebug() << "shit [" << shit_counter << "]"
                     << file_list[i] << last_r.size();
            bt_printResult(last_r);
            if( shit_counter<4 )
            {
                o_decoder->wav_id++;
                o_decoder->resetODecoder();
                delete decodable.features->cmvn;
                decodable.features->cmvn = new BtCMVN(decodable.features->o_features);
                decodable.features->delta->min_frame = o_decoder->status.min_frame;
                last_r.clear();
                continue;
            }
//            exit(1);
        }
        preProcess();
        saveFeature(file_list[i], decodable.features->o_features);
//        saveWave(file_list[i]);
        o_decoder->wav_id++;
        o_decoder->resetODecoder();
        delete decodable.features->cmvn;
        decodable.features->cmvn = new BtCMVN(decodable.features->o_features);
        decodable.features->delta->min_frame = o_decoder->status.min_frame;
        last_r.clear();
    }
    qDebug() << "shit [" << shit_counter << "]";
}

void BtEnn::openWave(QString filename)
{
    if( wav_file.isOpen() )
    {
        wav_file.close();
    }

    wav_file.setFileName(filename);
    if( !wav_file.open(QIODevice::ReadWrite) )
    {
        qDebug() << "Failed To Open" << filename;
        exit(1);
    }
//    qDebug() << ">>>>" << QFileInfo(filename).fileName();

    char buff[200];

    wav_file.read(4); // ="RIFF" is the father of wav
    wav_file.read(buff,4);//chunk size(int)
    wav_file.read(buff,4);//format="WAVE"
    wav_file.read(buff,4);//subchunk1 id(str="fmt ")
    wav_file.read(buff,4);//subchunk1(fmt) size(int=16)
    wav_file.read(buff,2);//wav format=1(PCM)(int)

    wav_file.read(buff,2);//Channel Count(int=2)
    uint16_t channel_count = *((uint16_t *)buff);
    wav_file.read(buff,4);//Sample Rate(int=16K)
    uint32_t sample_rate = *((uint32_t *)buff);

    wav_file.read(buff,4);//Byte per sec(int, 64K=16*4)
    wav_file.read(buff,2);//Byte Per Block(int, 4(2*2))
    wav_file.read(buff,2);//Bit Per Sample(int, 16 bit)

    wav_file.read(buff,4);//subchunk2 id(str="data")
    wav_file.read(buff,4);//subchunk2 size(int=sample count)
    uint16_t data_size = *((uint32_t *)buff);
//    qDebug() << "sample_rate:"  << sample_rate
//             << "channel:" << channel_count
//             << "chunk_size:" << data_size;
}

void BtEnn::readWav(BtCyclic *out)
{
    QVector<int16_t> data_buff;
    char buff[200];
    int i = 0;

    while( !wav_file.atEnd() )
    {
        i++;

        wav_file.read(buff, 2);
        data_buff.push_back(*((uint16_t *)buff));
        wav_file.read(buff, 2); // skip second channel
    }
    out->write(&data_buff);
}

void BtEnn::preProcess()
{
    for( int i=0 ; i<last_r.size() ; i++ )
    {
        last_r[i].start -= 0.1;
        last_r[i].end += 0.1;

        if( last_r[i].start<0 )
        {
            last_r[i].start = 0;
        }
        double u_end = o_decoder->uframe/100.0;
        if( last_r[i].end>u_end )
        {
            last_r[i].end = u_end;
        }
    }
}

bool BtEnn::sanityCheck(QString filename)
{
    QFileInfo file_info(filename);
    QString fname = file_info.fileName();

    fname.remove(".wav");
    int dot_index = fname.indexOf(".");
    fname = fname.mid(0, dot_index);

    wav_w->readWordList();
    QString d_name = wav_w->wordToId(last_r); // d_name : detected name

    if( d_name==fname )
    {
        return true;
    }
    else
    {
        qDebug() << "d_name" << d_name << "fname" << fname;
        return false;
    }
}

void BtEnn::saveFeature(QString filename, BtCFB *cfb)
{
    QFileInfo file_info(filename);
    QString fname = file_info.fileName();

    fname.remove(".wav");

    for( int i=0 ; i<last_r.size() ; i++ )
    {
        QString path = KAL_AU_DIR"enn/";
        path += last_r[i].word;
        bt_mkDir(path);
        path += "/" + cat_dir;
        path += "_" + fname;
        path += "_" + QString::number(i);

        int len = 100*(last_r[i].end - last_r[i].start);
        int start = 100*last_r[i].start + o_decoder->status.min_frame;

//        qDebug() << "timing :" << len << start << 100*last_r[i].start;

        QVector<BtFrameBuf *> buffer;
        for( int j=0 ; j<len ; j++ )
        {
            BtFrameBuf *buf = cfb->get(start + j);
            buffer.push_back(buf);
        }
        saveImage(path, buffer);
//        saveCSV(path, buffer);
    }
}

void BtEnn::saveImage(QString filename, QVector<BtFrameBuf *> data)
{
    int len = data.length();
    double sum = 0;
    QImage *img = new QImage(len, BT_ENN_SIZE, QImage::Format_RGB888);

    calcStat(data);
    for( int i=0 ; i<len ; i++ )
    {
        for( int j=0 ; j<BT_ENN_SIZE ; j++ )
        {
            double val = data[i]->enn[j];
            sum += val;
            val -= offset_delta;
            val /= scale_delta;
            val *= 0.15;
            val += 0.5;
            if( val>1 )
            {
                val = 1;
            }
            if( val<0 )
            {
                val = 0;
            }

            float sat_col = 1;
            float hue_col = (1 - val) * 256/360.0;
            float val_col = val;
            QColor pixel;
            pixel.setHsvF(hue_col, sat_col, val_col);
            img->setPixelColor(i, j, pixel);
        }
    }

    QImage img_s = img->scaled(BT_ENN_SIZE, BT_ENN_SIZE);

    if( !img_s.save(filename + ".png", "PNG") )
    {
        qDebug() << "Error: saving image failed.";
    }
//    qDebug() << "E: path: " << filename << max_delta[0] <<
//    max_delta[1] << max_delta[2] << min_delta[0] << min_delta[1]
//    << min_delta[2];
//    qDebug() << "siza :" << len;
    QString cmd = "eog ";
    cmd += filename;
    cmd += ".png &";
//    system(cmd.toStdString().c_str());
}

void BtEnn::saveCSV(QString filename, QVector<BtFrameBuf *> data)
{
    QFile file;
    QTextStream out(&file);
    int len = data.length();

    file.setFileName(filename + ".csv");
    if( !file.open(QIODevice::WriteOnly | QIODevice::Text) )
    {
        qDebug() << "Failed To Create" << filename;
        exit(1);
    }


    for( int i=0 ; i<len ; i++ )
    {
        for( int j=0 ; j<BT_ENN_SIZE ; j++ )
        {
            int val = data[i]->enn[j];
            val += offset_delta;
            out << QString::number(val);
            out << ",";
        }
        out << "\n";
    }

    file.close();
}

void BtEnn::saveWave(QString filename)
{
    double end = o_decoder->uframe/100.0;
    double rw_len = end - last_r[0].end;  // rewind length
    double word_len;
    double sil_len; // silence length

    QFileInfo file_info(filename);
    QString fname = file_info.fileName();
    fname.remove(".wav");

    rw_len *= BT_REC_RATE;
    cy_buf->rewind(rw_len);

    for( int i=0 ; i<last_r.size() ; i++ )
    {
        QString path = KAL_AU_DIR"enn/";
        path += last_r[i].word;
        bt_mkDir(path);
        path += "/" + cat_dir;
        path += "_" + fname;
        path += "_" + QString::number(i);
        path += ".wav";

        word_len = last_r[i].end - last_r[i].start;
        word_len *= BT_REC_RATE;

        wav_w->writeEnn(path, word_len);

        if( i<last_r.size()-1 )
        {
            sil_len  = last_r[i+1].end - last_r[i].end;
            sil_len *= BT_REC_RATE;
            cy_buf->rewind(-sil_len); // minus for forwarding :D
        }
    }
}

bool BtEnn::checkExist(QString path)
{
    QFileInfo file_info(path);
    QString fname = file_info.fileName();

    fname.remove(".wav");

    for( int i=0 ; i<exist_list.size() ; i++ )
    {
        if( exist_list[i].contains(fname) )
        {
//            qDebug() << exist_list[i] << fname;
            return true;
        }
    }

    return false;
}

void BtEnn::calcStat(QVector<BtFrameBuf *> data)
{
    int len = data.length();
    double var = 0;
    int N = len * BT_ENN_SIZE;

    double sum = 0;

    for( int i=0 ; i<len ; i++ )
    {
        for( int j=0 ; j<BT_ENN_SIZE ; j++ )
        {
            double val = data[i]->enn[j];
            sum += val;
            var += qPow(val, 2);
//            qDebug() << "val" << val;

            if( max_delta[0]<data[i]->enn[j] )
            {
                max_delta[0] = data[i]->enn[j];
            }
            if( min_delta[0]>data[i]->enn[j] )
            {
                min_delta[0] = data[i]->enn[j];
            }
        }
    }
    double mean = sum/N;
    var = qSqrt(var/N - qPow(mean, 2));
    offset_delta = mean;
    scale_delta = var;
//    qDebug() << "min_delta" << min_delta[0] << max_delta[0] << var << mean;
//    exit(0);
}
