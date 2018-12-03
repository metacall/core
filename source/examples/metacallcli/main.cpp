/*
 *	MetaCall Command Line Interface by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A command line interface example as metacall wrapper.
 *
 */

/* -- Headers -- */

#include "application.hpp"

/* -- Namespace Declarations -- */

using namespace metacallcli;

/* -- Methods -- */

int main(int argc, char * argv[])
{
	application app(argc, argv);

	app.run();

	return 0;
}
