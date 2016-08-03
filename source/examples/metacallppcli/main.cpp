
#include <metacallpp/common.h>

#include <metacallpp/IContext.h>
#include <metacallpp/MetacallNotReadyException.h>
#include <metacallpp/IReturn.h>
#include <metacallpp/ScopeBase.h>
#include <metacallpp/IMetacall.h>
#include <metacallpp/IMetacallConfig.h>
#include <metacallpp/IMetacallConfigBuilder.h>
#include <metacallpp/ScopeConfig.h>

#include <metacallpp/Call.h>
#include <metacallpp/Metacall.h>
#include <metacallpp/MetaFunction.h>
#include <metacallpp/Context.h>

#include <iostream>

using namespace std;
using namespace Beast;

void ConfigScope(IScopeConfig * scopeConfig) {

	IMetacallConfigBuilder * metacallConfig = scopeConfig->CreateMetacallConfig();

	metacallConfig->
		SetFunctionName("divide")->
		SetParameterType("String")->
		SetParameterType("Int")->
		SetParameterType("Long")->
		SetReturnType("int");

}

int main()
{
	IContext * context = new Context();

	IScopeConfig * scopeConfig = new ScopeConfig(MetacallInvokeTypes::Fixed);

	ConfigScope(scopeConfig);

	ScopeBase * scope = context->CreateScope(scopeConfig);

	IMetacall * meta = scope->GetMetacall("divide");

	IMetacall * meta2 = scope->GetMetacall("divide");

	cout << meta->IsReady() << endl;
	cout << meta2->IsReady() << endl;

	try
	{
		ICall * call = meta->Prepare();

		call->Parameters()->Param("String")->Param(1)->Param(1L);

		IReturn * ret = call->Invoke();

		delete ret;

		delete call;
	}
	catch (MetacallNotReadyException*) {
		cout << "MetacallNotReadyException" << endl;
	}

	MetaFunction<int> * getDeviceName = scope->GetFunc<int>("GetDeviceName");
	int *result = getDeviceName->Invoke();
	cout << *result << endl;
	delete result;

	delete scopeConfig;

	delete scope;

	delete context;

	getchar();

	return 0;
}

