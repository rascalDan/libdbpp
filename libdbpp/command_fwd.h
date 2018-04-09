#ifndef DB_COMMAND_FWD_H
#define DB_COMMAND_FWD_H

#include <string>
#include <map>
#include <memory>

namespace DB {
	typedef std::map<std::string, std::string> CommandOptionsMap;
	class CommandOptions;
	typedef std::shared_ptr<CommandOptions> CommandOptionsPtr;
	typedef std::shared_ptr<const CommandOptions> CommandOptionsCPtr;
	class Command;
	typedef std::shared_ptr<Command> CommandPtr;
	class ModifyCommand;
	typedef std::shared_ptr<ModifyCommand> ModifyCommandPtr;
	class SelectCommand;
	typedef std::shared_ptr<SelectCommand> SelectCommandPtr;
}


#endif

