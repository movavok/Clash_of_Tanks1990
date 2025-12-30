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
    enum class TileType : char {
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

    bool intersectsTankSolid(const QRectF&) const;
    bool intersectsBulletSolid(const QRectF&) const;
    bool intersectsAnyTiles(const QRectF&, const QVector<TileType>&) const;
    bool checkBlockedLine(const QPointF&, const QPointF&) const;
    bool destroyInRect(const QRectF&);

    bool loadFromFile(const QString&);

private:
    int cols;
    int rows;
    int tileSize;
    QVector<TileType> grid;

    int indexAt(int, int) const;
    TileType tileAt(int, int) const;
    void setTile(int, int, TileType);
    void tilesInRect(const QRectF&, QVector<QPoint>&) const;

    bool generateFromText(const QStringList&);
    TileType mapTileChar(QChar) const;
};

#endif // LEVEL_H
