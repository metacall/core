#include <iostream>

#include "llvm/ExecutionEngine/ExecutionEngine.h"
//#include "/usr/include/llvm-11/llvm/ExecutionEngine/ExecutionEngine.h"
//#include "/usr/include/llvm-c-11/llvm-c/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetSelect.h"

// Optimizations
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Transforms/Scalar.h"

using namespace llvm;

using std::cout;
using std::endl;
using std::unique_ptr;

using llvm::ArrayRef;
using llvm::EngineBuilder;
using llvm::ExecutionEngine;
using llvm::Function;
using llvm::GenericValue;
using llvm::LLVMContext;
using llvm::Module;
using llvm::parseIRFile;
using llvm::SMDiagnostic;
using llvm::StringRef;

/*llvm::Function *createSumFunction(Module *module)
{
	// Builds the following function:
    
    //int sum(int a, int b) {
        //int sum1 = 1 + 1;
        //int sum2 = sum1 + a;
        //int result = sum2 + b;
        //return result;
    //}
    

	LLVMContext &context = module->getContext();
	IRBuilder<> builder(context);

	// Define function's signature
	std::vector<Type *> Integers(2, builder.getInt32Ty());
	auto *funcType = FunctionType::get(builder.getInt32Ty(), Integers, false);

	// create the function "sum" and bind it to the module with ExternalLinkage,
	// so we can retrieve it later
	auto *fooFunc = Function::Create(
		funcType, Function::ExternalLinkage, "sum", module);

	// Define the entry block and fill it with an appropriate code
	auto *entry = BasicBlock::Create(context, "entry", fooFunc);
	builder.SetInsertPoint(entry);

	// Add constant to itself, to visualize constant folding
	Value *constant = ConstantInt::get(builder.getInt32Ty(), 0x1);
	auto *sum1 = builder.CreateAdd(constant, constant, "sum1");

	// Retrieve arguments and proceed with further adding...
	auto args = fooFunc->arg_begin();
	Value *arg1 = &(*args);
	args = std::next(args);
	Value *arg2 = &(*args);
	auto *sum2 = builder.CreateAdd(sum1, arg1, "sum2");
	auto *result = builder.CreateAdd(sum2, arg2, "result");

	// ...and return
	builder.CreateRet(result);

	// Verify at the end
	verifyFunction(*fooFunc);
	return fooFunc;
};*/

int main(int argc, char *argv[])
{
	// Initilaze native target
	/*llvm::TargetOptions Opts;
	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();

	LLVMContext context;
	auto myModule = std::make_unique<Module>("My First JIT", context);
	auto *module = myModule.get();

	std::unique_ptr<llvm::RTDyldMemoryManager> MemMgr(new llvm::SectionMemoryManager());

	// Create JIT engine
	// We can somehow use
	llvm::EngineBuilder factory(std::move(myModule));
	factory.setEngineKind(llvm::EngineKind::JIT);
	factory.setTargetOptions(Opts);
	factory.setMCJITMemoryManager(std::move(MemMgr));
	auto executionEngine = std::unique_ptr<llvm::ExecutionEngine>(factory.create());
	module->setDataLayout(executionEngine->getDataLayout());

	// Create optimizations, not necessary, whole block can be ommited.
	// auto fpm = llvm::make_unique<legacy::FunctionPassManager>(module);
	// fpm->add(llvm::createBasicAAWrapperPass());
	// fpm->add(llvm::createPromoteMemoryToRegisterPass());
	// fpm->add(llvm::createInstructionCombiningPass());
	// fpm->add(llvm::createReassociatePass());
	// fpm->add(llvm::createNewGVNPass());
	// fpm->add(llvm::createCFGSimplificationPass());
	// fpm->doInitialization();

	auto *func = createSumFunction(module); // create function
	executionEngine->finalizeObject();		// compile the module
	module->print(llvm::errs(), nullptr);	// print the compiled code

	// Get raw pointer
	auto *raw_ptr = executionEngine->getPointerToFunction(func);
	auto *func_ptr = (int (*)(int, int))raw_ptr;

	// Execute
	int arg1 = 5;
	int arg2 = 7;
	int result = func_ptr(arg1, arg2);
	std::cout << arg1 << " + " << arg2 << " + 1 + 1 = " << result << std::endl;

	return 0;*/

	//LLVMContext &context = getGlobalContext();
	LLVMContext context;
	SMDiagnostic error;

	unique_ptr<Module> mod = parseIRFile(StringRef("input.ll"), error, context);
	//ExecutionEngine *executionEngine = EngineBuilder(std::move(mod)).setEngineKind(llvm::EngineKind::Interpreter).create();

	/*Module *mod = ParseIR("input.ll", error, context);
	
	if (!mod) 
	{
		error.print("input.ll", errs());
		return 1;
	}*/

	if (mod)
	{
		cout << "Module is loaded" << endl;
		//mod->dump();
	}

	//outs() << *mod;

	/*for (auto curFref = mod->getFunctionList().begin(), endFref = mod->getFunctionList().end(); curFref != endFref; ++curFref)
	{
		outs() << "found function: " << curFref->getName() << "\n";
	}*/

	for (Module::const_iterator i = mod->getFunctionList().begin(), e = mod->getFunctionList().end(); i != e; ++i)
	{
		if (!i->isDeclaration())
		{
			/*for (auto arg = i->arg_begin(); arg != i->arg_end(); ++arg)
			{
				if (auto *ci = dyn_cast<ConstantInt>(arg))
					errs() << ci->getValue() << "\n";

				errs() << *arg << "\n";
			}*/

			outs() << i->getName() << " has " << i->size() << " basicblock(s).\n";
		}
	}

	/*Function *add = executionEngine->FindFunctionNamed(StringRef("adder"));
	GenericValue param1, param2;
	param1.IntVal = 5;
	param2.IntVal = 2;
	GenericValue params[] = { param1, param2 };
	ArrayRef<GenericValue> args = ArrayRef<GenericValue>(params, 2);
	GenericValue result = executionEngine->runFunction(add, args);
	outs() << param1.IntVal << " + " << param2.IntVal << " = " << result.IntVal;*/

	return 0;
}