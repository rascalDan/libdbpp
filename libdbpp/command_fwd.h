#ifndef DB_COMMAND_FWD_H
#define DB_COMMAND_FWD_H

#include <map>
#include <memory>
#include <string>

namespace DB {
	using CommandOptionsMap = std::map<std::string, std::string, std::less<>>;
	class CommandOptions;
	using CommandOptionsPtr = std::shared_ptr<CommandOptions>;
	using CommandOptionsCPtr = std::shared_ptr<const CommandOptions>;
	class Command;
	using CommandPtr = std::shared_ptr<Command>;
	class ModifyCommand;
	using ModifyCommandPtr = std::shared_ptr<ModifyCommand>;
	class SelectCommand;
	using SelectCommandPtr = std::shared_ptr<SelectCommand>;
}

#endif
