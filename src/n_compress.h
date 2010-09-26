#ifndef N_COMPRESS_H
#define N_COMPRESS_H

#include <QByteArray>
#include <QString>


namespace NCompress_n {

    enum CompressionType {
        ctGZip,
        ctDeflate
    };


    // Compress a QByteArray
    QByteArray deflateData(const QByteArray & data, bool ieCompat);

    //Return string value of a compression tpe
    QString toString(CompressionType ct);    
};

#endif // N_COMPRESS_H
