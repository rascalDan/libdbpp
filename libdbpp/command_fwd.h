#ifndef DB_COMMAND_FWD_H
#define DB_COMMAND_FWD_H

#include <string>
#include <map>
#include <boost/shared_ptr.hpp>

namespace DB {
	typedef std::map<std::string, std::string> CommandOptionsMap;
	class CommandOptions;
	class Command;
	typedef boost::shared_ptr<Command> CommandPtr;
	class ModifyCommand;
	typedef boost::shared_ptr<ModifyCommand> ModifyCommandPtr;
	class SelectCommand;
	typedef boost::shared_ptr<SelectCommand> SelectCommandPtr;
}


#endif

