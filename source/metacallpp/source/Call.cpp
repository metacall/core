/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*	Copyright (C) 2016 Federico Agustin Alvarez Bayon <loopzer@gmail.com>
*
*	High performance, type safe and exception safe object oriented
*	front-end for MetaCall library.
*
*/

/* -- Headers -- */

#include <metacallpp/Call.h>

namespace Beast {

	Call::Call(IMetacall * meta)
	{
		this->meta = meta;
	}

	IParameterBuilder * Call::Parameters() {
		if (this->buildParams == NULL) {
			this->buildParams = new ParameterBuilder(&this->parameters);
		}

		return this->buildParams;
	}

	IReturn * Call::Invoke() {
		cout << "Call from " << this->meta->GetFunctionName() << endl;
		return new Return();
	}

	Call::~Call()
	{
		if (this->buildParams != NULL) {
			delete this->buildParams;
		}
	}
}
