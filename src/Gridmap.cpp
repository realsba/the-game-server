// file   : Gridmap.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Gridmap.hpp"

#include "geometry/geometry.hpp"
#include "entity/Cell.hpp"

void Gridmap::resize(uint32_t width, uint32_t height, uint8_t power)
{
  m_box.b.x = width - 1;
  m_box.b.y = height - 1;
  m_width = width;
  m_height = height;
  m_power = power;
  uint32_t sectorSize = 1 << power;
  m_rowCount = (height + sectorSize - 1) >> power;
  m_colCount = (width + sectorSize - 1) >> power;
  m_sectors.resize(m_rowCount * m_colCount);
  int id = 0, x = 0, y = 0;
  for (Sector& sector : m_sectors) {
    sector.id = id++;
    sector.box = AABB(x, y, x + sectorSize - 1, y + sectorSize - 1);
    if (id % m_colCount) {
      x += sectorSize;
    } else {
      x = 0;
      y += sectorSize;
    }
  }
}

AABB Gridmap::clip(const AABB& box) const
{
  AABB aabb;
  aabb.a.x = std::max(box.a.x, m_box.a.x);
  aabb.a.y = std::max(box.a.y, m_box.a.y);
  aabb.b.x = std::min(box.b.x, m_box.b.x);
  aabb.b.y = std::min(box.b.y, m_box.b.y);
  return aabb;
}

Sector* Gridmap::getSector(const Vec2D& point) const
{
  if (!m_box.contain(point)) {
    return nullptr;
  }
  uint32_t row = static_cast<uint32_t>(point.y) >> m_power;
  uint32_t col = static_cast<uint32_t>(point.x) >> m_power;
  return &m_sectors.at(row * m_colCount + col);
}

std::set<Sector*> Gridmap::getSectors(const AABB& box) const
{
  std::set<Sector*> sectors;
  const AABB& aabb(clip(box));
  uint32_t rowStart = static_cast<uint32_t>(aabb.a.y) >> m_power;
  uint32_t rowEnd = static_cast<uint32_t>(aabb.b.y) >> m_power;
  uint32_t colStart = static_cast<uint32_t>(aabb.a.x) >> m_power;
  uint32_t colEnd = static_cast<uint32_t>(aabb.b.x) >> m_power;
  for (auto row = rowStart; row <= rowEnd; ++row) {
    for (auto col = colStart; col <= colEnd; ++col) {
      sectors.insert(&m_sectors.at(row * m_colCount + col));
    }
  }
  return sectors;
}

void Gridmap::insert(Cell* cell)
{
  if (cell->materialPoint) {
    Sector* sector = getSector(cell->position);
    if (sector) {
      sector->cells.insert(cell);
      cell->sectors.insert(sector);
    }
    return;
  }
  Vec2D delta(cell->radius, cell->radius);
  AABB aabb(cell->position - delta, cell->position + delta);
  aabb = clip(aabb);
  if (aabb.a.x > aabb.b.x || aabb.a.y > aabb.b.y) {
    return;
  }
  cell->leftTopSector = getSector(aabb.a);
  cell->rightBottomSector = getSector(aabb.b);
  uint32_t rowStart = static_cast<uint32_t>(aabb.a.y) >> m_power;
  uint32_t rowEnd = static_cast<uint32_t>(aabb.b.y) >> m_power;
  uint32_t colStart = static_cast<uint32_t>(aabb.a.x) >> m_power;
  uint32_t colEnd = static_cast<uint32_t>(aabb.b.x) >> m_power;
  uint32_t sectorSize = 1 << m_power;
  int32_t y = rowStart * sectorSize;
  for (auto row = rowStart; row <= rowEnd; ++row) {
    int32_t x = colStart * sectorSize;
    for (auto col = colStart; col <= colEnd; ++col) {
      AABB box(x, y, x + sectorSize, y + sectorSize);
      x += sectorSize;
      if (geometry::intersects(box, *cell)) {
        Sector* sector = &m_sectors.at(row * m_colCount + col);
        sector->cells.insert(cell);
        cell->sectors.insert(sector);
      }
    }
    y += sectorSize;
  }
}

void Gridmap::erase(Cell* cell)
{
  for (Sector* sector : cell->sectors) {
    sector->cells.erase(cell);
  }
  cell->sectors.clear();
}

void Gridmap::update(Cell* cell)
{
  if (cell->materialPoint) {
    Sector* current = *cell->sectors.begin();
    Sector* sector = getSector(cell->position);
    if (current != sector) {
      current->cells.erase(cell);
      cell->sectors.clear();
      sector->cells.insert(cell);
      cell->sectors.insert(sector);
    }
    return;
  }
  Vec2D delta(cell->radius, cell->radius);
  AABB aabb(cell->position - delta, cell->position + delta);
  aabb = clip(aabb);
  if (aabb.a.x > aabb.b.x || aabb.a.y > aabb.b.y) {
    return;
  }
  Sector* leftTopSector = getSector(aabb.a);
  Sector* rightBottomSector = getSector(aabb.b);
  if (cell->leftTopSector == leftTopSector && cell->rightBottomSector == rightBottomSector) {
    return;
  }
  cell->leftTopSector = leftTopSector;
  cell->rightBottomSector = rightBottomSector;
  uint32_t rowStart = static_cast<uint32_t>(aabb.a.y) >> m_power;
  uint32_t rowEnd = static_cast<uint32_t>(aabb.b.y) >> m_power;
  uint32_t colStart = static_cast<uint32_t>(aabb.a.x) >> m_power;
  uint32_t colEnd = static_cast<uint32_t>(aabb.b.x) >> m_power;
  uint32_t sectorSize = 1 << m_power;
  int32_t y = rowStart * sectorSize;
  for (Sector* sector : cell->sectors) {
    sector->cells.erase(cell);
  }
  cell->sectors.clear();
  for (auto row = rowStart; row <= rowEnd; ++row) {
    int32_t x = colStart * sectorSize;
    for (auto col = colStart; col <= colEnd; ++col) {
      AABB box(x, y, x + sectorSize, y + sectorSize);
      x += sectorSize;
      if (geometry::intersects(box, *cell)) {
        Sector* sector = &m_sectors.at(row * m_colCount + col);
        sector->cells.insert(cell);
        cell->sectors.insert(sector);
      }
    }
    y += sectorSize;
  }
}

void Gridmap::query(const AABB& box, const Handler& handler) const
{
  AABB aabb(clip(box));
  if (aabb.a.x > aabb.b.x || aabb.a.y > aabb.b.y) {
    return;
  }
  uint32_t rowStart = static_cast<uint32_t>(aabb.a.y) >> m_power;
  uint32_t rowEnd = static_cast<uint32_t>(aabb.b.y) >> m_power;
  uint32_t colStart = static_cast<uint32_t>(aabb.a.x) >> m_power;
  uint32_t colEnd = static_cast<uint32_t>(aabb.b.x) >> m_power;
  for (auto row = rowStart; row <= rowEnd; ++row) {
    for (auto col = colStart; col <= colEnd; ++col) {
      const Sector& sector = m_sectors.at(row * m_colCount + col);
      for (Cell* obj : sector.cells) {
        if (obj->intersects(aabb)) {
          if (!handler(*obj)) {
            return;
          }
        }
      }
    }
  }
}

size_t Gridmap::count(const AABB& box) const
{
  const AABB& aabb(clip(box));
  if (aabb.a.x > aabb.b.x || aabb.a.y > aabb.b.y) {
    return 0;
  }
  uint32_t rowStart = static_cast<uint32_t>(aabb.a.y) >> m_power;
  uint32_t rowEnd = static_cast<uint32_t>(aabb.b.y) >> m_power;
  uint32_t colStart = static_cast<uint32_t>(aabb.a.x) >> m_power;
  uint32_t colEnd = static_cast<uint32_t>(aabb.b.x) >> m_power;
  size_t cnt = 0;
  for (auto row = rowStart; row <= rowEnd; ++row) {
    for (auto col = colStart; col <= colEnd; ++col) {
      const Sector& sector = m_sectors.at(row * m_colCount + col);
      for (Cell* obj : sector.cells) {
        if (!obj->zombie && geometry::intersects(aabb, obj->position)) {
          ++cnt;
        }
      }
    }
  }
  return cnt;
}
