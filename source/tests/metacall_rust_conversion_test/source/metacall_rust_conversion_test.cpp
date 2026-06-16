/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <gtest/gtest.h>

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>
#include <metacall/metacall_value.h>

class metacall_rust_conversion_test : public testing::Test {
    public:
};

TEST_F(metacall_rust_conversion_test, DefaultConstructor) {
    metacall_print_info();

    // metacall_log_null();

    ASSERT_EQ((int)0, (int)metacall_initialize()); 

    {
        static const char *identity_fn[METACALL_SIZE] = { NULL }; 

        //identity_fn[METACALL_BOOL] = "identity_bool";
        identity_fn[METACALL_CHAR] = "identity_char";
        identity_fn[METACALL_SHORT] = "identity_short";
        identity_fn[METACALL_INT] = "identity_int";
        identity_fn[METACALL_LONG] = "identity_long";
        identity_fn[METACALL_FLOAT] = "identity_float";
        identity_fn[METACALL_DOUBLE] = "identity_double";
        identity_fn[METACALL_STRING] = "identity_string";

    const char *scripts[] = { "identity.rs" };
    void *handle = NULL;
    void *ret;

    ASSERT_EQ((int)0, (int)metacall_load_from_file("rs", scripts, 1, &handle));

    for(size_t id = 0; id < METACALL_SIZE; ++id) {

        if(identity_fn[id] == NULL) {
            std::cout << metacall_value_id_name((enum metacall_value_id)id) << " => NULL (unsupported)" << std::endl;
            continue;
        }

        void *args[1] = { metacall_value_create((enum metacall_value_id)id)};
        ret = metacallhv(handle, identity_fn[id], args);

        ASSERT_NE(ret, nullptr);
        ASSERT_EQ(metacall_value_id(ret), (enum metacall_value_id)id);

        std::cout << metacall_value_id_name((enum metacall_value_id)id) << " => " << metacall_value_id_name(metacall_value_id(ret)) << std::endl;

        metacall_value_destroy(ret);
        metacall_value_destroy(args[0]);

    } 
}

    metacall_destroy();

}

