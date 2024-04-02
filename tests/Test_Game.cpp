// file   : tests/geometry/Test_Vec2D.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include <catch2/catch_test_macros.hpp>

#include "../src/Room.hpp"

#include <chrono>

using namespace std::chrono_literals;

config::Room getDefaultRoomConfig()
{
  config::Room config;

  config.numThreads = 4;
  config.spawnPosTryCount = 10;

  config.updateInterval = 20ms;
  config.syncInterval = 60ms;
  config.checkExpirableCellsInterval = 3s;

  config.viewportBase = 743;
  config.viewportBuffer = 0.1;
  config.aspectRatio = 1.77778;

  config.width = 6144;
  config.height = 6144;
  config.maxMass = 50000;
  config.maxPlayers = 50;
  config.maxRadius = 100;
  config.scaleRatio = 0.75;
  config.explodeVelocity = 500;

  config.botNames = {"Nebula", "Solaris", "Celestia", "Quasar", "Zenith", "Lunar", "Stardust", "Nova", "Galaxia", "Cosmos"};

  config.resistanceRatio = 750.0;
  config.elasticityRatio = 30.0;

  config.cellMinMass = 35;
  config.cellRadiusRatio = 6.0;

  config.leaderboard.limit = 20;
  config.leaderboard.updateInterval = 1s;

  config.player.mass = 250;
  config.player.maxAvatars = 16;
  config.player.deflationThreshold = 30s;
  config.player.deflationInterval = 500ms;
  config.player.deflationRatio = 0.1;
  config.player.annihilationThreshold = 1min;
  config.player.pointerForceRatio = 2.5;

  config.bot.mass = 500;
  config.bot.respawnDelay = 5s;

  config.avatar.minVelocity = 200;
  config.avatar.maxVelocity = 600;
  config.avatar.explosionMinMass = 35;
  config.avatar.explosionParts = 5;
  config.avatar.splitMinMass = 100;
  config.avatar.splitVelocity = 600;
  config.avatar.ejectionMinMass = 82;
  config.avatar.ejectionVelocity = 550;
  config.avatar.ejectionMass = 50;
  config.avatar.ejectionMassLoss = 100;
  config.avatar.recombinationDuration = 8s;

  config.food.mass = 5;
  config.food.radius = 8;
  config.food.quantity = 2000;
  config.food.maxQuantity = 5000;
  config.food.minVelocity = 100;
  config.food.maxVelocity = 130;
  config.food.resistanceRatio = 40.0;
  config.food.minColorIndex = 0;
  config.food.maxColorIndex = 15;

  config.virus.mass = 165;
  config.virus.quantity = 10;
  config.virus.maxQuantity = 20;
  config.virus.lifeTime = 5min;
  config.virus.color = 13;

  config.phage.mass = 165;
  config.phage.quantity = 10;
  config.phage.maxQuantity = 10;
  config.phage.lifeTime = 5min;
  config.phage.color = 14;

  config.mother.mass = 240;
  config.mother.maxMass = 1200;
  config.mother.quantity = 10;
  config.mother.maxQuantity = 20;
  config.mother.lifeTime = 10min;
  config.mother.color = 12;
  config.mother.checkRadius = 60;
  config.mother.baseFoodProduction = 5.0;
  config.mother.nearbyFoodLimit = 100;
  config.mother.foodCheckInterval = 10s;
  config.mother.foodGenerationInterval = 1s;

  config.generator.food.interval = 1s;
  config.generator.food.quantity = 3;

  config.generator.virus.interval = 7s;
  config.generator.virus.quantity = 1;

  config.generator.phage.interval = 7s;
  config.generator.phage.quantity = 1;

  config.generator.mother.interval = 10s;
  config.generator.mother.quantity = 1;

  return config;
}


TEST_CASE("Game: test1", "[Game]")
{
  asio::io_context ioContext;
  auto room = std::make_unique<Room>(asio::make_strand(ioContext), 1);
  auto config = getDefaultRoomConfig();
  room->init(config);
  room->start();

  //ioContext.run_for(250ms);
}

