// SPDX-License-Identifier: MIT
#include "materialstyleplugin.h"
#include "materialstyle.h"

QStyle *MaterialStylePlugin::create(const QString &key)
{
    if (key.compare(QLatin1String("materialdesign"), Qt::CaseInsensitive) == 0) {
        return new MaterialStyle;
    }
    return nullptr;
}
