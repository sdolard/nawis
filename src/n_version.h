#ifndef N_VERSION_H
#define N_VERSION_H

// Qt
#include <QString>

inline const QString getNawisVersion() {
    return "0.1.0"; // release.version.bugfix
}

inline const QString getNawisBuild() {
    return "alpha"; // release.version.bugfix
}

namespace NVersion_n {

    const QString namedVersion(bool os = true);
};

#endif // N_VERSION_H
