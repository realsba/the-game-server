#ifndef THEGAME_CHAT_MESSAGE_HPP
#define THEGAME_CHAT_MESSAGE_HPP

#include <cstdint>
#include <string>

struct ChatMessage {
  ChatMessage(uint32_t authorId, std::string author, std::string text)
    : text(std::move(text))
    , author(std::move(author))
    , authorId(authorId)
  {
  }

  std::string text;
  std::string author;
  uint32_t authorId;
};

#endif /* THEGAME_CHAT_MESSAGE_HPP */
