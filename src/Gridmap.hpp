// file   : Gridmap.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef GRIDMAP_HPP
#define GRIDMAP_HPP

#include <stdint.h>
#include <functional>
#include <vector>
#include <set>

#include "geometry/AABB.hpp"

class Cell;

struct Sector {
  std::set<Cell*> cells;
  AABB box;
  uint16_t id {0};
};

class Gridmap {
public:
  typedef std::function<bool(Cell&)> Handler;

  void resize(uint32_t width, uint32_t height, uint8_t power);
  AABB clip(const AABB& box) const;
  Sector* getSector(const Vec2D& point) const;
  std::set<Sector*> getSectors(const AABB& box) const;

  void insert(Cell* cell);
  void erase(Cell* cell);
  void update(Cell* cell);
  void query(const AABB& box, const Handler& handler) const;
  size_t count(const AABB& box) const;

private:
  mutable std::vector<Sector> m_sectors;
  AABB      m_box;
  uint32_t  m_width {0};
  uint32_t  m_height {0};
  uint32_t  m_rowCount {0};
  uint32_t  m_colCount {0};
  uint32_t  m_queryStamp {0};
  uint8_t   m_power {0};
};

#endif /* GRIDMAP_HPP */
