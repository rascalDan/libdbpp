#include <iostream>
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <signal.h>
#include "mockDatabase.h"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

void emptyHandler(int) { }

int
main(int argc, char ** argv)
{
	po::options_description opts("LibDB++ Create Mock Database Options");
	std::vector<fs::path> scripts;
	std::string connector, master, database;
	bool drop;

	opts.add_options()
		("help,h", "Show this help message")
		("drop,X", po::bool_switch(&drop)->default_value(false), "Drop immediately after creation (don't wait for signal)")
		("script", po::value(&scripts)->required(), "Setup script path")
		("connector,c", po::value(&connector)->required(), "Database connector (e.g. postgresql, mysql, sqlite)")
		("master,m", po::value(&master), "Master connection string")
		("database,d", po::value(&database)->default_value("mock"), "Database name prefix to use")
		;

	po::positional_options_description p;
	p.add("script", -1);

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(opts).positional(p).run(), vm);

	if (vm.count("help")) {
		std::cout << opts << std::endl;
		return 1;
	}
	po::notify(vm);

	std::cout << "Creating database...";
	auto mock = DB::MockDatabaseFactory::createNew(connector, master, database, scripts);
	std::cout << " done." << std::endl;

	if (!drop) {
		std::cout << "Done. ctrl+c to tear down and exit." << std::endl;
		sigset(SIGINT, &emptyHandler);
		pause();

		std::cout << std::endl;
	}

	std::cout << "Tearing down database..." << std::endl;
	delete mock;
	std::cout << " done." << std::endl;
	return 0;
}

