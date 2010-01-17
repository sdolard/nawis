#ifndef N_VERSION_H
#define N_VERSION_H

// Qt
#include <QString>

#define NAWIS_VERSION        "0.1.0" // release.version.bugfix
#define NAWIS_BUILD          "alpha"

namespace NVersion_n {

	const QString namedVersion(bool os = true);
};

#endif // N_VERSION_H
