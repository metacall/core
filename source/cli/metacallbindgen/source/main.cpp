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

struct type_map
{
	std::string metacall_id;   // e.g. "METACALL_INT"
	std::string unwrap;        // e.g. "metacall_value_to_int"
	std::string wrap;          // e.g. "metacall_value_create_int"
	std::string cast;          // e.g. "(int)" — applied to unwrap result before real call
};

struct arg_info
{
	std::string name;
	type_map t;
};

struct fn_info
{
	std::string name;
	CXType return_cx_type;
	type_map return_t;
	bool return_is_void = false;
	std::vector<arg_info> args;
	bool skip = false;
	std::string skip_reason;
};

std::string cx_string_to_std(CXString s)
{
	const char *c = clang_getCString(s);
	std::string result = c != nullptr ? c : "";
	clang_disposeString(s);
	return result;
}

// Map a CXType to metacall type helpers. Returns false if unsupported.
bool map_cxtype(CXType t, type_map &out)
{
	CXType canonical = clang_getCanonicalType(t);

	switch (canonical.kind)
	{
		case CXType_Void:
			out = { "METACALL_NULL", "", "metacall_value_create_null", "" };
			return true;
		case CXType_Bool:
			out = { "METACALL_BOOL", "metacall_value_to_bool", "metacall_value_create_bool", "" };
			return true;
		case CXType_Char_S:
		case CXType_Char_U:
		case CXType_SChar:
		case CXType_UChar:
			out = { "METACALL_CHAR", "metacall_value_to_char", "metacall_value_create_char", "" };
			return true;
		case CXType_Short:
		case CXType_UShort:
			out = { "METACALL_SHORT", "metacall_value_to_short", "metacall_value_create_short", "" };
			return true;
		case CXType_Int:
		case CXType_UInt:
			out = { "METACALL_INT", "metacall_value_to_int", "metacall_value_create_int", "" };
			return true;
		case CXType_Long:
		case CXType_ULong:
		case CXType_LongLong:
		case CXType_ULongLong:
			out = { "METACALL_LONG", "metacall_value_to_long", "metacall_value_create_long", "" };
			return true;
		case CXType_Float:
			out = { "METACALL_FLOAT", "metacall_value_to_float", "metacall_value_create_float", "" };
			return true;
		case CXType_Double:
			out = { "METACALL_DOUBLE", "metacall_value_to_double", "metacall_value_create_double", "" };
			return true;
		case CXType_Pointer:
		{
			CXType pointee = clang_getCanonicalType(clang_getPointeeType(canonical));
			// char * / const char * -> string
			if (pointee.kind == CXType_Char_S || pointee.kind == CXType_Char_U ||
				pointee.kind == CXType_SChar || pointee.kind == CXType_UChar)
			{
				out = { "METACALL_STRING", "metacall_value_to_string", "metacall_value_create_string", "" };
				return true;
			}
			// Any other pointer -> generic PTR
			out = { "METACALL_PTR", "metacall_value_to_ptr", "metacall_value_create_ptr", "(void *)" };
			return true;
		}
		default:
			return false;
	}
}

CXChildVisitResult visit(CXCursor cursor, CXCursor /*parent*/, CXClientData data)
{
	auto *fns = static_cast<std::vector<fn_info> *>(data);

	if (clang_getCursorKind(cursor) != CXCursor_FunctionDecl)
	{
		return CXChildVisit_Continue;
	}

	std::string name = cx_string_to_std(clang_getCursorSpelling(cursor));

	if (name.rfind("metacall", 0) != 0)
	{
		return CXChildVisit_Continue;
	}

	// Skip the API we prototype-registered by hand.
	if (name == "metacall_print_info")
	{
		return CXChildVisit_Continue;
	}

	fn_info fn;
	fn.name = name;

	if (clang_Cursor_isVariadic(cursor))
	{
		fn.skip = true;
		fn.skip_reason = "variadic";
	}

	CXType return_type = clang_getCursorResultType(cursor);
	fn.return_cx_type = return_type;
	fn.return_is_void = (clang_getCanonicalType(return_type).kind == CXType_Void);

	if (!fn.skip && !map_cxtype(return_type, fn.return_t))
	{
		fn.skip = true;
		fn.skip_reason = std::string("unsupported return type: ") +
			cx_string_to_std(clang_getTypeSpelling(return_type));
	}

	int num_args = clang_Cursor_getNumArguments(cursor);
	for (int i = 0; !fn.skip && i < num_args; ++i)
	{
		CXCursor arg_cursor = clang_Cursor_getArgument(cursor, i);
		arg_info arg;
		arg.name = cx_string_to_std(clang_getCursorSpelling(arg_cursor));
		if (arg.name.empty())
		{
			arg.name = "arg" + std::to_string(i);
		}
		CXType arg_type = clang_getCursorType(arg_cursor);
		if (!map_cxtype(arg_type, arg.t))
		{
			fn.skip = true;
			fn.skip_reason = std::string("unsupported arg[") + std::to_string(i) + "] type: " +
				cx_string_to_std(clang_getTypeSpelling(arg_type));
		}
		else
		{
			fn.args.push_back(arg);
		}
	}

	fns->push_back(fn);
	return CXChildVisit_Continue;
}

void emit_call_args(FILE *out, const fn_info &fn)
{
	for (size_t i = 0; i < fn.args.size(); ++i)
	{
		if (i > 0) std::fprintf(out, ", ");
		std::fprintf(out, "%s%s(args[%zu])",
			fn.args[i].t.cast.c_str(),
			fn.args[i].t.unwrap.c_str(),
			i);
	}
}

void emit_wrapper(FILE *out, const fn_info &fn)
{
	std::fprintf(out, "static void *bindgen_%s(size_t argc, void *args[], void *data)\n{\n", fn.name.c_str());
	std::fprintf(out, "\t(void)argc;\n\t(void)args;\n\t(void)data;\n\n");

	if (fn.return_is_void)
	{
		std::fprintf(out, "\t%s(", fn.name.c_str());
		emit_call_args(out, fn);
		std::fprintf(out, ");\n\n\treturn metacall_value_create_null();\n}\n\n");
		return;
	}

	std::string return_spelling = cx_string_to_std(clang_getTypeSpelling(fn.return_cx_type));

	std::fprintf(out, "\t%s ret = %s(", return_spelling.c_str(), fn.name.c_str());
	emit_call_args(out, fn);
	std::fprintf(out, ");\n");

	if (fn.return_t.metacall_id == "METACALL_STRING")
	{
		std::fprintf(out, "\treturn metacall_value_create_string(ret, ret == NULL ? 0 : strlen(ret));\n}\n\n");
	}
	else
	{
		std::fprintf(out, "\treturn %s(ret);\n}\n\n", fn.return_t.wrap.c_str());
	}
}

void emit_register_call(FILE *out, const fn_info &fn)
{
	std::fprintf(out, "\tif (metacall_register(\"%s\", &bindgen_%s, NULL, %s, %zu",
		fn.name.c_str(), fn.name.c_str(), fn.return_t.metacall_id.c_str(), fn.args.size());
	for (const auto &a : fn.args)
	{
		std::fprintf(out, ", %s", a.t.metacall_id.c_str());
	}
	std::fprintf(out, ") != 0) return 1;\n");
}

int emit_output(const std::vector<fn_info> &fns, const char *out_path)
{
	FILE *out = std::fopen(out_path, "w");
	if (!out)
	{
		std::fprintf(stderr, "metacallbindgen: cannot open %s\n", out_path);
		return 1;
	}

	std::fprintf(out,
		"/* Auto-generated by metacallbindgen. Do not edit by hand. */\n\n"
		"#include <metacall/metacall.h>\n"
		"#include <metacall/metacall_self_register.h>\n\n"
		"#include <string.h>\n\n"
		"static void *bindgen_metacall_print_info(size_t argc, void *args[], void *data)\n"
		"{\n"
		"\t(void)argc; (void)args; (void)data;\n"
		"\tconst char *info = metacall_print_info();\n"
		"\treturn metacall_value_create_string(info, info == NULL ? 0 : strlen(info));\n"
		"}\n\n");

	size_t emitted = 0, skipped = 0;
	for (const auto &fn : fns)
	{
		if (fn.skip)
		{
			std::fprintf(out, "/* skipped: %s — %s */\n", fn.name.c_str(), fn.skip_reason.c_str());
			++skipped;
			continue;
		}
		emit_wrapper(out, fn);
		++emitted;
	}

	std::fprintf(out,
		"\nint metacall_self_register_all(void)\n{\n"
		"\tif (metacall_register(\"metacall_print_info\", &bindgen_metacall_print_info, NULL, METACALL_STRING, 0) != 0) return 1;\n");
	for (const auto &fn : fns)
	{
		if (fn.skip) continue;
		emit_register_call(out, fn);
	}
	std::fprintf(out, "\treturn 0;\n}\n");

	std::fclose(out);
	std::fprintf(stderr, "metacallbindgen: emitted %zu wrappers, skipped %zu\n", emitted, skipped);
	return 0;
}

int usage(const char *argv0)
{
	std::fprintf(stderr,
		"usage: %s <path/to/metacall.h> -o <output.c> [-I<include> ...]\n",
		argv0);
	return 1;
}

} // namespace

int main(int argc, char *argv[])
{
	if (argc < 4) return usage(argv[0]);

	const char *header_path = argv[1];
	const char *out_path = nullptr;
	std::vector<const char *> clang_args;

	for (int i = 2; i < argc; ++i)
	{
		if (std::strcmp(argv[i], "-o") == 0 && i + 1 < argc)
		{
			out_path = argv[++i];
		}
		else
		{
			clang_args.push_back(argv[i]);
		}
	}

	if (!out_path) return usage(argv[0]);

	CXIndex index = clang_createIndex(0, 1);
	CXTranslationUnit unit = nullptr;
	CXErrorCode error = clang_parseTranslationUnit2(
		index, header_path,
		clang_args.data(), static_cast<int>(clang_args.size()),
		nullptr, 0,
		CXTranslationUnit_SkipFunctionBodies,
		&unit);

	if (error != CXError_Success || unit == nullptr)
	{
		std::fprintf(stderr, "metacallbindgen: failed to parse %s (clang error %d)\n", header_path, static_cast<int>(error));
		clang_disposeIndex(index);
		return 2;
	}

	std::vector<fn_info> fns;
	CXCursor cursor = clang_getTranslationUnitCursor(unit);
	clang_visitChildren(cursor, &visit, &fns);

	int rc = emit_output(fns, out_path);

	clang_disposeTranslationUnit(unit);
	clang_disposeIndex(index);
	return rc;
}
