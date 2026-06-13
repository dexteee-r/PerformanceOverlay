#pragma once

#include <QtQuick3D/QQuick3DGeometry>
#include <QtQml/qqmlregistration.h>

// Géométrie d'un nuage de points sphérique pour le noyau « FLUX DE CHARGE ».
//
// Le rendu de vrais GL points (PrimitiveType.Points + gl_PointSize) ne marche pas
// sur le backend RHI Direct3D 11 (Windows par défaut) : les points y sont clampés
// à 1 px. On émet donc, pour chaque point, un petit QUAD (2 triangles) billboardé
// dans le shader vers la caméra et masqué en rond dans le fragment → contrôle total
// de la taille, fidèle au rendu de la maquette, et indépendant du backend.
//
// Distribution des points : sphère de Fibonacci (spirale dorée) → répartition
// régulière paramétrée par un seul `count` (= garde-fou perf direct).
//
// Attributs par sommet : position (vec3, = point unitaire sur la sphère, sert AUSSI
// de normale puisque la sphère est centrée à l'origine) + texCoord0 (vec2, offset du
// coin du quad dans [-1,1]). Indices 32 bits (count*4 dépasse vite 65535).
class SpherePointGeometry : public QQuick3DGeometry
{
    Q_OBJECT
    QML_NAMED_ELEMENT(SpherePointGeometry)

    // Nombre de points du nuage. Borné [500, 16000] : au-delà, coût GPU inutile pour
    // un overlay (la maquette densifie à ~195k, illisible et lourd ici).
    Q_PROPERTY(int count READ count WRITE setCount NOTIFY countChanged)

public:
    explicit SpherePointGeometry(QQuick3DObject *parent = nullptr);

    int count() const { return m_count; }
    void setCount(int c);

signals:
    void countChanged();

private:
    void rebuild();

    int m_count = 4200;
};
