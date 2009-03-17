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
#include "hrDefHandler.hpp"
#include <QImage>
#include <QVector>
#include <QRgb>
#include <QVariant>

struct DefHeader
{
    quint32 type;
    quint32 width;
    quint32 height;
    quint32 countBlocks;
};

struct BlockHeader
{
    quint32 index;
    quint32 countFrames;
    quint8 junk[8];
};

struct FrameHeader
{
    quint32 size;
    quint32 type;
    quint32 widthFull;
    quint32 heightFull;
    quint32 widthFrame;
    quint32 heightFrame;
    quint32 marginLeft;
    quint32 marginTop;
};

struct Block
{
    int countFrames;
    QVector<quint32> offsets;
};

class DefReader
{
public:
    DefReader(QIODevice *device);
    ~DefReader();
    bool read(QImage *image);
    int currentImageNumber() const;
    bool jumpToImage(int imageNumber);
    bool jumpToNextImage();

    int count();
    int getWidth();
    int getHeight();

    static bool canRead(QIODevice *device);

private:
    bool readHeader();
    bool readPalette();
    bool readBlockHeaders();

    bool readFrame0(QImage & image, FrameHeader & fh);
    bool readFrame1(QImage & image, FrameHeader & fh);

    bool checkFrame(FrameHeader &fh);
    void fillFrameBorders(FrameHeader &fh);

    QIODevice *dev;

    QVector<Block> blocks;
    QVector<QRgb> colors;
    quint8 *imageBuffer;
    int curFrame;
    int countFrames;
    int curBlock;
    int countBlocks;
    int height;
    int width;

    bool isHeaderRead;
};

DefReader::DefReader(QIODevice *device)
{
    dev = device;
    imageBuffer = NULL;
    curFrame = 0;
    countFrames = 0;
    curBlock = 0;
    countBlocks = 0;
    height = 0;
    width = 0;
    isHeaderRead = false;
}

DefReader::~DefReader()
{
    if (!imageBuffer)
        delete imageBuffer;
}

int DefReader::count()
{
    if (countFrames == 0 && !isHeaderRead)
        if (readHeader())
            isHeaderRead = true;
    return countFrames;
}

int DefReader::getWidth()
{
    if (!isHeaderRead)
        readHeader();
    return width;
}

int DefReader::getHeight()
{
    if (!isHeaderRead)
        readHeader();
    return width;
}

int DefReader::currentImageNumber() const
{
    return curFrame;
}

bool DefReader::jumpToImage(int imageNumber)
{
    if (!count())
        return false;

    if (imageNumber < 0)
    {
        if (qAbs(imageNumber) > countBlocks)
            return false;
        curBlock = qAbs(imageNumber) - 1;
        curFrame = 0;
        countFrames = blocks[curBlock].countFrames;
    }
    else if (imageNumber >= 0 && imageNumber < countFrames)
        curFrame = imageNumber;
    else
        return false;
    return true;
}

bool DefReader::jumpToNextImage()
{
    if (!count())
        return false;

    curFrame < countFrames ? curFrame++ : curFrame = 0;
    return true;
}

bool DefReader::readPalette()
{
    quint8 buf[256 * 3];
    if (dev->read((char*)buf, 256 * 3) == 256 * 3)
    {
        QRgb rgb;
        for(int i = 0, j = 0; i < 256; i++, j += 3)
        {
            rgb = qRgb(buf[j], buf[j + 1], buf[j + 2]);
            colors.append(rgb);
        }

        return true;
    }
    return false;
}

bool DefReader::readBlockHeaders()
{
    BlockHeader bh;
    for (int i = 0; i < countBlocks; i++)
    {
        Block block;
        if (dev->read((char*)&bh, sizeof(bh)) == sizeof(bh))
        {
            block.countFrames = bh.countFrames;

            if (!dev->seek(dev->pos() + bh.countFrames * 13)) // skip names
                return false;

            int cnt = bh.countFrames;
            quint32 *offsets = new quint32[cnt];
            if (dev->read((char*)offsets, cnt * 4) == cnt * 4)
            {
                for (int j = 0; j < cnt; j++)
                    block.offsets.append(offsets[j]);
                blocks.append(block);

                countFrames = blocks[0].countFrames;
            }
            else
            {
                delete [] offsets;
                return false;
            }
            delete [] offsets;
        }
        else return false;
    }
    return true;
}

bool DefReader::readHeader()
{
    DefHeader dh;
    if (dev->read((char*)&dh, sizeof(dh)) == sizeof(dh))
    {
        width = dh.width;
        height = dh.height;
        countBlocks = dh.countBlocks;

        return readPalette() && readBlockHeaders();
    }
    return false;
}

bool DefReader::readFrame0(QImage & image, FrameHeader & fh)
{
    if (!imageBuffer) delete imageBuffer;
    imageBuffer = new quint8[fh.size];

    if (dev->read((char*)imageBuffer, fh.size) == fh.size)
    {
        image = QImage(imageBuffer, width, height, width, QImage::Format_Indexed8);
        image.setColorTable(colors);
    }
    else
        return false;
    return true;
}

bool DefReader::checkFrame(FrameHeader &fh)
{
    if (fh.marginLeft + fh.widthFrame <= fh.widthFull
        && fh.marginTop + fh.heightFrame <= fh.heightFull)
        return true;
    else
        return false;
}

void DefReader::fillFrameBorders(FrameHeader &fh)
{
    if (fh.widthFrame == fh.widthFull
        && fh.heightFrame == fh.heightFull)
        return;
    quint8 *line = imageBuffer;
    quint32 i = 0;
    quint32 j = 0;

    for (; j < fh.marginTop; j++)
        for (i = 0; i < fh.widthFull; i++)
            *line++ = 0;

    for (; j < fh.marginTop + fh.heightFrame; j++)
    {
        for (i = 0; i < fh.marginLeft; i++)
            *line++ = 0;
        line += fh.widthFrame;
        for (i += fh.widthFrame; i < fh.widthFull; i++)
            *line++ = 0;
    }

    for (; j < fh.heightFull; j++)
        for (i = 0; i < fh.widthFull; i++)
            *line++ = 0;
}

bool DefReader::readFrame1(QImage & image, FrameHeader & fh)
{
    if (!checkFrame(fh))
        return false;

    quint32 sizeFull = fh.widthFull * fh.heightFull;

    if (!imageBuffer) delete imageBuffer;
    imageBuffer = new quint8[sizeFull];

    //memset(imageBuffer, 0, sizeFull);
    fillFrameBorders(fh);

    quint8 *buf = new quint8[fh.size];
    if (dev->read((char*)buf, fh.size) != fh.size)
        return false;

    quint32 *offsets = (quint32*)&buf[0];

    for (quint32 curLine = 0; curLine < fh.heightFrame; curLine++)
    {
        quint32 offsetImageBuffer = (fh.marginTop + curLine) * fh.widthFull + fh.marginLeft;

        quint8 *imageLine = imageBuffer + offsetImageBuffer;
        quint32 offsetImageLine = 0;

        if (offsets[curLine] > fh.size)
            return false;

        quint8 *line = buf + offsets[curLine];

        int lenLine = 0;
        if (curLine < fh.heightFrame - 1)
            lenLine = offsets[curLine + 1] - offsets[curLine];
        else
            lenLine = fh.size - offsets[curLine];

        if (lenLine < 0 || offsets[curLine] + lenLine > fh.size)
            return false;

        int offset = 0;

        while (offset < lenLine)
        {
            quint8 typeSegment = line[offset];
            if (typeSegment == 0xFF)
            {
                quint8 lenSegment = line[++offset] + 1;
                if (offsetImageLine + lenSegment > fh.widthFrame)
                    return false;

                offset++;
                for (quint8 k = 0; k < lenSegment; k++)
                {
                    imageLine[offsetImageLine++] = line[offset++];
                }
            }
            else //if (typeSegment == 0x00)
            {
                quint8 lenSegment = line[++offset] + 1;
                if (offsetImageLine + lenSegment > fh.widthFrame)
                    return false;

                for (quint8 k = 0; k < lenSegment; k++)
                {
                    imageLine[offsetImageLine++] = typeSegment;//0;
                }
                offset++;
            }
        }
    }
    image = QImage(imageBuffer, fh.widthFull, fh.heightFull, fh.widthFull, QImage::Format_Indexed8);
    image.setColorTable(colors);
    delete [] buf;
    return true;
}


bool DefReader::read(QImage *image)
{
    if (!count()) // forces header to be read
        return false;

    if (!dev->seek(blocks[curBlock].offsets[curFrame]))
        return false;

    FrameHeader fh;
    dev->read((char*)&fh, sizeof(fh));

    if (fh.type == 0)
    {
        return readFrame0(*image, fh);
    }
    if (fh.type == 1)
    {
        return readFrame1(*image, fh);
    }

    return false;
}

bool DefReader::canRead(QIODevice *device)
{
    if (!device)
        return false;
    bool isProbablyDef = false;
    qint64 oldPos = device->pos();
    device->seek(0);
    // def format does not have a magic number
    DefHeader dh;
    if (device->read((char*)&dh, sizeof(dh)) == sizeof(dh))
    {
        if (dh.width <= 800
            && dh.height <= 600 // heroes 3 original resolution
            && dh.countBlocks < 0xFFFF // probably..
            )
        {
            isProbablyDef = true;
        }
    }
    device->seek(oldPos);
    return isProbablyDef;
}


hrDefHandler::hrDefHandler(QIODevice *device)
{
    setDevice(device);
    dr = new DefReader(device);
}

hrDefHandler::~hrDefHandler()
{
    if (!dr) delete dr;
}

bool hrDefHandler::read(QImage *image)
{
    /*if (!canRead())
        return false;*/
    return dr->read(image);
}

bool hrDefHandler::write(const QImage &image)
{
    qWarning("%s: not supported", Q_FUNC_INFO);
    return false;
}

int hrDefHandler::currentImageNumber() const
{
    return dr->currentImageNumber();
}

int hrDefHandler::imageCount() const
{
    return dr->count();
}

bool hrDefHandler::jumpToImage(int imageNumber)
{
    return dr->jumpToImage(imageNumber);
}

bool hrDefHandler::jumpToNextImage()
{
    return dr->jumpToNextImage();
}

int hrDefHandler::loopCount() const
{
    return 1;
}

int hrDefHandler::nextImageDelay() const
{
    return 0;
}

bool hrDefHandler::canRead() const
{
    return canRead(device());
}

bool hrDefHandler::canRead(QIODevice *device)
{
    if (!device)
    {
        qWarning("hrDefHandler::canRead() empty device");
        return false;
    }

    if (!DefReader::canRead(device))
    {
        return false;
    }

    if (!device->isSequential())
    {
        qWarning("hrDefHandler::canRead() random access devices only");
        return false;
    }

    return true;
}

// todo: add Name and SubType
QVariant hrDefHandler::option(ImageOption option) const
{
    if (option == Size)
    {
        QSize imageSize;
        imageSize = QSize(dr->getWidth(), dr->getHeight());
        if ( imageSize.isValid() )
            return imageSize;
    }
    else
        if (option == Animation)
            return true;

    return QVariant();
}

void hrDefHandler::setOption(ImageOption option, const QVariant &value)
{
    qWarning("%s: not supported", Q_FUNC_INFO);
}

bool hrDefHandler::supportsOption(ImageOption option) const
{
    return option == Size
        || option == Animation;
}

QByteArray hrDefHandler::name() const
{
    return "def";
}