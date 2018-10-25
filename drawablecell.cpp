#include "drawablecell.h"
#include <QImage>
#include <QPainter>

DrawableCell::DrawableCell()
{
    setAcceptedMouseButtons(Qt::AllButtons);
}


void DrawableCell::paint(QPainter *painter)
{
    if (!m_recognized.isEmpty()) {
        painter->drawText(boundingRect(), m_recognized);
        return;
    }

    painter->fillRect(boundingRect(), Qt::red);
    painter->drawImage(boundingRect(), m_drawn);
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
    update();
    const QImage scaled = m_drawn.scaled(28, 28, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    int black = 0;
    for (int y=0; y<scaled.height(); y++) {
        const uchar *line = scaled.scanLine(y);
        for (int x=0; x<scaled.width(); x++) {
            if (line[x] < 128) {
                black++;
            }
        }
    }

    if (black > (scaled.width() * scaled.height()) / 3) {
        m_drawn.fill(Qt::white);
    } else {
        m_drawn = scaled.scaled(boundingRect().size().toSize());
    }
}
