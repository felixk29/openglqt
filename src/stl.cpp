#include "stl.h"

#include <QDataStream>
#include <QFile>
#include <QTextStream>
#include <QtEndian>

namespace {
    constexpr qint64 HeaderSize=80;
    constexpr qint64 TriangleSize=50;
    constexpr qint64 NormalSize=12;
    constexpr qint64 AttributeSize=2;
    

    bool isBinary(const QByteArray &data){
        if(data.size()<HeaderSize+4){
            return false;
        }

        //count is saved after header
        const quint32 count= qFromLittleEndian<quint32>(data.constData()+HeaderSize);
        return data.size()==qint64(HeaderSize)+4+TriangleSize*qint64(count);

    }

    void addTriangle(MeshData &mesh, const QVector3D &a, const QVector3D &b,const QVector3D &c){
        const QVector3D n= QVector3D::normal(a,b,c);
        const auto first=std::uint32_t(mesh.vertices.size());
        mesh.vertices.push_back({a,n});
        mesh.vertices.push_back({b,n});
        mesh.vertices.push_back({c,n});
        mesh.indices.insert(mesh.indices.end(),{first,first+1,first+2});
    }

    bool parseBinary(const QByteArray &data, MeshData &mesh, QString &error){
        QDataStream in(data);
        in.setByteOrder(QDataStream::LittleEndian);
        in.setFloatingPointPrecision(QDataStream::SinglePrecision);

        in.skipRawData(HeaderSize);
        quint32 count=0;
        in>>count;
        mesh.vertices.reserve(count*3);
        mesh.indices.reserve(count*3);

        for (quint32 t=0; t<count;t++){
            in.skipRawData(NormalSize); //skip normals,newly calculated in addtriangle

            QVector3D point[3];
            for (QVector3D &p: point){
                float x,y,z;
                in >>x>>y>>z;
                p=QVector3D(x,y,z);
            }

            in.skipRawData(AttributeSize); //dont care 
            
            addTriangle(mesh,point[0],point[1],point[2]);
        }

        if (in.status() != QDataStream::Ok) {
            error = QStringLiteral("The file ended unexpectedly.");
            return false;
        }

        return true;

    }

    bool parseAscii(const QByteArray &data, MeshData &mesh, QString &error){
        QTextStream in(data);
        QString word;
        QVector3D point[3];
        int pointCount = 0;

        while(!in.atEnd()){
            in>>word;

            if(word!=QLatin1String("vertex"))
                continue;


            float x,y,z;
            in >> x >> y >> z;
            if (in.status()!=QTextStream::Ok){
                error = QStringLiteral("Invalid number after vertex.");
                return false;
            }

            point[pointCount++]=QVector3D(x,y,z);
            if(pointCount==3){
                addTriangle(mesh,point[0],point[1],point[2]);
                pointCount=0;
            }

        }

        return true;

    }


}


bool loadStl(const QString &path, MeshData &mesh, QString &error){
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)){
        error=file.errorString();
        return false;
    }

    const QByteArray data=file.readAll();
    mesh=MeshData();


    const bool ok = isBinary(data) ? 
                    parseBinary(data, mesh, error): parseAscii(data, mesh, error);
    
    if (!ok) return false;

    mesh.computeBounds();
    return true;
}