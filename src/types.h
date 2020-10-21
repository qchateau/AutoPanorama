#ifndef TYPES_H
#define TYPES_H

#include <QFileInfo>

namespace autopanorama {

struct OutputFiles {
    QFileInfo output;
    QFileInfo mask;
};

} // autopanorama

#endif // TYPES_H
