/*
 * tile.h
 * Copyright 2008-2014, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Edward Hutchins <eah1@yahoo.com>
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "object.h"
#include "tiled.h"

#include <QPainterPath>
#include <QPixmap>
#include <QSharedPointer>
#include <QUrl>

#include <memory>
#include <optional>

namespace Tiled {

class ObjectGroup;
class Tileset;

/**
 * A single frame of an animated tile.
 */
struct Frame
{
    bool operator == (const Frame &frame) const
    {
        return tileId == frame.tileId &&
                duration == frame.duration;
    }

    int tileId;
    int duration;
};

class TILEDSHARED_EXPORT Tile : public Object
{
public:
    Tile(int id, Tileset *tileset);
    Tile(const QPixmap &image, int id, Tileset *tileset);

    ~Tile() override;

    int id() const;

    Tileset *tileset() const;
    QSharedPointer<Tileset> sharedTileset() const;

    const QPixmap &image() const;
    const QPainterPath &imageShape() const;
    void setImage(const QPixmap &image);

    const Tile *currentFrameTile() const;

    const QUrl &imageSource() const;
    void setImageSource(const QUrl &imageSource);

    const QRect &imageRect() const;
    void setImageRect(const QRect &imageRect);

    int width() const;
    int height() const;
    QSize size() const;

    QPoint offset() const;

    /**
     * Returns the offset at which the image of this tile should be drawn,
     * relative to the position of the tile.
     *
     * This is the tile offset of the tileset, adjusted by the pivot of this
     * tile.
     */
    QPoint drawOffset() const;

    /**
     * The pivot of the tile is the point of its image, in pixels relative to
     * the top-left corner of the image, that is kept in place when the tile is
     * drawn on the map. The default pivot is (0, 0), which means the image is
     * drawn at its usual position, adjusted only by the tile offset of the
     * tileset.
     */
    QPoint pivot() const;
    void setPivot(const QPoint &pivot);

    // For Python API compatibility
    const QString &type() const { return className(); }
    void setType(const QString &type) { setClassName(type); };

    qreal probability() const;
    void setProbability(qreal probability);

    ObjectGroup *objectGroup() const;
    void setObjectGroup(std::unique_ptr<ObjectGroup> objectGroup);
    void swapObjectGroup(std::unique_ptr<ObjectGroup> &objectGroup);

    const QVector<Frame> &frames() const;
    void setFrames(const QVector<Frame> &frames);
    bool isAnimated() const;
    int currentFrameIndex() const;
    bool resetAnimation();
    bool advanceAnimation(int ms);

    LoadingStatus imageStatus() const;
    void setImageStatus(LoadingStatus status);

    Tile *clone(Tileset *tileset) const;

private:
    int mId;
    Tileset *mTileset;
    QPixmap mImage;
    mutable std::optional<QPainterPath> mImageShape;   // cache
    QUrl mImageSource;
    QRect mImageRect;
    QPoint mPivot;
    LoadingStatus mImageStatus;
    qreal mProbability;
    std::unique_ptr<ObjectGroup> mObjectGroup;

    QVector<Frame> mFrames;
    int mCurrentFrameIndex;
    int mUnusedTime;

    friend class Tileset; // To allow changing the tile id
};

/**
 * Returns ID of this tile within its tileset.
 */
inline int Tile::id() const
{
    return mId;
}

/**
 * Returns the tileset that this tile is part of.
 */
inline Tileset *Tile::tileset() const
{
    return mTileset;
}

/**
 * Returns the URL of the external image that represents this tile.
 * When this tile doesn't refer to an external image, an empty URL is
 * returned.
 */
inline const QUrl &Tile::imageSource() const
{
    return mImageSource;
}

inline void Tile::setImageSource(const QUrl &imageSource)
{
    mImageSource = imageSource;
}

/**
 * Returns the image source rect in pixels.
 */
inline const QRect &Tile::imageRect() const
{
    return mImageRect;
}

/**
 * Returns the width of this tile.
 */
inline int Tile::width() const
{
    return mImageRect.width();
}

/**
 * Returns the height of this tile.
 */
inline int Tile::height() const
{
    return mImageRect.height();
}

/**
 * Returns the size of this tile.
 */
inline QSize Tile::size() const
{
    return mImageRect.size();
}

/**
 * Returns the offset at which the image of this tile should be drawn, relative
 * to the position of the tile.
 *
 * The pivot is subtracted from the tile offset, so that the pivot point of the
 * image is aligned with the position of the tile.
 *
 * @see pivot
 */
inline QPoint Tile::drawOffset() const
{
    return offset() - mPivot;
}

/**
 * Returns the pivot point of this tile, in pixels relative to the top-left
 * corner of its image.
 *
 * @see setPivot
 */
inline QPoint Tile::pivot() const
{
    return mPivot;
}

/**
 * Returns the relative probability of this tile appearing while painting.
 */
inline qreal Tile::probability() const
{
    return mProbability;
}

/**
 * Set the relative probability of this tile appearing while painting.
 */
inline void Tile::setProbability(qreal probability)
{
    mProbability = probability;
}

/**
 * @return The group of objects associated with this tile. This is generally
 *         expected to be used for editing collision shapes.
 */
inline ObjectGroup *Tile::objectGroup() const
{
    return mObjectGroup.get();
}

inline const QVector<Frame> &Tile::frames() const
{
    return mFrames;
}

inline bool Tile::isAnimated() const
{
    return !mFrames.isEmpty();
}

inline int Tile::currentFrameIndex() const
{
    return mCurrentFrameIndex;
}

/**
 * Returns the loading status of the image referenced by this tile.
 */
inline LoadingStatus Tile::imageStatus() const
{
    return mImageStatus;
}

inline void Tile::setImageStatus(LoadingStatus status)
{
    mImageStatus = status;
}

/**
 * Name of the tile property used to persist the horizontal part of the pivot.
 */
extern TILEDSHARED_EXPORT const char * const pivotXPropertyName;

/**
 * Name of the tile property used to persist the vertical part of the pivot.
 */
extern TILEDSHARED_EXPORT const char * const pivotYPropertyName;

/**
 * Takes the pivot out of the given tile \a properties, if it is stored there.
 *
 * The properties are removed from \a properties, so they don't show up as
 * regular custom properties of the tile. Returns \c std::nullopt when no pivot
 * was stored in the given properties.
 */
TILEDSHARED_EXPORT std::optional<QPoint> takePivotFromProperties(Properties &properties);

/**
 * Stores the given \a pivot in the given tile \a properties.
 *
 * A null pivot is not stored, so that files of tiles without a custom pivot
 * remain unchanged. Any previously stored pivot is replaced.
 */
TILEDSHARED_EXPORT void addPivotToProperties(const QPoint &pivot, Properties &properties);

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::Tile*)
