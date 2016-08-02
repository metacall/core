/*
 *	MetaCall++ Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *	Copyright (C) 2016 Federico Agustin Alvarez Bayon <loopzer@gmail.com>
 *
 *	High performance, type safe and exception safe object oriented
 *	front-end for MetaCall library.
 *
 */

#include <gmock/gmock.h>

#include <metacallpp/common.h>
#include <metacallpp/IContext.h>
#include <metacallpp/MetacallNotReadyException.h>
#include <metacallpp/IReturn.h>
#include <metacallpp/ScopeBase.h>

#include <metacallpp/Call.h>
#include <metacallpp/Metacall.h>
#include <metacallpp/MetaFunction.h>
#include <metacallpp/Context.h>

#include <iostream>

using namespace std;
using namespace Beast;

class metacallpp_test : public testing::Test
{
  public:
};

TEST(metacallpp_test, Simple)
{
	IContext * context = new Context();

	ScopeBase * scope = context->CreateScope(NULL);

	IMetacall * meta = scope->GetMetacall("divide");

	EXPECT_EQ((bool) true, (bool) meta->IsReady());

	delete scope;

	delete context;
}

TEST(metacallpp_test, MetaFunction)
{
	IContext * context = new Context();

	ScopeBase * scope = context->CreateScope(NULL);

	MetaFunction<int> * getDeviceName = scope->GetFunc<int>("GetDeviceName");

	int * result = getDeviceName->Invoke();

	cout << *result << endl;

	EXPECT_EQ((int) 0, (int) *result);

	delete result;

	delete scope;

	delete context;
}
