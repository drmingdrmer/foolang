#include <stdlib.h>
#include <stdio.h>

#define dd(fmt, ...) \
        printf(fmt "\n", ##__VA_ARGS__)

typedef struct buf_s buf_t;
typedef struct sym_s sym_t;
typedef struct symbol_list_s symbol_list_t;
typedef struct str_s str_t;

struct str_s {
    size_t size;
    char *data;
};

struct buf_s {
    size_t  size;
    char   *data;
    buf_t  *next;
};

struct sym_s {
    int sym_type;
    union {
        int integer;
        str_t str;
    };
};

struct symbol_list_s {
    size_t size;
    size_t used;
    sym_t *symbols;
};

enum {
    ST_NONE,
    ST_NUM,
    ST_IDENT,
};

char *stat_strings[] = {
    "status_none",
    "status_num",
    "status_ident",
    NULL,
};

/* single char symbol like '=' or '+' use its ascii code as symbol type */
enum {
    SBT_NONE = 0x0100,
    SBT_NUM,
    SBT_IDENT,
};

char *symbol_type_strings[] = {
    "none",
    "number",
    "identifier",
    NULL,
};

int buf_init(buf_t *buf, size_t size)
{
    buf->size = size;
    buf->data = (char*)malloc(sizeof(char) * size);
    buf->next = NULL;
    return 0;
}

symbol_list_t *symbol_list_new(size_t size)
{
    symbol_list_t *sb = (symbol_list_t*)malloc(sizeof(symbol_list_t));
    sb->symbols = (sym_t*)malloc(sizeof(sym_t)*size);
    sb->size = size;
    sb->used = 0;
    return sb;
}

sym_t *sb_add_symbol(symbol_list_t *sb)
{
    if (sb->used == sb->size) {
        sym_t *prev = sb->symbols;
        sb->symbols = (sym_t*)realloc(sb->symbols, sizeof(sym_t) * sb->size * 2);
        sb->size *= 2;
    }

    sb->used++;

    return &sb->symbols[sb->used-1];
}

char *symbol_tostr(sym_t *s)
{
    /* TODO concurrency issue */
    static char sym_str_buf[1024];

    if (s->sym_type < SBT_NONE) {
        sym_str_buf[0] = (char)s->sym_type;
        sym_str_buf[1] = '\0';
        return sym_str_buf;
    }
    else {
        if (s->sym_type == SBT_NUM) {
            sprintf(sym_str_buf, "%s: %d",
                    symbol_type_strings[s->sym_type - SBT_NONE], s->integer);
        }
        else if (s->sym_type == SBT_IDENT) {
            sprintf(sym_str_buf, "%s: %.*s",
                    symbol_type_strings[s->sym_type - SBT_NONE], (int)s->str.size, s->str.data);
        }
        else {
            return NULL;
        }
    }
    return sym_str_buf;
}

int init_symbol(sym_t *s, int sym_type)
{
    s->sym_type = sym_type;
    return 0;
}

    int
lex(buf_t *buf, symbol_list_t *sb)
{
    char           c;
    sym_t         *sym;
    int            i;
    int            st;

    size_t size_symbuf = 1024;
    char symbuf[1024];
    size_t isymbuf = 0;


    st = ST_NONE;
    for (i = 0; i < buf->size; i++) {

        c = buf->data[i];
        /* dd("i=%d, c=%c %x, st=%d", i, c, c, st); */

        switch (c) {
            case '=':
                sym = sb_add_symbol(sb);
                init_symbol(sym, c);
                dd("new symbol: %s", symbol_tostr(sym));
                st = ST_NONE;
                break;

            case ' ':
            case '\t':
            case '\r':
            case '\n':
            case '\0':
                if (st == ST_NONE) {
                    /* dd("skip blank"); */
                }
                else if (st == ST_NUM) {
                    sym = sb_add_symbol(sb);
                    init_symbol(sym, SBT_NUM);

                    symbuf[isymbuf] = '\0';
                    sym->integer = atoi(symbuf);

                    dd("new symbol: %s", symbol_tostr(sym));

                    isymbuf = 0;
                    st = ST_NONE;
                }
                else if (st == ST_IDENT) {
                    sym = sb_add_symbol(sb);
                    init_symbol(sym, SBT_IDENT);

                    symbuf[isymbuf] = '\0';
                    sym->str.data = &buf->data[ i - isymbuf ];
                    sym->str.size = isymbuf;

                    dd("new symbol: %s", symbol_tostr(sym));

                    isymbuf = 0;
                    st = ST_NONE;
                }
                else {
                    dd("unknown st: %d", st);
                    return -1;
                }
                break;

            default:
                if (st == ST_NONE) {
                    if (c >= '0' && c <= '9') {
                        st = ST_NUM;
                        isymbuf = 0;
                        symbuf[isymbuf] = c;
                        isymbuf++;
                    }
                    else if (c >= 'a' && c <= 'z') {
                        st = ST_IDENT;
                        isymbuf = 0;
                        symbuf[isymbuf] = c;
                        isymbuf++;
                    }
                    else {
                        dd("invalid '%c'", c);
                        return -1;
                    }
                }
                else if (st == ST_NUM) {
                    if (c >= '0' && c <= '9') {
                        symbuf[isymbuf] = c;
                        isymbuf++;
                    }
                    else {
                        dd("invalid '%c' for ST_NUM", c);
                        return -1;
                    }
                }
                else if (st == ST_IDENT) {
                    if ((c >= '0' && c <= '9')
                        || (c >= 'a' && c <= 'z')) {
                        symbuf[isymbuf] = c;
                        isymbuf++;
                    }
                    else {
                        dd("invalid '%c' for ST_IDENT", c);
                        return -1;
                    }
                }
                else {
                    dd("invalid st: %d", st);
                    return -1;
                }
        }
    }
    return 0;
}

    int
parse(symbol_list_t *sb)
{
    int i;
    sym_t *sym;
    for (i = 0; i < sb->used; i++) {
        sym = &sb->symbols[i];
        dd("symbol: %s", symbol_tostr(sym));

        switch (sym->sym_type) {
            case constant:
                
                break;
        }
    }
    return 0;
}


    int
main(int argc, char **argv)
{
    buf_t          buf;
    int            rc;
    symbol_list_t *sb;

    char           b[] = "a = b";

    /* includes '\0' */
    buf_init(&buf, sizeof(b));

    /* leak */
    buf.data = b;

    sb = symbol_list_new(1024);
    rc = lex(&buf, sb);
    dd("rc=%d", rc);

    rc = parse(sb);
    dd("rc of parse=%d", rc);


    return 0;
}
