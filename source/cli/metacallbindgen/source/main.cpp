/*
 *	MetaCall Bindgen — build-time tool that parses metacall.h and
 *	generates C wrappers that self-register every metacall_* API via
 *	metacall_register. The wrappers get compiled into libmetacall so
 *	loaded scripts can call metacall_* APIs directly, no c_loader.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License").
 */

#include <clang-c/Index.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace {

struct visitor_state
{
	std::vector<std::string> function_names;
};

std::string cx_string_to_std(CXString s)
{
	const char *c = clang_getCString(s);
	std::string result = c != nullptr ? c : "";
	clang_disposeString(s);
	return result;
}

CXChildVisitResult visit(CXCursor cursor, CXCursor /*parent*/, CXClientData data)
{
	auto *state = static_cast<visitor_state *>(data);

	if (clang_getCursorKind(cursor) != CXCursor_FunctionDecl)
	{
		return CXChildVisit_Continue;
	}

	std::string name = cx_string_to_std(clang_getCursorSpelling(cursor));

	if (name.rfind("metacall_", 0) != 0 && name != "metacallv" && name.rfind("metacall", 0) != 0)
	{
		return CXChildVisit_Continue;
	}

	state->function_names.push_back(name);

	return CXChildVisit_Continue;
}

int usage(const char *argv0)
{
	std::fprintf(stderr,
		"usage: %s <path/to/metacall.h> [-I<include> ...]\n",
		argv0);
	return 1;
}

} // namespace

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		return usage(argv[0]);
	}

	const char *header_path = argv[1];

	std::vector<const char *> clang_args;
	for (int i = 2; i < argc; ++i)
	{
		clang_args.push_back(argv[i]);
	}

	CXIndex index = clang_createIndex(0, 1);

	CXTranslationUnit unit = nullptr;
	CXErrorCode error = clang_parseTranslationUnit2(
		index,
		header_path,
		clang_args.data(),
		static_cast<int>(clang_args.size()),
		nullptr,
		0,
		CXTranslationUnit_SkipFunctionBodies,
		&unit);

	if (error != CXError_Success || unit == nullptr)
	{
		std::fprintf(stderr, "metacallbindgen: failed to parse %s (clang error %d)\n", header_path, static_cast<int>(error));
		clang_disposeIndex(index);
		return 2;
	}

	visitor_state state;
	CXCursor cursor = clang_getTranslationUnitCursor(unit);
	clang_visitChildren(cursor, &visit, &state);

	std::printf("Discovered %zu metacall_* function declarations:\n", state.function_names.size());
	for (const auto &name : state.function_names)
	{
		std::printf("  %s\n", name.c_str());
	}

	clang_disposeTranslationUnit(unit);
	clang_disposeIndex(index);

	return 0;
}
