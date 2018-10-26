#ifndef DRAWABLECELL_H
#define DRAWABLECELL_H

#include <QQuickPaintedItem>
#include <QImage>

class DrawableCell : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QString recognized READ recognized NOTIFY recognizedChanged)
public:
    DrawableCell();
    void paint(QPainter *painter) override;

signals:
    void recognizedChanged();

public slots:
    const QString &recognized() const { return m_recognized; }

protected:
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QPointF m_lastPoint;
    QString m_recognized;
    QImage m_drawn;
};

#endif // DRAWABLECELL_H
