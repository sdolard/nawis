#include <zlib.h>

#include "n_compress.h"

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
		static QByteArray ieCompressionHack;
		ieCompressionHack.resize(2);
		ieCompressionHack[0] = 0x78;
		ieCompressionHack[1] = 0x9C;
		if (compressedData.left(2) == ieCompressionHack)
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
