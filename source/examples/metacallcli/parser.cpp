/*
 *	MetaCall Command Line Interface by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A command line interface example as metacall wrapper.
 *
 */

/* -- Headers -- */

#include "parser.h"

/* -- Namespace Declarations -- */

using namespace metacallcli;

/* -- Methods -- */

parser::parser(const tokenizer::iterator & it) : it(it)
{

}

parser::~parser()
{

}
