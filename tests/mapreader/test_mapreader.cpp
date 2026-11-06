#include "map.h"
#include "mapobject.h"
#include "mapreader.h"
#include "maprenderer.h"
#include "maptovariantconverter.h"
#include "mapwriter.h"
#include "objectgroup.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"
#include "varianttomapconverter.h"

#include <QBuffer>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QtTest/QtTest>

using namespace Tiled;

class test_MapReader : public QObject
{
    Q_OBJECT

private slots:
    void loadMap();
    void tilePivot();
    void tilePivotJson();
    void tilePivotRendering();
};

void test_MapReader::loadMap()
{
    MapReader reader;
    auto map = reader.readMap("../data/mapobject.tmx");

    // TODO: Also test tilesets (internal and external), properties and tile
    // layer data.

    QVERIFY(map.get());
    QCOMPARE(map->layerCount(), 2);
    QCOMPARE(map->width(), 100);
    QCOMPARE(map->height(), 80);
    QCOMPARE(map->tileWidth(), 32);
    QCOMPARE(map->tileHeight(), 32);

    TileLayer *tileLayer = dynamic_cast<TileLayer*>(map->layerAt(0));

    QVERIFY(tileLayer);
    QCOMPARE(tileLayer->width(), 100);
    QCOMPARE(tileLayer->height(), 80);

    ObjectGroup *objectGroup = dynamic_cast<ObjectGroup*>(map->layerAt(1));

    QVERIFY(objectGroup);
    QCOMPARE(objectGroup->name(), QLatin1String("Objects"));
    QCOMPARE(objectGroup->objects().count(), 1);

    MapObject *mapObject = objectGroup->objects().at(0);

    QCOMPARE(mapObject->name(), QLatin1String("Some object"));
    QCOMPARE(mapObject->className(), QLatin1String("WARP"));
    QCOMPARE(mapObject->x(), qreal(200));
    QCOMPARE(mapObject->y(), qreal(200));
    QCOMPARE(mapObject->width(), qreal(128));
    QCOMPARE(mapObject->height(), qreal(64));
}

void test_MapReader::tilePivot()
{
    Map::Parameters mapParameters;
    mapParameters.orientation = Map::Orthogonal;
    mapParameters.width = 1;
    mapParameters.height = 1;
    mapParameters.tileWidth = 32;
    mapParameters.tileHeight = 32;

    Map map(mapParameters);

    auto tileset = Tileset::create(QStringLiteral("Pivot"), 32, 32);
    Tile *tile = tileset->findOrCreateTile(0);
    tile->setPivot(QPoint(8, 20));

    QCOMPARE(tile->drawOffset(), QPoint(-8, -20));
    QCOMPARE(tileset->drawMargins(), QMargins(8, 20, 0, 0));

    map.addTileset(tileset);

    auto tileLayer = new TileLayer(QStringLiteral("Tiles"), 0, 0, 1, 1);
    tileLayer->setCell(0, 0, Cell(tile));
    map.addLayer(tileLayer);

    QByteArray data;
    QBuffer buffer(&data);
    QVERIFY(buffer.open(QIODevice::WriteOnly));

    MapWriter writer;
    writer.writeMap(&map, &buffer);
    buffer.close();

    QVERIFY2(writer.errorString().isEmpty(), qPrintable(writer.errorString()));
    QVERIFY(data.contains("name=\"pivotx\""));
    QVERIFY(data.contains("name=\"pivoty\""));

    QVERIFY(buffer.open(QIODevice::ReadOnly));

    MapReader reader;
    auto readMap = reader.readMap(&buffer, QString());
    buffer.close();

    QVERIFY(readMap);

    QCOMPARE(readMap->tilesetCount(), 1);

    auto readTileset = readMap->tilesets().first();
    QCOMPARE(readTileset->tileCount(), 1);

    Tile *readTile = readTileset->findTile(0);
    QVERIFY(readTile);
    QCOMPARE(readTile->pivot(), QPoint(8, 20));
    QCOMPARE(readTile->drawOffset(), QPoint(-8, -20));
    QCOMPARE(readTileset->drawMargins(), QMargins(8, 20, 0, 0));

    // The pivot should not end up as a regular custom property
    QVERIFY(readTile->properties().isEmpty());

    // The pivot should be honored through the tile referenced by the cell
    auto readTileLayer = dynamic_cast<TileLayer*>(readMap->layerAt(0));
    QVERIFY(readTileLayer);
    QCOMPARE(readTileLayer->cellAt(0, 0).tile()->pivot(), QPoint(8, 20));

    // TileLayer::drawMargins() adds the maximum tile size to the top and right
    const QMargins tilesetMargins = readTileset->drawMargins();
    QCOMPARE(readTileLayer->drawMargins(),
             QMargins(tilesetMargins.left(),
                      tilesetMargins.top() + 32,
                      tilesetMargins.right() + 32,
                      tilesetMargins.bottom()));
}

void test_MapReader::tilePivotJson()
{
    Map::Parameters mapParameters;
    mapParameters.orientation = Map::Orthogonal;
    mapParameters.width = 1;
    mapParameters.height = 1;
    mapParameters.tileWidth = 32;
    mapParameters.tileHeight = 32;

    Map map(mapParameters);

    auto tileset = Tileset::create(QStringLiteral("Pivot"), 32, 32);
    Tile *tile = tileset->findOrCreateTile(0);
    tile->setPivot(QPoint(8, 20));

    map.addTileset(tileset);

    auto tileLayer = new TileLayer(QStringLiteral("Tiles"), 0, 0, 1, 1);
    tileLayer->setCell(0, 0, Cell(tile));
    map.addLayer(tileLayer);

    MapToVariantConverter toVariant;
    const QVariant variant = toVariant.toVariant(map, QDir());

    VariantToMapConverter toMap;
    const auto readMap = toMap.toMap(variant, QDir());
    QVERIFY2(readMap, qPrintable(toMap.errorString()));

    QCOMPARE(readMap->tilesetCount(), 1);

    auto readTileset = readMap->tilesets().first();
    QCOMPARE(readTileset->tileCount(), 1);

    Tile *readTile = readTileset->findTile(0);
    QVERIFY(readTile);
    QCOMPARE(readTile->pivot(), QPoint(8, 20));
    QCOMPARE(readTile->drawOffset(), QPoint(-8, -20));

    // The pivot should not end up as a regular custom property
    QVERIFY(readTile->properties().isEmpty());
}

void test_MapReader::tilePivotRendering()
{
    Map::Parameters mapParameters;
    mapParameters.orientation = Map::Orthogonal;
    mapParameters.width = 1;
    mapParameters.height = 1;
    mapParameters.tileWidth = 32;
    mapParameters.tileHeight = 32;

    Map map(mapParameters);

    auto tileset = Tileset::create(QStringLiteral("Pivot"), 32, 32);

    QPixmap image(32, 32);
    image.fill(Qt::red);

    Tile *tile = tileset->addTile(image);
    tile->setPivot(QPoint(8, 20));

    map.addTileset(tileset);

    auto tileLayer = new TileLayer(QStringLiteral("Tiles"), 0, 0, 1, 1);
    tileLayer->setCell(0, 0, Cell(tile));
    map.addLayer(tileLayer);

    QImage canvas(64, 64, QImage::Format_ARGB32);
    canvas.fill(Qt::transparent);

    auto renderTileLayer = [&](const QPoint &pivot) {
        tile->setPivot(pivot);

        canvas.fill(Qt::transparent);

        QPainter painter(&canvas);

        // Put the top-left corner of the tile at the center of the canvas
        painter.translate(32, 32);

        const auto renderer = MapRenderer::create(&map);
        renderer->drawTileLayer(&painter, tileLayer);

        painter.end();

        return canvas;
    };

    const QImage unshifted = renderTileLayer(QPoint(0, 0));
    const QImage pivoted = renderTileLayer(QPoint(8, 20));

    // Without a pivot, the image is drawn at the position of the tile
    QCOMPARE(unshifted.pixelColor(32, 32), QColor(Qt::red));
    QCOMPARE(unshifted.pixelColor(26, 14), QColor(Qt::transparent));

    // With a pivot, the image is drawn at (32, 32) - the pivot, so at (24, 12)
    QCOMPARE(pivoted.pixelColor(26, 14), QColor(Qt::red));
    QCOMPARE(pivoted.pixelColor(54, 42), QColor(Qt::red));

    // Areas that would only be covered without applying the pivot
    QCOMPARE(pivoted.pixelColor(58, 50), QColor(Qt::transparent));
    QCOMPARE(pivoted.pixelColor(2, 2), QColor(Qt::transparent));
}

QTEST_MAIN(test_MapReader)
#include "test_mapreader.moc"
