#ifndef LEVEL_H
#define LEVEL_H

#include <QVector>
#include <QPainter>
#include <QRectF>
#include <QStringList>
#include <QString>
#include <cmath>
#include <QFile>
#include <QTextStream>
#include "../systems/audio.h"

class Level {
public:
    enum TileType : int {
        Empty = 0,
        Wall = 1,
        BrickWeak = 2,
        BrickStrong = 3,
        Grass = 4
    };

    Level(int, int, int);

    void render(QPainter*) const;
    void renderForeground(QPainter*) const; // overlays

    int getTileSize() const { return tileSize; }
    int getCols() const { return cols; }
    int getRows() const { return rows; }

    bool intersectsSolid(const QRectF&) const;
    bool intersectsGrass(const QRectF&) const;
    bool destroyInRect(const QRectF&);

    bool generateFromText(const QStringList&);
    bool loadFromFile(const QString&);

private:
    int cols;
    int rows;
    int tileSize;
    QVector<int> grid;

    int indexAt(int, int) const;
    int tileAt(int, int) const;
    void setTile(int, int, int);
    void tilesInRect(const QRectF&, QVector<QPoint>&) const;
};

#endif // LEVEL_H
