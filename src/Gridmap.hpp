// file   : src/Gridmap.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_GRIDMAP_HPP
#define THEGAME_GRIDMAP_HPP

#include "geometry/AABB.hpp"

#include <cstdint>
#include <functional>
#include <set>
#include <unordered_set>
#include <vector>

class Cell;

struct Sector {
  std::unordered_set<Cell*> cells;
  AABB box;
};

class Gridmap {
public:
  using Handler = std::function<bool(Cell&)>;

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
  uint8_t   m_power {0};
};

#endif /* THEGAME_GRIDMAP_HPP */
