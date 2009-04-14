// openhomm - open source clone of Heroes of Might and Magic III
// Copyright (C) 2009 openhomm developers team (see AUTHORS)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef HRSCENE_H
#define HRSCENE_H

#include "hrTile.hpp"
#include "hrGraphicsItem.hpp"
#include "hrCoord.hpp"
#include "hrSceneObject.hpp"

class hrScene
{
public:
    hrScene(int width, int height);
    ~hrScene();

    void addTile(hrTile tile);
    void addObject(hrSceneObject object);
    void removeObject(int x, int y);

    void setCursor(QString name);
    const QCursor& getCursor(int frame);

    QRect getSize() const;
    void setSceneViewport(QRect r);
    QRect getSceneViewport() const;

    QImage getImage(int id
                   , int frame
                   , bool horizontal = false
                   , bool vertical = false) const;
    QImage getImageTerrain(const hrTile &tile) const;
    QImage getImageRiver(const hrTile &tile) const;
    QImage getImageRoad(const hrTile &tile) const;
    QImage getImage(const hrSceneObject &object) const;
    hrGraphicsItem* getItem(const hrSceneObject &object) const;
    hrGraphicsItem* getItem(int id) const;

    QVector<hrTile> getViewportTiles() const;
    hrTile getTile(int x, int y) const;
    QLinkedList<hrSceneObject> getViewportObjects() const;
    QLinkedList<hrSceneObject> getAllObjects() const;

public slots:
//mouseClick()
//mouseMove()


private:
    QVector< QVector<hrTile> > tiles;
    QLinkedList<hrSceneObject> objects;
    QMap<int, hrGraphicsItem*> items;
    QHash<QString, hrGraphicsItem*> items_obj;
    QVector<QCursor> cursor;
    QRect viewport;
    QRect size;

    void addItem(int id, QString name, bool mirrored = false);
    void addItem(QString name);
    void CyclShiftPalette(int a, int b, QImage &im);
};

#endif // HRSCENE_H
