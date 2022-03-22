﻿#include "bt_captain.h"
#include <QDebug>

BtCaptain::BtCaptain(QObject *parent) : QObject(parent)
{
    setbuf(stdout,NULL); //to work out printf

    time_shifter = new QTimer;
    connect(time_shifter, SIGNAL(timeout()),
            this, SLOT(shiftHistory()));
    time_shifter->start(BT_HISTORY_UPDATE);

    start_treshold = -BT_HISTORY_SIZE/1000.0;
}

void BtCaptain::parse(QVector<BtWord> in_words)
{
    if( in_words.empty() )
    {
        return;
    }

    x_buf = ""; //fill inside add word
    for( int i=0 ; i<in_words.length() ; i++ )
    {
        if( in_words[i].time>start_treshold ) //5 second history size
        {
            addWord(in_words[i], i);
        }
    }
    exec(x_buf);
    writeResult();
}

void BtCaptain::exec(QString word)
{
    if( x_buf.isEmpty() )
    {
        return;
    }

    QString cmd = KAL_SI_DIR"main.sh \"";
    cmd += word;
    cmd += "\"";
    system(cmd.toStdString().c_str());
    qDebug() << "exec" << x_buf;
}

void BtCaptain::addWord(BtWord word, int id)
{
    BtHistory buf;
    // change not final last to final
    // remove all that are not final
    if( id<current.length() )
    {
        if( current[id].words.last()!=word.word )
        {
            if( current[id].is_final )
            {
                current[id].words.push_back(word.word);
            }
            else
            {
                int last_id = current[id].words.length()-1;
                current[id].words[last_id] = word.word;
            }
            addXBuf(word);
        }
        else if( current[id].is_final==0 ) //and same word
        {
            addXBuf(word);
        }
        if( word.is_final )
        {
            current[id].conf = word.conf;
            current[id].time = word.time;
            current[id].is_final = 1;
        }
        return;
    }

    addXBuf(word);
    buf.words.append(word.word);
    buf.conf = word.conf;
    buf.time = word.time;
    buf.is_final = word.is_final;

    current.append(buf);
}

void BtCaptain::addXBuf(BtWord word)
{
    if( word.is_final )
    {
        if( x_buf.length() )
        {
            x_buf += " ";
        }
        x_buf += word.word;
    }
}

void BtCaptain::shiftHistory()
{
    start_treshold += BT_HISTORY_UPDATE/1000.0;

    for( int i=0 ; i<history.length() ; i++ )
    {
        if( history[i].time<start_treshold ) //5 second history size
        {
            history.remove(i);
            i--;
        }
    }

    for( int i=0 ; i<current.length() ; i++ )
    {
        if( current[i].time<start_treshold ) //5 second history size
        {
            current.remove(i);
            i--;
        }
    }

    writeResult();
}

void BtCaptain::writeResult()
{
    QFile bar_file(BT_BAR_RESULT);

    if( !bar_file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Error opening" << BT_BAR_RESULT;
        return;
    }
    QTextStream out(&bar_file);

    for( int i=0 ; i<history.length() ; i++ )
    {
        out << getWordFmt(history[i]);
    }

    for( int i=0 ; i<current.length() ; i++ )
    {
        out << getWordFmt(current[i]);
    }

    out << "\n";

    bar_file.close();
}

//get word polybar formatted
QString BtCaptain::getWordFmt(BtHistory word)
{
    QString buf;

    if( word.is_final )
    {
        buf = "%{F#ddd}";
    }
    else
    {
        buf = "%{F#777}";
    }

    if( word.conf<KAL_HARD_TRESHOLD )
    {
        buf += "%{u#f00}%{+u}";
    }
    else if( word.conf==1.00 )
    {
        buf += "%{u#1d1}%{+u}";
    }
    else if( word.conf>KAL_CONF_TRESHOLD )
    {
        buf += "%{u#16A1CF}%{+u}";
    }
    else
    {
        buf += "%{u#CF8516}%{+u}";
    }

    int len = word.words.length();
    for( int i=0 ; i<len ; i++ )
    {
        buf += word.words[i];

        if( i<(len-1) )
        {
            buf += "->";
        }
    }

    buf += "%{-u} ";

    return buf;
}

void BtCaptain::flush()
{
    for( int i=0 ; i<current.length() ; i++ )
    {
        if( current[i].time>start_treshold ) //5 second history size
        {
            history.push_back(current[i]);
            current.remove(i);
            i--;
        }
    }
}
