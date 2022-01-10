#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <errno.h>
#include <math.h>

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGITS(ch)  ('0' <= (ch) && (ch) <= '9')
#define ISDIGITS1TO9(ch)  ('1' <= (ch) && (ch) <= '9')
#define ISEXP(ch)  ((ch) == 'e' || ch == 'E')
#define ISMP(ch) ((ch) == '+' || ch == '-' )

typedef struct {
    const char* json;
} lept_context;

static void lept_parse_whitespace(lept_context* c)
{
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

/*static int lept_parse_true(lept_context* c, lept_value* v) {
    EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_context* c, lept_value* v) {
    EXPECT(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

static int lept_parse_null(lept_context* c, lept_value* v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}*/

static int lept_parse_literal(lept_context* c, lept_value* v, const char *literal, lept_type type)
{
    size_t i = 0;
    EXPECT(c, literal[0]);
    for (i = 0; literal[i + 1]; i++) {
        if (c->json[i] != literal[i + 1]) {
            return LEPT_PARSE_INVALID_VALUE;
        }
    }
    c->json += i;
    v->type = type;
    return LEPT_PARSE_OK;
}
static int lept_parse_number(lept_context* c, lept_value* v)
{
    char* end;
    char *p = (char*)c->json;
    /*int status = 0;*/
    /* \TODO validate number */
    end = (char*)c->json;
    /*使用状态自动机*/
    /*for (; *end && status != -1; end++) {
        if (status == 0) {
            if (*end == '-') {
                status = 1;
            }
            else if (ISDIGITS1TO9(*end)) {
                status = 3;
            }
            else if (*end == '0') {
                status = 2;
            }
            else status = -1;
        }
        else if (status == 1) {
            if (*end == '0') {
                status = 2;
            }
            else if (ISDIGITS1TO9(*end)) {
                status = 3;
            }
            else status = -1;
        }
        else if (status == 2) {
            if (ISEXP(*end)) {
                status = 6;
            }
            else if (*end == '.') {
                status = 4;
            }
            else status = -1;
        }
        else if (status == 3) {
            if (ISDIGITS(*end)) {
                status = 3;
            }
            else if (*end == '.') {
                status = 4;
            }
            else if (ISEXP(*end)) {
                status = 6;
            }
            else status = -1;
        }
        else if (status == 4) {
            if (ISDIGITS(*end)) {
                status = 5;
            }
            else status = -1;
        }
        else if (status == 5) {
            if (ISDIGITS(*end)) {
                status = 5;
            }
            else if (ISEXP(*end)) {
                status = 6;
            }
            else status = -1;
        }
        else if (status == 6) {
            if (ISMP(*end)) {
                status = 7;
            }
            else if (ISDIGITS(*end)) {
                status = 8;
            }
            else status = -1;
        }
        else if (status == 7) {
            if (ISDIGITS(*end)) {
                status = 8;
            }
            else status = -1;
        }
        else if (status == 8) {
            if (ISDIGITS(*end)) {
                status = 8;
            }
            else status = -1;
        }
    }*/


    if (*p == '-') {
        p++;
    }

    if (*p == '0') {
        p++;
    }
    else {
        if (!ISDIGITS1TO9(*p)) {
            return LEPT_PARSE_INVALID_VALUE;
        }
        for(; ISDIGITS(*p); p++);
    }

    if (*p == '.') {
        p++;
        if (!ISDIGITS(*p)) {
            return LEPT_PARSE_INVALID_VALUE;
        }
        for(; ISDIGITS(*p); p++);
    }

    if (*p == 'E' || *p == 'e') {
        p++;
        if (*p == '-' || *p == '+') {
            p++;
        }
        for(; ISDIGITS(*p); p++);
    }

    end = p;
    if (c->json == end)
        return LEPT_PARSE_INVALID_VALUE;

    v->n = strtod(c->json, &p);
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL)) {
        v->type = LEPT_NULL;
        return LEPT_PARSE_NUMBER_TOO_BIG;
    }

    v->type = LEPT_NUMBER;
    c->json = end;
    return LEPT_PARSE_OK;
    /*状态自动机*/
    /*if (status == 2 || status == 3 || status == 5 || status == 8) {
        v->type = LEPT_NUMBER;
        v->n = strtod(c->json, &end);
        c->json = end;
        if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL)) {
            errno = 0;
            v->type = LEPT_NULL;
            return LEPT_PARSE_NUMBER_TOO_BIG;
        }
        return LEPT_PARSE_OK;
    }
    if (*c->json == '0') {
        c->json += 1;
        v->type = LEPT_NUMBER;
        return LEPT_PARSE_OK;
    }
    c->json = end - 1;
    v->type = LEPT_NULL;
    return LEPT_PARSE_INVALID_VALUE;
    */
}

static int lept_parse_value(lept_context* c, lept_value* v)
{
    switch (*c->json) {
    case 't':
        return lept_parse_literal(c, v, "true", LEPT_TRUE);
    case 'f':
        return lept_parse_literal(c, v, "false", LEPT_FALSE);
    case 'n':
        return lept_parse_literal(c, v, "null", LEPT_NULL);
    default:
        return lept_parse_number(c, v);
    case '\0':
        return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json)
{
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v)
{
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v)
{
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
