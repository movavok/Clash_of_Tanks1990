#include "level.h"
#include <cmath>

Level::Level(int newCollumns, int newRows, int newTileSize)
    : cols(newCollumns), rows(newRows), tileSize(newTileSize), grid(newCollumns * newRows, Empty)
{
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            bool border = (x == 0 || y == 0 || x == cols - 1 || y == rows - 1);
            if (border) setTile(x, y, Wall);
            else if ((x + y) % 5 == 0) setTile(x, y, Brick);
            else setTile(x, y, Empty);
        }
    }
}

int Level::tileAt(int cordX, int cordY) const {
    if (cordX < 0 || cordY < 0 || cordX >= cols || cordY >= rows) return Wall;
    return grid[indexAt(cordX, cordY)];
}

void Level::setTile(int cordX, int cordY, int type) {
    if (cordX < 0 || cordY < 0 || cordX >= cols || cordY >= rows) return;
    grid[indexAt(cordX, cordY)] = type;
}

void Level::render(QPainter* painter) const {
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            int type = grid[indexAt(x, y)];
            QRect r(x * tileSize, y * tileSize, tileSize, tileSize);
            switch (type) {
            case Empty:
                painter->setBrush(QColor(30,30,30));
                painter->setPen(Qt::NoPen);
                painter->drawRect(r);
                break;
            case Wall:
                painter->setBrush(QColor(120,120,120));
                painter->setPen(Qt::black);
                painter->drawRect(r);
                break;
            case Brick:
                painter->setBrush(QColor(160,70,50));
                painter->setPen(Qt::black);
                painter->drawRect(r);
                break;
            }
        }
    }
}

bool Level::intersectsSolid(const QRectF& rect) const {
    int left   = static_cast<int>(std::floor(rect.left() / tileSize));
    int right  = static_cast<int>(std::floor((rect.right() - 1) / tileSize));
    int top    = static_cast<int>(std::floor(rect.top() / tileSize));
    int bottom = static_cast<int>(std::floor((rect.bottom() - 1) / tileSize));

    for (int ty = top; ty <= bottom; ++ty) {
        for (int tx = left; tx <= right; ++tx) {
            int type = tileAt(tx, ty);
            if (type == Wall || type == Brick) return true;
        }
    }
    return false;
}

bool Level::destroyInRect(const QRectF& rect) {
    bool destroyed = false;
    int left   = static_cast<int>(std::floor(rect.left() / tileSize));
    int right  = static_cast<int>(std::floor((rect.right() - 1) / tileSize));
    int top    = static_cast<int>(std::floor(rect.top() / tileSize));
    int bottom = static_cast<int>(std::floor((rect.bottom() - 1) / tileSize));

    for (int ty = top; ty <= bottom; ++ty) {
        for (int tx = left; tx <= right; ++tx) {
            int type = tileAt(tx, ty);
            if (type == Brick) {
                setTile(tx, ty, Empty);
                destroyed = true;
            }
        }
    }
    return destroyed;
}

int Level::indexAt(int cordX, int cordY) const { return cordY * cols + cordX; }
