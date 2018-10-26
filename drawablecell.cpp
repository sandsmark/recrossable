#include "drawablecell.h"
#include <QImage>
#include <QPainter>
#include "characterrecognizer.h"

DrawableCell::DrawableCell()
{
    setAcceptedMouseButtons(Qt::AllButtons);
    m_drawn = QImage(width(), height(), QImage::Format_Grayscale8);
    m_drawn.fill(Qt::white);
}


void DrawableCell::paint(QPainter *painter)
{
    painter->drawImage(boundingRect(), m_drawn);

    if (!m_recognized.isEmpty()) {
        QFont font;
        font.setPixelSize(height() / 2);
        painter->setFont(font);
        painter->fillRect(boundingRect(), QColor(255, 255, 255, 128));
        painter->drawText(boundingRect(), Qt::AlignHCenter | Qt::AlignBottom, m_recognized);
    }
}

void DrawableCell::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    Q_UNUSED(oldGeometry);
    m_drawn = QImage(newGeometry.size().toSize(), QImage::Format_Grayscale8);
    m_drawn.fill(Qt::white);
}


void DrawableCell::mousePressEvent(QMouseEvent *event)
{
    m_lastPoint = event->localPos();
}

void DrawableCell::mouseMoveEvent(QMouseEvent *event)
{
    m_recognized.clear();
    QPainter p(&m_drawn);
    p.setPen(QPen(Qt::black, 5));
    p.drawLine(m_lastPoint, event->localPos());
    m_lastPoint = event->localPos();
    update();
}

void DrawableCell::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if (m_drawn.format() != QImage::Format_Grayscale8) {
        qWarning() << "Invalid format" << m_drawn.format();
        m_drawn = m_drawn.convertToFormat(QImage::Format_Grayscale8);
    }

    int beginY = m_drawn.height();
    int beginX = m_drawn.width();
    int endY = 0;
    int endX = 0;

    for (int y=0; y<m_drawn.height(); y++) {
        const uchar *line = m_drawn.scanLine(y);
        for (int x=0; x<m_drawn.width(); x++) {
            if (line[x] < 128) {
                beginY = std::min(y, beginY);
                beginX = std::min(x, beginX);
                endY = std::max(endY, y);
                endX = std::max(endX, x);
            }
        }
    }

    beginX++;
    beginY++;
    endX--;
    endY--;
    qDebug() << beginX << beginY << endX << endY;
    if (endX == 0 || endY == 0) {
        qWarning() << "empty";
        return;
    }

    const int width = endX - beginX;
    const int centerX = beginX + width / 2;
    const int height = endY - beginY;
    const int centerY = beginY + height / 2;
    const int edge = std::max(width, height);
    if (beginX >= endX || beginY >= endY) {
        qWarning() << "invalid";
        return;
    }

    QRect cropRect(0, 0, edge, edge);
    cropRect.moveCenter(QPoint(centerX, centerY));
    if (cropRect.top() < 0) {
        cropRect.moveTop(0);
    }
    if (cropRect.left() < 0) {
        cropRect.moveLeft(0);
    }
    if (cropRect.right() >= m_drawn.width()) {
        cropRect.moveRight(m_drawn.width() - 1);
    }
    if (cropRect.bottom() >= m_drawn.height()) {
        cropRect.moveBottom(m_drawn.height() - 1);
    }
    QImage scaled = m_drawn.copy(cropRect).scaled(20, 20);
    scaled.invertPixels();
    scaled = scaled.copy(-4, -4, 28, 28).convertToFormat(QImage::Format_Grayscale8);


    int black = 0;
    for (int y=0; y<scaled.height(); y++) {
        const uchar *line = scaled.scanLine(y);
        for (int x=0; x<scaled.width(); x++) {
            if (line[x] > 128) {
                black++;
            }
        }
    }

    // Check if the user has tried to cover up something wrong
    if (black > (scaled.width() * scaled.height()) / 3) {
        qDebug() << "Clearing";
        m_drawn.fill(Qt::white);
        m_recognized.clear();
        emit recognizedChanged();
        update();
        return;
    }


    QString recognized = CharacterRecognizer::instance()->recognize(scaled);
    if (recognized != m_recognized) {
        m_recognized = recognized;
        emit recognizedChanged();
    }
}
