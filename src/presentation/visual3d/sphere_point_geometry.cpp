#include "sphere_point_geometry.h"

#include <QByteArray>
#include <QVector3D>
#include <cmath>

SpherePointGeometry::SpherePointGeometry(QQuick3DObject *parent)
    : QQuick3DGeometry(parent)
{
    rebuild();
}

void SpherePointGeometry::setCount(int c)
{
    if (c < 500)        c = 500;
    else if (c > 32000) c = 32000;
    if (c == m_count)
        return;
    m_count = c;
    rebuild();
    emit countChanged();
}

void SpherePointGeometry::rebuild()
{
    const int n = m_count;

    // 5 floats / sommet : position(3) + corner uv(2). 4 sommets / point.
    constexpr int kFloatsPerVertex = 5;
    constexpr int kVertsPerPoint = 4;
    const int stride = kFloatsPerVertex * int(sizeof(float));

    QByteArray vbuf;
    vbuf.resize(n * kVertsPerPoint * stride);
    auto *v = reinterpret_cast<float *>(vbuf.data());

    QByteArray ibuf;
    ibuf.resize(n * 6 * int(sizeof(quint32)));
    auto *idx = reinterpret_cast<quint32 *>(ibuf.data());

    // Coins du quad (repère écran) : on les passe en texCoord0, le shader les
    // utilise pour billboarder (offset) puis pour le masque rond (dot <= 1).
    static const float corners[4][2] = {
        { -1.f, -1.f }, { 1.f, -1.f }, { 1.f, 1.f }, { -1.f, 1.f }
    };

    const float golden = float(M_PI) * (3.f - std::sqrt(5.f)); // angle d'or
    const float denom = (n > 1) ? float(n - 1) : 1.f;

    float *vp = v;
    quint32 *ip = idx;
    for (int i = 0; i < n; ++i) {
        // Sphère de Fibonacci : y régulier de +1 à -1, angle en spirale.
        const float y = 1.f - (float(i) / denom) * 2.f;
        const float r = std::sqrt(std::max(0.f, 1.f - y * y));
        const float theta = golden * float(i);
        const float x = std::cos(theta) * r;
        const float z = std::sin(theta) * r;

        for (int k = 0; k < kVertsPerPoint; ++k) {
            vp[0] = x;
            vp[1] = y;
            vp[2] = z;
            vp[3] = corners[k][0];
            vp[4] = corners[k][1];
            vp += kFloatsPerVertex;
        }

        const quint32 base = quint32(i * kVertsPerPoint);
        ip[0] = base + 0; ip[1] = base + 1; ip[2] = base + 2;
        ip[3] = base + 0; ip[4] = base + 2; ip[5] = base + 3;
        ip += 6;
    }

    clear();
    setStride(stride);
    setVertexData(vbuf);
    setIndexData(ibuf);
    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0,
                 QQuick3DGeometry::Attribute::F32Type);
    addAttribute(QQuick3DGeometry::Attribute::TexCoord0Semantic, 3 * int(sizeof(float)),
                 QQuick3DGeometry::Attribute::F32Type);
    addAttribute(QQuick3DGeometry::Attribute::IndexSemantic, 0,
                 QQuick3DGeometry::Attribute::U32Type);
    setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);

    // Bornes élargies : les points sont déplacés vers l'extérieur par le bruit
    // (jusqu'à ~1.3) + scale global ; éviter un culling intempestif.
    setBounds(QVector3D(-1.5f, -1.5f, -1.5f), QVector3D(1.5f, 1.5f, 1.5f));

    update();
}
