#include <zlib.h>

#include "n_compress.h"

static const char ieCompressionHackChar[] = {
   0x78, 0x9C
};

static QByteArray ieCompressionHackBA = QByteArray::fromRawData(ieCompressionHackChar, sizeof(ieCompressionHackChar));

QByteArray NCompress_n::deflateData(const QByteArray & data, bool ieCompat)
{
    if (data.isEmpty())
        return QByteArray();

    QByteArray compressedData;

    // Deflate
    compressedData.resize(int (data.length() * 1.01 + 12));
    uLongf size = compressedData.length();
    if (compress ((Bytef*)compressedData.data(), &size, (Bytef*)data.constData(), data.size()) != Z_OK)
        return QByteArray();

    compressedData.resize(size);
    if (ieCompat)
    {
        if (compressedData.left(2) == ieCompressionHackBA)
            compressedData.remove(0, 2);
    }
    return compressedData;
}


QString NCompress_n::toString(NCompress_n::CompressionType ct)
{
    if (ct == NCompress_n::ctDeflate)
        return "deflate";

    if (ct == NCompress_n::ctGZip)
        return "gzip";

    Q_ASSERT(false);
    return "";
}
