/**
 * Copyright 2019-2020 DigitalOcean Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stddef.h>

// Public
#include "prom_alloc.h"

// Private
#include "prom_assert.h"
#include "prom_string_builder_i.h"
#include "prom_string_builder_t.h"

// The initial size of a string created via prom_string_builder
#define PROM_STRING_BUILDER_INIT_SIZE 32

// prom_string_builder_init prototype declaration
int prom_string_builder_init(prom_string_builder_t* self);

struct prom_string_builder
{
    char* str;        /**< the target string  */
    size_t allocated; /**< the size allocated to the string in bytes */
    size_t len;       /**< the length of str */
    size_t init_size; /**< the initialize size of space to allocate */
};

prom_string_builder_t* prom_string_builder_new(void)
{
    int r = 0;

    prom_string_builder_t* self = (prom_string_builder_t*)prom_malloc(sizeof(prom_string_builder_t));
    self->init_size = PROM_STRING_BUILDER_INIT_SIZE;
    r = prom_string_builder_init(self);
    if (r)
    {
        prom_string_builder_destroy(self);
        return NULL;
    }

    return self;
}

int prom_string_builder_init(prom_string_builder_t* self)
{
    PROM_ASSERT(self != NULL);
    if (self == NULL)
        return 1;
    self->str = (char*)prom_malloc(self->init_size);
    *self->str = '\0';
    self->allocated = self->init_size;
    self->len = 0;
    return 0;
}

int prom_string_builder_destroy(prom_string_builder_t* self)
{
    PROM_ASSERT(self != NULL);
    if (self == NULL)
        return 0;
    prom_free(self->str);
    self->str = NULL;
    prom_free(self);
    self = NULL;
    return 0;
}

/**
 * @brief API PRIVATE Grows the size of the string given the value we want to add
 *
 * The method continuously shifts left until the new size is large enough to accommodate add_len. This private method
 * is called in methods that need to add one or more characters to the underlying string.
 */
static int prom_string_builder_ensure_space(prom_string_builder_t* self, size_t add_len)
{
    PROM_ASSERT(self != NULL);
    if (self == NULL)
        return 1;
    if (add_len == 0 || self->allocated >= self->len + add_len + 1)
        return 0;
    while (self->allocated < self->len + add_len + 1)
        self->allocated <<= 1;
    self->str = (char*)prom_realloc(self->str, self->allocated);
    return 0;
}

int prom_string_builder_add_str(prom_string_builder_t* self, const char* str)
{
    PROM_ASSERT(self != NULL);
    int r = 0;

    if (self == NULL)
        return 1;
    if (str == NULL || *str == '\0')
        return 0;

    size_t len = strlen(str);
    r = prom_string_builder_ensure_space(self, len);
    if (r)
        return r;

    memcpy(self->str + self->len, str, len);
    self->len += len;
    self->str[self->len] = '\0';
    return 0;
}

int prom_string_builder_add_char(prom_string_builder_t* self, char c)
{
    PROM_ASSERT(self != NULL);
    int r = 0;

    if (self == NULL)
        return 1;
    r = prom_string_builder_ensure_space(self, 1);
    if (r)
        return r;

    self->str[self->len] = c;
    self->len++;
    self->str[self->len] = '\0';
    return 0;
}

int prom_string_builder_truncate(prom_string_builder_t* self, size_t len)
{
    PROM_ASSERT(self != NULL);
    if (self == NULL)
        return 1;
    if (len >= self->len)
        return 0;

    self->len = len;
    self->str[self->len] = '\0';
    return 0;
}

int prom_string_builder_clear(prom_string_builder_t* self)
{
    PROM_ASSERT(self != NULL);
    prom_free(self->str);
    self->str = NULL;
    return prom_string_builder_init(self);
}

size_t prom_string_builder_len(prom_string_builder_t* self)
{
    PROM_ASSERT(self != NULL);
    return self->len;
}

char* prom_string_builder_dump(prom_string_builder_t* self)
{
    PROM_ASSERT(self != NULL);
    // +1 to accommodate \0
    char* out = (char*)prom_malloc((self->len + 1) * sizeof(char));
    memcpy(out, self->str, self->len + 1);
    return out;
}

char* prom_string_builder_str(prom_string_builder_t* self)
{
    PROM_ASSERT(self != NULL);
    return self->str;
}
