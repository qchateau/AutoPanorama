#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QDir>

namespace autopanorama {

QString generateNewFilename(const QString& filename, const QDir& dir);

} // autopanorama

#endif // UTILS_H
