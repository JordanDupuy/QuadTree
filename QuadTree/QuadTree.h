#pragma once
#include "TQuadTree.h"

class Rectangle {
  float m_x1, m_y1, m_x2, m_y2;
public:
  Rectangle(float x1 = 0.0f, float y1 = 0.0f, float x2 = 1.0f, float y2 = 1.0f)
    : m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2)
  {
  }

  bool operator==(const Rectangle& other) const = default;

  std::partial_ordering operator<=>(const Rectangle& other) const = default;

  float x1() const { return m_x1; }
  float y1() const { return m_y1; }
  float x2() const { return m_x2; }
  float y2() const { return m_y2; }
};


using QuadTree = TQuadTree<Rectangle>; // QuadTree is a TQuadTree of Rectangle
static_assert(std::input_iterator<QuadTree::iterator>);
