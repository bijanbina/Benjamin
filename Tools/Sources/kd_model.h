#ifndef KD_MODEL_H
#define KD_MODEL_H

#include "config.h"

#include <QObject>
#include <QDebug>

#include "config.h"
#include "backend.h"
class KdModel
{
public:
    KdModel(std::string model_filename);
    ~KdModel();
};

#endif // KD_MODEL_H
