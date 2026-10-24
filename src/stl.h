#pragma once

#include "meshdata.h"

#include <QString>

// Loads a binary or ASCII STL file. On failure returns false and sets `error`.
bool loadStl(const QString &path, MeshData &mesh, QString &error);
