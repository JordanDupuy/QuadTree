#include "Particules.h"
#include <random>
#include <QPainter>
#include <QWheelEvent>
#include <QDebug>

QPointF Particules::pixelToLogical(const QPoint& p) const
{
  const int size = std::min(width(), height());
  return QPointF((p - m_translate) / (size * 0.8 * m_scale));
}

void Particules::computeTranslate()
{
  int side = std::min(width(), height());
  m_translate = m_centerPixel - side * 0.8 * m_scale * m_centerLogic;
}

Particules::Particules(QWidget* parent)
  : QWidget(parent)
{
  std::random_device rd;
  std::default_random_engine dre(rd());
  std::uniform_real_distribution<float> urd(0.0f, 1.0f);

  for (size_t i = 0; i < m_nbParticules; ++i)
  {
    struct {
      float x1, y1, x2, y2;
    } r;
    float size = urd(dre) * 0.05f;
    r.x1 = urd(dre) * (1.0f - size);
    r.y1 = urd(dre) * (1.0f - size);
    r.x2 = r.x1 + size;
    r.y2 = r.y1 + size;
    m_QuadTree.insert(CRect(r.x1, r.y1, r.x2, r.y2));
    m_List.push_back(CRect(r.x1, r.y1, r.x2, r.y2));
  }

  m_centerPixel = { width() / 2.0f, height() / 2.0f };
  computeTranslate();

  startTimer(1000, Qt::TimerType::PreciseTimer);
}

Particules::~Particules()
{}

void Particules::paintEvent(QPaintEvent* event)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  painter.setBackgroundMode(Qt::OpaqueMode);
  painter.setBackground(QBrush(Qt::blue));
  painter.eraseRect(rect());

  int side = std::min(width(), height());

  painter.translate(m_translate);
  painter.scale(side * 0.8f * m_scale, side * 0.8f * m_scale);

  painter.setBrush(QBrush(Qt::black));
  painter.setPen(QPen(Qt::NoPen));
  painter.drawRect(QRectF(0.0f, 0.0f, 1.0f, 1.0f));

  QPointF ptTL = pixelToLogical({ 0, 0 });
  QPointF ptBR = pixelToLogical({ width(), height() });
  SLimits limits(ptTL.x(), ptTL.y(), ptBR.x(), ptBR.y());
  QPen pen(Qt::white, 0.0);
  painter.setPen(pen);
  painter.setBrush(QBrush(Qt::NoBrush));
  switch (m_IterAlgorithm)
  {
  case Particules::EIterAlgorithm::list:
    for (const auto& rect : m_List)
      painter.drawRect(rect);
    break;
  case Particules::EIterAlgorithm::quadTreeAllFunction:
  {
    auto rects = m_QuadTree.getAll();
    for (const auto& rect : rects)
      painter.drawRect(rect);
  }
    break;
  case Particules::EIterAlgorithm::quadTreeFindInscribedFunction:
  {
    auto rects = m_QuadTree.findInscribed(limits);
    for (const auto& rect : rects)
      painter.drawRect(rect);
  }
    break;
  case Particules::EIterAlgorithm::quadTreeFindCollidingFunction:
  {
    auto rects = m_QuadTree.findColliding(limits);
    for (const auto& rect : rects)
      painter.drawRect(rect);
  }
    break;
  case Particules::EIterAlgorithm::quadTreeIterators:
  {
    auto end = m_QuadTree.end();
    for (auto it = m_QuadTree.begin(); it != end; ++it)
      painter.drawRect(*it);
  }
    break;
  case Particules::EIterAlgorithm::quadTreeInscribedIterators:
  {
    auto end = m_QuadTree.end();
    for (auto it = m_QuadTree.beginInscribed(limits); it != end; ++it)
      painter.drawRect(*it);
  }
    break;
  case Particules::EIterAlgorithm::quadTreeCollidingIterators:
  {
    auto end = m_QuadTree.end();
    for (auto it = m_QuadTree.beginColliding(limits); it != end; ++it)
      painter.drawRect(*it);
  }
    break;
  default:
    break;
  }

  if (m_fps > 0)
  {
    painter.setTransform(QTransform());
    QRect rect(0, 0, width(), height());
    QRect boundingrect;
    QString text = QString("FPS: %1").arg(m_fps);
    painter.drawText(rect, Qt::AlignRight | Qt::AlignTop, text, &boundingrect);
    painter.fillRect(boundingrect, Qt::white);
    painter.setPen(QPen(Qt::yellow));
    painter.drawText(boundingrect, text);
  }

  ++m_frameTimeIndex;

  update();
}

void Particules::wheelEvent(QWheelEvent* event)
{
  m_centerLogic = pixelToLogical(event->position().toPoint());
  m_centerPixel = event->position();

  if (event->angleDelta().y() > 0)
    m_scale *= 1.1;
  else
    m_scale /= 1.1;

  computeTranslate();
  update();
}

void Particules::resizeEvent(QResizeEvent* event)
{
  if (event->oldSize() == QSize {-1, -1})
    m_centerPixel = { width() / 2.0f, height() / 2.0f };
  else
  {
    QSize diff = event->size() - event->oldSize();
    m_centerPixel += QPointF(diff.width() / 2.0f, diff.height() / 2.0f);
  }
  computeTranslate();
}

void Particules::timerEvent(QTimerEvent* event)
{
  Q_UNUSED(event);
  m_fps = m_frameTimeIndex;
  m_frameTimeIndex = 0;
}
