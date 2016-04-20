#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifdef PRINT_LOG
#define print_log printf
#else
#define print_log(text, value) (void)value
#endif

#define ARG_LIST_SIZE 5

enum arg_type_id
{
	TYPE_CHAR,
	TYPE_INT,
	TYPE_PTR,

	TYPE_COUNT
};

typedef struct arg_type
{
	enum arg_type_id id;
	void * data;
	size_t size;
} * arg;

void arg_type_define(arg a, enum arg_type_id id, void * data, size_t size)
{
	a->id = id;
	a->data = data;
	a->size = size;
}

typedef struct func_type
{
	struct arg_type arg_list[ARG_LIST_SIZE];
	size_t size;
} * func;

typedef void (*func_arg_callback)(func f, va_list va);

void func_arg_cb_char(func f, va_list va)
{
	char c = (char)va_arg(va, int); // because of type promotion

	print_log("char: %c\n", c);
}

void func_arg_cb_int(func f, va_list va)
{
	int i = va_arg(va, int);

	print_log("int: %d\n", i);
}

void func_arg_cb_ptr(func f, va_list va)
{
	void * p = va_arg(va, void *);

	print_log("ptr: %p\n", p);
}

void func_call_1(func f, ...)
{
	static func_arg_callback arg_callback[TYPE_COUNT] =
	{
		&func_arg_cb_char,
		&func_arg_cb_int,
		&func_arg_cb_ptr
	};

	size_t count;

	va_list va;

	va_start(va, f);

	//for (f->count = 0; f->count < f->size; ++f->count)
	for (count = 0; count < f->size; ++count)
	{
		//arg_callback[f->arg_list[f->count].id](f, va);
		arg_callback[f->arg_list[count].id](f, va);
	}

	va_end(va);
}

void func_call_2(func f, ...)
{
	size_t count;

	va_list va;

	va_start(va, f);

	for (count = 0; count < f->size; ++count)
	{
		enum arg_type_id id = f->arg_list[count].id;

		if (id == TYPE_CHAR)
		{
			char c = (char)va_arg(va, int); // because of type promotion

			print_log("char: %c\n", c);
		}
		else if (id == TYPE_INT)
		{
			int i = va_arg(va, int);

			print_log("int: %d\n", i);
		}
		else if (id == TYPE_PTR)
		{
			void * p = va_arg(va, void *);

			print_log("ptr: %p\n", p);
		}
	}

	va_end(va);
}

void func_call_3_impl(func f, va_list va)
{
	size_t count;

	for (count = 0; count < f->size; ++count)
	{
		enum arg_type_id id = f->arg_list[count].id;

		if (id == TYPE_CHAR)
		{
			char c = (char)va_arg(va, int); // because of type promotion

			print_log("char: %c\n", c);
		}
		else if (id == TYPE_INT)
		{
			int i = va_arg(va, int);

			print_log("int: %d\n", i);
		}
		else if (id == TYPE_PTR)
		{
			void * p = va_arg(va, void *);

			print_log("ptr: %p\n", p);
		}
	}
}

void func_call_3(func f, ...)
{
	va_list va;

	va_start(va, f);

	func_call_3_impl(f, va);

	va_end(va);
}

void func_call_4_impl_ptr(func f, va_list * va)
{
	size_t count;

	for (count = 0; count < f->size; ++count)
	{
		enum arg_type_id id = f->arg_list[count].id;

		if (id == TYPE_CHAR)
		{
			char c = (char)va_arg(*va, int); // because of type promotion

			print_log("char: %c\n", c);
		}
		else if (id == TYPE_INT)
		{
			int i = va_arg(*va, int);

			print_log("int: %d\n", i);
		}
		else if (id == TYPE_PTR)
		{
			void * p = va_arg(*va, void *);

			print_log("ptr: %p\n", p);
		}
	}
}

void func_call_4(func f, ...)
{
	va_list va;

	va_start(va, f);

	func_call_4_impl_ptr(f, &va);

	va_end(va);
}

void func_call_c(char c, int i, void * p)
{
	print_log("char: %c\n", c);
	print_log("int: %d\n", i);
	print_log("ptr: %p\n", p);
}

int main(int argc, char *argv[])
{
	struct func_type f;

	arg_type_define(&f.arg_list[0], TYPE_CHAR, NULL, sizeof(char));
	arg_type_define(&f.arg_list[1], TYPE_INT, NULL, sizeof(int));
	arg_type_define(&f.arg_list[2], TYPE_PTR, NULL, sizeof(void *));

	f.size = 3;

	{
		int it, size;

		char c = 'm';
		int i = 123456789;
		void * p = &f;

		if (argc > 1)
		{
			size = atoi(argv[1]);
		}
		else
		{
			size = 20000000;
		}

		for (it = 0; it < size; ++it)
		{
			#ifdef FUNC_VA_ARGS
				#ifdef FUNC_VA_ARGS_1
					func_call_1(&f, c, i, p);
				#elif defined(FUNC_VA_ARGS_2)
					func_call_2(&f, c, i, p);
				#elif defined(FUNC_VA_ARGS_3)
					func_call_3(&f, c, i, p);
				#elif defined(FUNC_VA_ARGS_4)
					func_call_4(&f, c, i, p);
				#endif
			#else
				func_call_c(c, i, p);
			#endif
		}

		print_log("ptr: %p\n", &f);
	}

	return 0;
}
