#ifndef LEVEL_H
#define LEVEL_H

#include <QVector>
#include <QPainter>
#include <QRectF>

class Level {
public:
    enum TileType : int {
        Empty = 0,
        Wall = 1,
        Brick = 2
    };

    Level(int, int, int);

    void render(QPainter*) const;

    int getTileSize() const { return tileSize; }
    int getCols() const { return cols; }
    int getRows() const { return rows; }

    bool intersectsSolid(const QRectF&) const;
    bool destroyInRect(const QRectF&);

private:
    int cols;
    int rows;
    int tileSize;
    QVector<int> grid;

    int indexAt(int, int) const;
    int tileAt(int, int) const;
    void setTile(int, int, int);
};

#endif // LEVEL_H
