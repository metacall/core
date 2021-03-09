/*
 *	Loader Library by Parra Studios
 *	A plugin for loading c code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

#include <c_loader/c_loader_impl.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>

#include <reflect/type.h>
#include <reflect/function.h>
#include <reflect/scope.h>
#include <reflect/context.h>

#include <log/log.h>

#include <new>

#include <ffi.h>

#include <clang/Driver/Options.h>
#include <clang/AST/AST.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/ASTConsumers.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Rewrite/Core/Rewriter.h>


#define C_LOADER_FILE_MAX_SIZE	((size_t)(2 * 1024 * 1024)) /* 2 MB */

typedef struct loader_impl_c_type
{
	void * todo;

} * loader_impl_c;

typedef struct loader_impl_c_handle_type
{
/*
	char * buffer;
	size_t buffer_size;
*/
	void * todo;

} * loader_impl_c_handle;

typedef struct loader_impl_c_function_type
{
	void * todo;

} * loader_impl_c_function;

int function_c_interface_create(function func, function_impl impl)
{
	(void)func;
	(void)impl;

	return 0;
}

function_return function_c_interface_invoke(function func, function_impl impl, function_args args, size_t size)
{
	/* TODO */

	(void)func;
	(void)impl;
	(void)args;
	(void)size;

	return NULL;
}

function_return function_c_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void * context)
{
	/* TODO */

	(void)func;
	(void)impl;
	(void)args;
	(void)size;
	(void)resolve_callback;
	(void)reject_callback;
	(void)context;

	return NULL;
}

void function_c_interface_destroy(function func, function_impl impl)
{
	loader_impl_c_function c_function = static_cast<loader_impl_c_function>(impl);

	(void)func;

	if (c_function != NULL)
	{
		delete c_function;
	}
}

function_interface function_c_singleton()
{
	static struct function_interface_type c_interface =
	{
		&function_c_interface_create,
		&function_c_interface_invoke,
		&function_c_interface_await,
		&function_c_interface_destroy
	};

	return &c_interface;
}

loader_impl_data c_loader_impl_initialize(loader_impl impl, configuration config)
{
	loader_impl_c c_impl;

	(void)impl;
	(void)config;

	c_impl = new loader_impl_c_type();

	if (c_impl != nullptr)
	{
		int argc = 1;

		const char * argv[] = { "c_loader" };

		llvm::cl::OptionCategory category(argv[0]);

		clang::tooling::CommonOptionsParser op(argc, argv, category);
/*
		clang::tooling::ClangTool tooling(op.getCompilations(), op.getSourcePathList());
*/
		/* Register initialization */
		loader_initialization_register(impl);

		return static_cast<loader_impl_data>(c_impl);
	}

	return NULL;
}

int c_loader_impl_execution_path(loader_impl impl, const loader_naming_path path)
{
	(void)impl;
	(void)path;

	return 0;
}

loader_handle c_loader_impl_load(loader_impl impl, const loader_naming_path path, loader_naming_name name)
{
	loader_impl_c_handle c_handle = new loader_impl_c_handle_type();

	(void)impl;

	if (c_handle != nullptr)
	{
		return static_cast<loader_handle>(c_handle);

		/*
		FILE * file;

		file = fopen(path, "r");

		if (file != NULL)
		{
			size_t size = 0;

			if (fseek(file, 0, SEEK_END) == 0)
			{
				long int tell_size = ftell(file);

				if (tell_size > 0)
				{
					size = (size_t)tell_size;
				}
			}

			if (size > 0 && size < C_LOADER_FILE_MAX_SIZE && fseek(file, 0, SEEK_SET) == 0)
			{
				char * buffer = malloc(sizeof(char) * size);

				if (buffer != NULL)
				{
					size_t result = fread(buffer, sizeof(char), size, file);

					if (result == size)
					{
						fclose(file);

						c_handle->buffer = buffer;
						c_handle->buffer_size = size;

						log_write("metacall", LOG_LEVEL_DEBUG, "C module <%s> correctly loaded", name);

						return (loader_handle)c_handle;
					}

					free(buffer);
				}
			}

			fclose(file);
		}

		free(c_handle);
		*/
	}

	return NULL;
}

int c_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	loader_impl_c_handle c_handle = static_cast<loader_impl_c_handle>(handle);

	(void)impl;

	if (c_handle != NULL)
	{
		/*
		if (c_handle->buffer != NULL)
		{
			free(c_handle->buffer);

			c_handle->buffer = NULL;
		}

		c_handle->buffer_size = 0;
		*/

		delete c_handle;

		return 0;
	}

	return 1;
}

int c_loader_impl_discover_func(loader_impl impl, loader_handle handle, context ctx, function f)
{
	(void)impl;
	(void)handle;
	(void)ctx;
	(void)f;

	return 0;
}

int c_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_c_handle c_handle = static_cast<loader_impl_c_handle>(handle);

	(void)impl;
	(void)ctx;

	if (c_handle != NULL)
	{


		return 0;
	}

	return 1;
}

int c_loader_impl_destroy(loader_impl impl)
{
	loader_impl_c c_impl = static_cast<loader_impl_c>(loader_impl_get(impl));

	if (c_impl != NULL)
	{
		/* Destroy children loaders */
		loader_unload_children();

		delete c_impl;

		return 0;
	}

	return 1;
}

/*
using namespace std;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

Rewriter rewriter;
int numFunctions = 0;


class ExampleVisitor : public RecursiveASTVisitor<ExampleVisitor> {
private:
    ASTContext *astContext; // used for getting additional AST info

public:
    explicit ExampleVisitor(CompilerInstance *CI) 
      : astContext(&(CI->getASTContext())) // initialize private members
    {
        rewriter.setSourceMgr(astContext->getSourceManager(), astContext->getLangOpts());
    }

    virtual bool VisitFunctionDecl(FunctionDecl *func) {
        numFunctions++;
        string funcName = func->getNameInfo().getName().getAsString();
        if (funcName == "do_math") {
            rewriter.ReplaceText(func->getLocation(), funcName.length(), "add5");
            errs() << "** Rewrote function def: " << funcName << "\n";
        }    
        return true;
    }

    virtual bool VisitStmt(Stmt *st) {
        if (ReturnStmt *ret = dyn_cast<ReturnStmt>(st)) {
            rewriter.ReplaceText(ret->getRetValue()->getLocStart(), 6, "val");
            errs() << "** Rewrote ReturnStmt\n";
        }        
        if (CallExpr *call = dyn_cast<CallExpr>(st)) {
            rewriter.ReplaceText(call->getLocStart(), 7, "add5");
            errs() << "** Rewrote function call\n";
        }
        return true;
    }
};



class ExampleASTConsumer : public ASTConsumer {
private:
    ExampleVisitor *visitor; // doesn't have to be private

public:
    // override the constructor in order to pass CI
    explicit ExampleASTConsumer(CompilerInstance *CI)
        : visitor(new ExampleVisitor(CI)) // initialize the visitor
    { }

    // override this to call our ExampleVisitor on the entire source file
    virtual void HandleTranslationUnit(ASTContext &Context) {
        // we can use ASTContext to get the TranslationUnitDecl, which is
        //     a single Decl that collectively represents the entire source file 
        visitor->TraverseDecl(Context.getTranslationUnitDecl());
    }

};



class ExampleFrontendAction : public ASTFrontendAction {
public:
    virtual ASTConsumer *CreateASTConsumer(CompilerInstance &CI, StringRef file) {
        return new ExampleASTConsumer(&CI); // pass CI pointer to ASTConsumer
    }
};



int main(int argc, const char **argv) {
    // parse the command-line args passed to your code
    CommonOptionsParser op(argc, argv);        
    // create a new Clang Tool instance (a LibTooling environment)
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());

    // run the Clang Tool, creating a new FrontendAction (explained below)
    int result = Tool.run(newFrontendActionFactory<ExampleFrontendAction>());

    errs() << "\nFound " << numFunctions << " functions.\n\n";
    // print out the rewritten source code ("rewriter" is a global var.)
    rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID()).write(errs());
    return result;
}
*/

