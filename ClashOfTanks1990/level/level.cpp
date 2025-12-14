#include "level.h"
#include "../systems/audio.h"
#include <cmath>
#include <QFile>
#include <QTextStream>

Level::Level(int newCollumns, int newRows, int newTileSize)
    : cols(newCollumns), rows(newRows), tileSize(newTileSize), grid(newCollumns * newRows, Empty) {}

int Level::tileAt(int cordX, int cordY) const {
    if (cordX < 0 || cordY < 0 || cordX >= cols || cordY >= rows) return Wall;
    return grid[indexAt(cordX, cordY)];
}

void Level::setTile(int cordX, int cordY, int type) {
    if (cordX < 0 || cordY < 0 || cordX >= cols || cordY >= rows) return;
    grid[indexAt(cordX, cordY)] = type;
}

void Level::render(QPainter* painter) const {
    static QPixmap bg(":/tiles/background.png");
    static QPixmap wall(":/tiles/wall.png");
    static QPixmap weak(":/tiles/brickWeak.png");
    static QPixmap strong(":/tiles/brickStrong.png");
    static QPixmap grass(":/tiles/grass.png");
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            int type = grid[indexAt(x, y)];
            QRect r(x * tileSize, y * tileSize, tileSize, tileSize);
            switch (type) {
            case Empty: break;
            case Wall: painter->drawPixmap(r, wall); break;
            case BrickWeak: painter->drawPixmap(r, weak); break;
            case BrickStrong: painter->drawPixmap(r, strong); break;
            case Grass: painter->drawPixmap(r, bg); break;
            }
        }
    }
}

void Level::renderForeground(QPainter* painter) const {
    static QPixmap grass(":/tiles/grass.png");
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            int type = grid[indexAt(x, y)];
            if (type != Grass) continue;
            QRect r(x * tileSize, y * tileSize, tileSize, tileSize);
            painter->save();
            painter->setOpacity(0.6);
            painter->drawPixmap(r, grass);
            painter->restore();
        }
    }
}

bool Level::intersectsSolid(const QRectF& rect) const {
    QVector<QPoint> coords;
    tilesInRect(rect, coords);

    for (const QPoint& p : coords) {
        int type = tileAt(p.x(), p.y());
        if (type == Wall || type == BrickWeak || type == BrickStrong) return true;
    }
    return false;
}

bool Level::intersectsGrass(const QRectF& rect) const {
    QVector<QPoint> coords;
    tilesInRect(rect, coords);

    for (const QPoint& p : coords)
        if (tileAt(p.x(), p.y()) == Grass) return true;
    return false;
}

bool Level::destroyInRect(const QRectF& rect) {
    bool destroyed = false;

    QVector<QPoint> coords;
    tilesInRect(rect, coords);

    for (const QPoint& p : coords) {
        int type = tileAt(p.x(), p.y());
        if (type == BrickStrong) { setTile(p.x(), p.y(), BrickWeak); destroyed = true; Audio::play("brickBreaking"); }
        else if (type == BrickWeak) { setTile(p.x(), p.y(), Empty); destroyed = true; Audio::play("brickBreaking"); }
    }
    return destroyed;
}

int Level::indexAt(int cordX, int cordY) const { return cordY * cols + cordX; }

void Level::tilesInRect(const QRectF& rect, QVector<QPoint>& out) const {
    int left   = static_cast<int>(std::floor(rect.left() / tileSize));
    int right  = static_cast<int>(std::floor((rect.right() - 1) / tileSize));
    int top    = static_cast<int>(std::floor(rect.top() / tileSize));
    int bottom = static_cast<int>(std::floor((rect.bottom() - 1) / tileSize));
    out.clear();
    for (int ty = top; ty <= bottom; ++ty)
        for (int tx = left; tx <= right; ++tx)
            out.append(QPoint(tx, ty));
}

int mapTileChar(QChar symbol) {
    switch (symbol.toLatin1()) {
    case '#': return Level::Wall;
    case 'b': return Level::BrickWeak;
    case 'B': return Level::BrickStrong;
    case 'g': return Level::Grass;
    default:  return Level::Empty;
    }
}

bool Level::generateFromText(const QStringList& lines) {
    if (lines.isEmpty()) return false;
    int newRows = lines.size();
    int newCols = 0;
    for (const QString& line : lines) newCols = std::max(newCols, static_cast<int>(line.length()));
    if (newCols <= 0) return false;

    rows = newRows;
    cols = newCols;
    grid.resize(cols * rows);

    for (int y = 0; y < rows; ++y) {
        QString line = lines[y];
        if (line.length() < cols) line = line + QString(cols - line.length(), QChar(' '));
        for (int x = 0; x < cols; ++x) {
            int type = mapTileChar(line[x]);
            setTile(x, y, type);
        }
    }
    return true;
}

bool Level::loadFromFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QTextStream in(&file);
    QStringList lines;
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (!line.isNull()) lines.append(line);
    }
    file.close();
    return generateFromText(lines);
}
