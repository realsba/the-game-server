// file   : RoomConfig.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_ROOM_CONFIG_HPP
#define THEGAME_ROOM_CONFIG_HPP

#include <cstdint>
#include <chrono>

class RoomConfig {
public:
  float eps {0.01};

  std::chrono::milliseconds updateInterval;
  std::chrono::milliseconds tickInterval;
  uint32_t  simulationIterations {0};
  uint32_t  spawnPosTryCount {0};
  uint32_t  checkPlayers {0};
  uint32_t  updateLeaderboard {0};
  uint32_t  destroyOutdatedCells {0};
  uint32_t  checkMothers {0};
  uint32_t  mothersProduce {0};

  uint32_t  viewportBase {0};           // коротша сторона (висота)
  float     viewportBuffer {0};         // запас в кожну сторону
  float     aspectRatio {0};

  uint32_t  width {0};
  uint32_t  height {0};
  uint32_t  maxMass {0};
  uint32_t  maxPlayers {0};
  uint32_t  maxRadius {0};
  uint32_t  leaderboardVisibleItems {0};
  float     scaleRatio {0};
  uint32_t  explodeImpulse {0};

  uint32_t  playerMaxCells {0};
  uint32_t  playerDeflationTime {0};
  float     playerDeflationRatio {0};
  uint32_t  playerAnnihilationTime {0};
  float     playerForceRatio {0};

  uint32_t  botAmount {0};
  uint32_t  botStartMass {0};
  float     botForceCornerRatio {0};
  float     botForceFoodRatio {0};
  float     botForceHungerRatio {0};
  float     botForceDangerRatio {0};
  float     botForceStarRatio {0};

  float     resistanceRatio {0};        // коефіцієнт лобового опору
  float     elasticityRatio {0};        // коефіцієнт відштовхування

  uint32_t  cellMinMass {0};
  float     cellRadiusRatio {0};

  uint32_t  avatarStartMass {0};
  uint32_t  avatarMinSpeed {0};
  uint32_t  avatarMaxSpeed {0};
  uint32_t  avatarExplodeMinMass {0};
  uint32_t  avatarExplodeParts {0};
  uint32_t  avatarSplitMinMass {0};
  uint32_t  avatarSplitImpulse {0};
  uint32_t  avatarEjectMinMass {0};
  uint32_t  avatarEjectImpulse {0};
  uint32_t  avatarRecombineTime {0};
  uint32_t  avatarEjectMass {0};
  uint32_t  avatarEjectMassLoss {0};

  uint32_t  foodStartAmount {0};
  uint32_t  foodMaxAmount {0};
  uint32_t  foodMass {0};
  uint32_t  foodRadius {0};
  uint32_t  foodMinImpulse {0};
  uint32_t  foodMaxImpulse {0};
  float     foodResistanceRatio {0};

  float     massImpulseRatio {0};

  uint32_t  virusStartMass {0};
  uint32_t  virusStartAmount {0};
  uint32_t  virusMaxAmount {0};
  uint32_t  virusLifeTime {0};
  uint32_t  virusColor {0};

  uint32_t  phageStartMass {0};
  uint32_t  phageStartAmount {0};
  uint32_t  phageMaxAmount {0};
  uint32_t  phageLifeTime {0};
  uint32_t  phageColor {0};

  uint32_t  motherStartMass {0};
  uint32_t  motherStartAmount {0};
  uint32_t  motherMaxAmount {0};
  uint32_t  motherLifeTime {0};
  uint32_t  motherExplodeMass {0};
  uint32_t  motherColor {0};
  uint32_t  motherCheckRadius {0};

  float     spawnFoodMass {0};
  float     spawnVirusMass {0};
  float     spawnPhageMass {0};
  float     spawnMotherMass {0};
};

#endif /* THEGAME_ROOM_CONFIG_HPP */
