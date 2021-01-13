#include "settee.h"

/* Types */

typedef struct {
    JanetTable *data;
    int is_frozen;
} settee_set_t;

/* Forward Declarations */

/* Utility Methods */

int settee_set_in(settee_set_t *set, Janet value) {
    JanetKV *bucket = janet_table_find(set->data, value);
    if (NULL != bucket && !janet_checktype(bucket->key, JANET_NIL)) {
        return 1;
    } else {
        return 0;
    }
}

/* Deinitialising */

static int settee_set_gc(void *p, size_t size) {
    (void) size;
    (void) p;
    return 0;
}

/* Initialising  */

static void settee_set_init(settee_set_t *set, JanetView values, int is_frozen) {
    set->data = janet_table(values.len);
    for (size_t i = 0; i < (size_t)values.len; i++) {
        janet_table_put(set->data, values.items[i], janet_wrap_true());
    }
    set->is_frozen = is_frozen;
}

static void settee_set_clone(settee_set_t *old_set, settee_set_t *new_set) {
    new_set->data = janet_table(old_set->data->count);
    Janet wrapped_table = janet_wrap_table(old_set->data);
    Janet key = janet_next(wrapped_table, janet_wrap_nil());
    for (size_t i = 0; !janet_checktype(key, JANET_NIL); i++) {
        janet_table_put(new_set->data, key, janet_wrap_true());
        key = janet_next(wrapped_table, key);
    }
    new_set->is_frozen = old_set->is_frozen;
}

/* Marking */

static int settee_set_mark(void *p, size_t size) {
    (void) size;
    settee_set_t *set = (settee_set_t *)p;
    janet_mark(janet_wrap_table(set->data));
    return 0;
}

/* Accessing */

static JanetMethod settee_set_methods[2];

static int settee_set_get(void *p, Janet key, Janet *out) {
    if (janet_checktype(key, JANET_KEYWORD)) {
        int success = janet_getmethod(janet_unwrap_keyword(key), settee_set_methods, out);
        if (success) return 1;
    }

    settee_set_t *set = (settee_set_t *)p;
    if (!settee_set_in(set, key)) return 0;

    *out = key;
    return 1;
}

/* Inserting */

static void settee_set_put(void *p, Janet value, Janet ignored) {
    (void) ignored;
    settee_set_t *set = (settee_set_t *)p;
    janet_table_put(set->data, value, janet_wrap_true());
}

/* Stringifying */

static void settee_set_to_string(void *p, JanetBuffer *buf) {
    settee_set_t *set = (settee_set_t *)p;
    if (!set->is_frozen) janet_buffer_push_cstring(buf, "@");
    janet_buffer_push_cstring(buf, "#{");
    Janet wrapped_table = janet_wrap_table(set->data);
    Janet key = janet_next(wrapped_table, janet_wrap_nil());
    for (size_t i = 0; !janet_checktype(key, JANET_NIL); i++) {
        if (i > 0) janet_buffer_push_cstring(buf, " ");
        janet_buffer_push_string(buf, janet_description(key));
        key = janet_next(wrapped_table, key);
    }
    janet_buffer_push_cstring(buf, "}");
}

/* Comparing */

static int settee_set_compare_sizes(settee_set_t *a, settee_set_t *b) {
    return (a->data->count == b->data->count) ? 0 :
           (a->data->count >  b->data->count) ? 1 : -1;
}

static int settee_set_compare(void *p1, void *p2) {
    settee_set_t *a = (settee_set_t *)p1;
    settee_set_t *b = (settee_set_t *)p2;
    if (a->data->count != b->data->count) {
        return settee_set_compare_sizes(a, b);
    }
    Janet wrapped_table = janet_wrap_table(a->data);
    Janet key = janet_next(wrapped_table, janet_wrap_nil());
    for (size_t i = 0; !janet_checktype(key, JANET_NIL); i++) {
        if (!settee_set_in(b, key)) return settee_set_compare_sizes(a, b);
        key = janet_next(wrapped_table, key);
    }
    return 0;
}

/* Hashing */

static int32_t settee_set_hash(void *p, size_t size) {
    (void) size;
    settee_set_t *set = (settee_set_t *)p;
    uint32_t hash = 5381;
    Janet wrapped_table = janet_wrap_table(set->data);
    Janet key = janet_next(wrapped_table, janet_wrap_nil());
    for (size_t i = 0; !janet_checktype(key, JANET_NIL); i++) {
        hash = (hash << 5) + hash + janet_hash(key);
        key = janet_next(wrapped_table, key);
    }
    return (int32_t)hash;
}

/* Traversing */

static Janet settee_set_next(void *p, Janet key) {
    settee_set_t *set = (settee_set_t *)p;
    return janet_next(janet_wrap_table(set->data), key);
}

/* Type Definition */

static const JanetAbstractType settee_set_type = {
    "settee/set",
    settee_set_gc,
    settee_set_mark, /* GC Mark */
    settee_set_get, /* Get */
    settee_set_put, /* Put */
    NULL, /* Marshall */
    NULL, /* Unmarshall */
    settee_set_to_string, /* String */
    settee_set_compare, /* Compare */
    settee_set_hash, /* Hash */
    settee_set_next, /* Next */
    JANET_ATEND_NEXT
};

/* C Functions */

static Janet cfun_settee_new(int32_t argc, Janet *argv) {
    JanetView values;
    if (0 == argc) {
        values.len = 0;
        values.items = NULL;
    } else {
        values.len = argc;
        values.items = janet_tuple_n(argv, argc);
    }

    settee_set_t *set = (settee_set_t *)janet_abstract(&settee_set_type, sizeof(settee_set_t));
    settee_set_init(set, values, 0);

    return janet_wrap_abstract(set);
}

static Janet cfun_settee_frozen(int32_t argc, Janet *argv) {
    JanetView values;
    if (0 == argc) {
        values.len = 0;
        values.items = NULL;
    } else {
        values.len = argc;
        values.items = janet_tuple_n(argv, argc);
    }

    settee_set_t *set = (settee_set_t *)janet_abstract(&settee_set_type, sizeof(settee_set_t));
    settee_set_init(set, values, 1);

    return janet_wrap_abstract(set);
}

static Janet cfun_settee_is_frozen(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 1);

    settee_set_t *set = (settee_set_t *)janet_getabstract(argv, 0, &settee_set_type);

    if (set->is_frozen) {
        return janet_wrap_true();
    } else {
        return janet_wrap_false();
    }
}

static Janet cfun_settee_in(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 2);

    settee_set_t *set = (settee_set_t *)janet_getabstract(argv, 0, &settee_set_type);
    Janet value = *(argv + 1);

    if (settee_set_in(set, value)) {
        return janet_wrap_true();
    } else {
        return janet_wrap_false();
    }
}

static Janet cfun_settee_add(int32_t argc, Janet *argv) {
    janet_arity(argc, 2, -1);

    settee_set_t *set = (settee_set_t *)janet_getabstract(argv, 0, &settee_set_type);

    JanetView values;
    values.len = argc - 1;
    values.items = janet_tuple_n(++argv, (argc - 1));

    settee_set_t *res;
    if (set->is_frozen) {
        res = (settee_set_t *)janet_abstract(&settee_set_type, sizeof(settee_set_t));
        settee_set_clone(set, res);
    } else {
        res = set;
    }

    for (size_t i = 0; i < (size_t)values.len; i++) {
        janet_table_put(res->data, values.items[i], janet_wrap_true());
    }

    return janet_wrap_abstract(res);
}

static Janet cfun_settee_remove(int32_t argc, Janet *argv) {
    janet_arity(argc, 2, -1);

    settee_set_t *set = (settee_set_t *)janet_getabstract(argv, 0, &settee_set_type);

    JanetView values;
    values.len = argc - 1;
    values.items = janet_tuple_n(++argv, (argc - 1));

    settee_set_t *res;
    if (set->is_frozen) {
        res = (settee_set_t *)janet_abstract(&settee_set_type, sizeof(settee_set_t));
        settee_set_clone(set, res);
    } else {
        res = set;
    }

    for (size_t i = 0; i < (size_t)values.len; i++) {
        janet_table_remove(res->data, values.items[i]);
    }

    return janet_wrap_abstract(res);
}


static const JanetReg cfuns[] = {
    {"new", cfun_settee_new, NULL},
    {"frozen", cfun_settee_frozen, NULL},
    {"frozen?", cfun_settee_is_frozen, NULL},
    {"in?", cfun_settee_in, NULL},
    {"add", cfun_settee_add, NULL},
    {"remove", cfun_settee_remove, NULL},
    {NULL, NULL, NULL}
};

/* Methods */

static Janet settee_set_method_length(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 1);
    settee_set_t *set = (settee_set_t *)janet_getabstract(argv, 0, &settee_set_type);
    return janet_wrap_number(set->data->count);
}

static JanetMethod settee_set_methods[] = {
    {"length", settee_set_method_length},
    {NULL, NULL}
};

/* Environment Registration */

void settee_register_type(JanetTable *env) {
    (void) env;
    janet_register_abstract_type(&settee_set_type);
}

void settee_register_functions(JanetTable *env) {
    janet_cfuns(env, "settee", cfuns);
}

JANET_MODULE_ENTRY(JanetTable *env) {
    settee_register_type(env);
    settee_register_functions(env);
}
