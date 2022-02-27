#pragma once

#include <QMetaType>
#include <stdint.h>
#include <vector>

enum class CommandType : uint8_t {
  Pause = 0x01,
  Time = 0x02,
  Resume = 0x03,
  State = 0x04,
  End
};

struct SuitCommand {
  CommandType type = CommandType::End;
  std::vector<uint8_t> data;

  size_t size() const;

  explicit SuitCommand(CommandType type, std::vector<uint8_t> data)
      : type(type), data(data) {}

  explicit SuitCommand(CommandType type) : type(type) {}

  explicit SuitCommand() {}
};

Q_DECLARE_METATYPE(SuitCommand);