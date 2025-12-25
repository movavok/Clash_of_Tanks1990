#include "level.h"

Level::Level(int newCollumns, int newRows, int newTileSize)
    : cols(newCollumns), rows(newRows), tileSize(newTileSize), grid(newCollumns * newRows, TileType::Empty) {}

Level::TileType Level::tileAt(int cordX, int cordY) const {
    if (cordX < 0 || cordY < 0 || cordX >= cols || cordY >= rows) return TileType::Wall;
    return grid[indexAt(cordX, cordY)];
}

void Level::setTile(int cordX, int cordY, TileType type) {
    if (cordX < 0 || cordY < 0 || cordX >= cols || cordY >= rows) return;
    grid[indexAt(cordX, cordY)] = type;
}

void Level::render(QPainter* painter) const {
    static QPixmap bg(":/tiles/background.png");
    static QPixmap wall(":/tiles/wall.png");
    static QPixmap weak(":/tiles/brickWeak.png");
    static QPixmap strong(":/tiles/brickStrong.png");
    static QPixmap water(":/tiles/water.png");
    for (int y = 0; y < rows; ++y)
        for (int x = 0; x < cols; ++x) {
            TileType type = grid[indexAt(x, y)];
            QRect rect(x * tileSize, y * tileSize, tileSize, tileSize);
            switch (type) {
            case TileType::Empty: break;
            case TileType::Wall: painter->drawPixmap(rect, wall); break;
            case TileType::BrickWeak: painter->drawPixmap(rect, weak); break;
            case TileType::BrickStrong: painter->drawPixmap(rect, strong); break;
            case TileType::Grass: painter->drawPixmap(rect, bg); break; //grass rendered in foreground
            case TileType::Water: painter->drawPixmap(rect, water); break;
            }
        }
}

void Level::renderForeground(QPainter* painter) const {
    static QPixmap grass(":/tiles/grass.png");
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            TileType type = grid[indexAt(x, y)];
            if (type != TileType::Grass) continue;
            QRect rect(x * tileSize, y * tileSize, tileSize, tileSize);
            painter->save();
            painter->setOpacity(0.6);
            painter->drawPixmap(rect, grass);
            painter->restore();
        }
    }
}

bool Level::intersectsTankSolid(const QRectF& rect) const {
    return intersectsAnyTiles(rect, {TileType::Wall,
                                     TileType::BrickWeak,
                                     TileType::BrickStrong,
                                     TileType::Water});
}

bool Level::intersectsBulletSolid(const QRectF& rect) const {
    return intersectsAnyTiles(rect, {TileType::Wall,
                                     TileType::BrickWeak,
                                     TileType::BrickStrong});
}

bool Level::intersectsAnyTiles(const QRectF& rect, const QVector<TileType>& tiles) const {
    QVector<QPoint> coords;
    tilesInRect(rect, coords);

    for (const QPoint& point : coords)
        if (tiles.contains(tileAt(point.x(), point.y()))) return true;
    return false;
}

bool Level::destroyInRect(const QRectF& rect) {
    bool destroyed = false;

    QVector<QPoint> coords;
    tilesInRect(rect, coords);

    for (const QPoint& point : coords) {
        TileType type = tileAt(point.x(), point.y());
        if (type == TileType::BrickStrong) { setTile(point.x(), point.y(), TileType::BrickWeak); destroyed = true; Audio::play("brickBreaking"); }
        else if (type == TileType::BrickWeak) { setTile(point.x(), point.y(), TileType::Empty); destroyed = true; Audio::play("brickBreaking"); }
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

Level::TileType Level::mapTileChar(QChar symbol) const {
    switch (symbol.toLatin1()) {
    case '#': return TileType::Wall;
    case 'b': return TileType::BrickWeak;
    case 'B': return TileType::BrickStrong;
    case 'g': return TileType::Grass;
    case '~': return TileType::Water;
    default:  return TileType::Empty;
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
            TileType type = mapTileChar(line[x]);
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
