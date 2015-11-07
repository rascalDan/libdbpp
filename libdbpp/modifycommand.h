#ifndef DB_MODIFYCOMMAND_H
#define DB_MODIFYCOMMAND_H

#include "command.h"
#include <visibility.h>
#include <boost/shared_ptr.hpp>

namespace DB {
	/// Presents a command not expected to return any data.
	class DLL_PUBLIC ModifyCommand : public virtual Command {
		public:
			/// Creates a new command from the given SQL.
			ModifyCommand(const std::string & sql);
			~ModifyCommand();

			/// Execute the command and return effected row count
			virtual unsigned int	execute(bool allowNoChange = true) = 0;
	};
	typedef boost::shared_ptr<ModifyCommand> ModifyCommandPtr;
}

#endif

