#ifndef DB_MODIFYCOMMAND_H
#define DB_MODIFYCOMMAND_H

#include "command.h"

namespace DB {
	class ModifyCommand : public virtual Command {
		public:
			ModifyCommand(const std::string & sql);
			~ModifyCommand();
			// Execute the command and return effected row count
			virtual unsigned int	execute(bool allowNoChange = true) = 0;
	};
}

#endif

