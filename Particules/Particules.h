#pragma once

#include <QtWidgets/QWidget>
#include "../QuadTree/TQuadTree.h"
#include <list>
#include <queue>

class CRect
{
  float m_x1, m_y1, m_x2, m_y2;
public:
  CRect(float x1, float y1, float x2, float y2)
    : m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2)
  {}
  CRect(const CRect& r) = default;
  CRect& operator=(const CRect& r) = default;

  float x1() const { return m_x1; }
  float y1() const { return m_y1; }
  float x2() const { return m_x2; }
  float y2() const { return m_y2; }

  operator QRectF() const { return QRectF(m_x1, m_y1, m_x2 - m_x1, m_y2 - m_y1); }
  bool operator==(const CRect& r) const = default;
};

class Particules : public QWidget
{
  Q_OBJECT
  TQuadTree<CRect> m_QuadTree;
  std::list<CRect> m_List;
#ifdef _DEBUG
  const size_t m_nbParticules = 10000;
#else
  const size_t m_nbParticules = 100000;
#endif
  qreal m_scale = 1.0;
  QPointF m_centerLogic = { 0.5, 0.5 };
  QPointF m_centerPixel;
  QPointF m_translate;
  size_t m_frameTimeIndex = 0;
  size_t m_fps = 0;

  QPointF pixelToLogical(const QPoint& p) const;
  void computeTranslate();

  enum class EIterAlgorithm
  {
    list,
    quadTreeAllFunction,
    quadTreeFindInscribedFunction,
    quadTreeFindCollidingFunction,
    quadTreeIterators,
    quadTreeInscribedIterators,
    quadTreeCollidingIterators
  } m_IterAlgorithm;

public:
  Particules(QWidget* parent = nullptr);
  ~Particules();

protected:
  void paintEvent(QPaintEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;
  void timerEvent(QTimerEvent* event) override;

public slots:
  void onIterAlgorithmList() { m_IterAlgorithm = EIterAlgorithm::list; update(); }
  void onIterAlgorithmQuadTreeAllFunction() { m_IterAlgorithm = EIterAlgorithm::quadTreeAllFunction; update(); }
  void onIterAlgorithmQuadTreeFindInscribedFunction() { m_IterAlgorithm = EIterAlgorithm::quadTreeFindInscribedFunction; update(); }
  void onIterAlgorithmQuadTreeFindCollidingFunction() { m_IterAlgorithm = EIterAlgorithm::quadTreeFindCollidingFunction; update(); }
  void onIterAlgorithmQuadTreeIterators() { m_IterAlgorithm = EIterAlgorithm::quadTreeIterators; update(); }
  void onIterAlgorithmQuadTreeInscribedIterators() { m_IterAlgorithm = EIterAlgorithm::quadTreeInscribedIterators; update(); }
  void onIterAlgorithmQuadTreeCollidingIterators() { m_IterAlgorithm = EIterAlgorithm::quadTreeCollidingIterators; update(); }

};
