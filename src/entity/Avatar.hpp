// file   : src/entity/Avatar.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_ENTITY_AVATAR_HPP
#define THEGAME_ENTITY_AVATAR_HPP

#include "Cell.hpp"

class Avatar : public Cell {
public:
  using Avatars = std::vector<Avatar*>;

  Avatar(const asio::any_io_executor& executor, IEntityFactory& entityFactory, const config::Room& config, uint32_t id);

  void modifyMass(float value) override;

  void format(Buffer& buffer) override;

  void interact(Cell& cell) override;
  void interact(Avatar& avatar) override;
  void interact(Food& food) override;
  void interact(Bullet& bullet) override;
  void interact(Virus& virus) override;
  void interact(Phage& phage) override;
  void interact(Mother& mother) override;

  bool isAttractiveFor(const Avatar& avatar) override;

  float getMaxVelocity() const;
  bool isRecombined() const;

  void applyPointAttractionForce(const Vec2D& point, float forceRatio);

  void eject(const Vec2D& point);
  bool split(const Vec2D& point);
  void startRecombination();

  void deflate();
  void annihilate();
  void explode();

  void subscribeToAvatarCreation(void* tag, EventEmitter<Avatar*>::Handler&& handler);

private:
  EventEmitter<Avatar*> m_avatarCreationEmitter;
  TimePoint             m_fusionTime;
  float                 m_maxVelocity {0};
  mutable bool          m_recombined {false};
};

#endif /* THEGAME_ENTITY_AVATAR_HPP */
