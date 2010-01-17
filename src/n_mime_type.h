#ifndef N_MIME_TYPE_H
#define N_MIME_TYPE_H

// Qt
#include <QString>
#include <QFileInfo>

// http://www.iana.org/assignments/media-types/
// http://www.mimetype.org/

namespace NMimeType_n {
	// Return mime typ fx suffix
	// Return "application/octet-stream" if suffix is not registered
	const QString fileSuffixToMIME(const QString & suffix);

	// Return true if file format is note considered alreadey commressed
	// If content type is not managed, return will be false, in order to limit server overload
	bool contentTypeNeedCompression(const QString & contentType);
};

#endif //N_MIME_TYPE_H
