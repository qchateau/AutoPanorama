#include "utils.h"

#include <QFileInfo>

namespace autopanorama {

QString generateNewFilename(const QString& filename, const QDir& dir)
{
    QFileInfo output(filename);
    QString basename = output.completeBaseName();
    QString suffix = output.suffix();

    QString final_basename;
    int nr = 0;
    do {
        final_basename = basename;
        if (nr++ > 0)
            final_basename += "_" + QString::number(nr);

        output = QFileInfo(dir.filePath(final_basename + "." + suffix));
    } while (output.exists());
    return output.absoluteFilePath();
}

} // autopanorama
