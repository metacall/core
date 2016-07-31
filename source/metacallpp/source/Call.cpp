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
