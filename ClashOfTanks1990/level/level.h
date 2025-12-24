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
    enum TileType : char {
        Empty = ' ',
        Wall = 'w',
        BrickWeak = 'b',
        BrickStrong = 'B',
        Grass = 'g',
        Water = '~'
    };

    Level(int, int, int);

    void render(QPainter*) const;
    void renderForeground(QPainter*) const; // overlays

    int getTileSize() const { return tileSize; }
    int getCols() const { return cols; }
    int getRows() const { return rows; }

    bool intersectsSolid(const QRectF&) const;
    bool intersectsTile(const QRectF&, char) const;
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
    void setTile(int, int, char);
    void tilesInRect(const QRectF&, QVector<QPoint>&) const;
};

#endif // LEVEL_H
