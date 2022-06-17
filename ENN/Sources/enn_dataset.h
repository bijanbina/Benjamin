#ifndef ENN_PARSER_H
#define ENN_PARSER_H

#include <QString>
#include <QDir>
#include <QDebug>

#include <tiny_dnn/tiny_dnn.h>
#include "config.h"

using namespace std;
using namespace tiny_dnn;

class EnnDataset
{
public:
    EnnDataset(QString word); // binary word to classify
    ~EnnDataset();

    vector<vec_t>   train_images;
    vector<label_t> train_labels;
    vector<vec_t>   train_labels_v;
    vector<vec_t>   test_images;
    vector<label_t> test_labels;
    vector<vec_t>   test_labels_v;

    QString m_name; //model name
private:
    void parseImagesT(QString path);
    void parseImagesF(QString path);
    void shuffleData();
    QStringList listImages(QString path, int num=-1);
    QStringList listDirs(QString path);
};

#endif // ENN_PARSER_H
