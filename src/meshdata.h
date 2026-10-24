#pragma once

#include <QVector3D>

#include <cstdint>
#include <vector>

struct Vertex {
    QVector3D position;
    QVector3D normal;
};


struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;

    //bounding box
    QVector3D boundsMin;                  
    QVector3D boundsMax;

    int triangleCount() const { return int(indices.size() / 3); }
    QVector3D size() const { return boundsMax - boundsMin; }
    QVector3D center() const { return (boundsMin + boundsMax) * 0.5f; }

    void computeBounds()
    {
        if (vertices.empty())
            return;
        boundsMin = boundsMax = vertices.front().position;
        for (const Vertex &v : vertices) {
            for (int axis = 0; axis < 3; ++axis) {
                boundsMin[axis] = std::min(boundsMin[axis], v.position[axis]);
                boundsMax[axis] = std::max(boundsMax[axis], v.position[axis]);
            }
        }
    }
};
