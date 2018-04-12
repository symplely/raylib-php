#include "php.h"
#undef LOG_INFO
#undef LOG_WARNING
#undef LOG_DEBUG
#define Rectangle RectangleWin
#define CloseWindow CloseWindowWin
#define ShowCursor ShowCursorWin
#define DrawTextA DrawTextAWin
#define DrawTextExA DrawTextExAWin
#define LoadImageA LoadImageAWin
#include "raylib.h"
#include "raylib-texture.h"
#include "raylib-utils.h"


//------------------------------------------------------------------------------------------------------
//-- raylib Texture PHP Custom Object
//------------------------------------------------------------------------------------------------------
/* {{{ ZE2 OO definitions */
zend_object_handlers php_raylib_texture_object_handlers;

static HashTable php_raylib_texture_prop_handlers;

typedef int (*raylib_texture_read_int_t)(struct Texture2D *texture);

typedef struct _raylib_texture_prop_handler {
    raylib_texture_read_int_t read_int_func;
} raylib_texture_prop_handler;
/* }}} */

static void php_raylib_texture_register_prop_handler(HashTable *prop_handler, char *name, raylib_texture_read_int_t read_int_func) /* {{{ */
{
    raylib_texture_prop_handler hnd;

    hnd.read_int_func = read_int_func;
    zend_hash_str_add_mem(prop_handler, name, strlen(name), &hnd, sizeof(raylib_texture_prop_handler));

    /* Register for reflection */
    zend_declare_property_null(php_raylib_texture_ce, name, strlen(name), ZEND_ACC_PUBLIC);
}
/* }}} */

static zval *php_raylib_texture_property_reader(php_raylib_texture_object *obj, raylib_texture_prop_handler *hnd, zval *rv) /* {{{ */
{
    int retint = 0;

    if (obj != NULL && hnd->read_int_func) {
        retint = hnd->read_int_func(&obj->texture);
        if (retint == -1) {
            php_error_docref(NULL, E_WARNING, "Internal raylib texture error returned");
            return NULL;
        }
    }

    ZVAL_LONG(rv, (long)retint);

    return rv;
}
/* }}} */

static zval *php_raylib_texture_get_property_ptr_ptr(zval *object, zval *member, int type, void **cache_slot) /* {{{ */
{
    php_raylib_texture_object *obj;
    zval tmp_member;
    zval *retval = NULL;
    raylib_texture_prop_handler *hnd = NULL;
    zend_object_handlers *std_hnd;

    if (Z_TYPE_P(member) != IS_STRING) {
        ZVAL_COPY(&tmp_member, member);
        convert_to_string(&tmp_member);
        member = &tmp_member;
        cache_slot = NULL;
    }

    obj = Z_TEXTURE_OBJ_P(object);

    if (obj->prop_handler != NULL) {
        hnd = zend_hash_find_ptr(obj->prop_handler, Z_STR_P(member));
    }

    if (hnd == NULL) {
        std_hnd = zend_get_std_object_handlers();
        retval = std_hnd->get_property_ptr_ptr(object, member, type, cache_slot);
    }

    if (member == &tmp_member) {
        zval_dtor(member);
    }

    return retval;
}
/* }}} */

static zval *php_raylib_texture_read_property(zval *object, zval *member, int type, void **cache_slot, zval *rv) /* {{{ */
{
    php_raylib_texture_object *obj;
    zval tmp_member;
    zval *retval = NULL;
    raylib_texture_prop_handler *hnd = NULL;
    zend_object_handlers *std_hnd;

    if (Z_TYPE_P(member) != IS_STRING) {
        ZVAL_COPY(&tmp_member, member);
        convert_to_string(&tmp_member);
        member = &tmp_member;
        cache_slot = NULL;
    }

    obj = Z_TEXTURE_OBJ_P(object);

    if (obj->prop_handler != NULL) {
        hnd = zend_hash_find_ptr(obj->prop_handler, Z_STR_P(member));
    }

    if (hnd != NULL) {
        retval = php_raylib_texture_property_reader(obj, hnd, rv);
        if (retval == NULL) {
            retval = &EG(uninitialized_zval);
        }
    } else {
        std_hnd = zend_get_std_object_handlers();
        retval = std_hnd->read_property(object, member, type, cache_slot, rv);
    }

    if (member == &tmp_member) {
        zval_dtor(member);
    }

    return retval;
}
/* }}} */

static int php_raylib_texture_has_property(zval *object, zval *member, int type, void **cache_slot) /* {{{ */
{
    php_raylib_texture_object *obj;
    zval tmp_member;
    raylib_texture_prop_handler *hnd = NULL;
    zend_object_handlers *std_hnd;
    int retval = 0;

    if (Z_TYPE_P(member) != IS_STRING) {
        ZVAL_COPY(&tmp_member, member);
        convert_to_string(&tmp_member);
        member = &tmp_member;
        cache_slot = NULL;
    }

    obj = Z_TEXTURE_OBJ_P(object);

    if (obj->prop_handler != NULL) {
        hnd = zend_hash_find_ptr(obj->prop_handler, Z_STR_P(member));
    }

    if (hnd != NULL) {
        zval tmp, *prop;

        if (type == 2) {
            retval = 1;
        } else if ((prop = php_raylib_texture_property_reader(obj, hnd, &tmp)) != NULL) {
            if (type == 1) {
                retval = zend_is_true(&tmp);
            } else if (type == 0) {
                retval = (Z_TYPE(tmp) != IS_NULL);
            }
        }

        zval_ptr_dtor(&tmp);
    } else {
        std_hnd = zend_get_std_object_handlers();
        retval = std_hnd->has_property(object, member, type, cache_slot);
    }

    if (member == &tmp_member) {
        zval_dtor(member);
    }

    return retval;
}
/* }}} */

static HashTable *php_raylib_texture_get_gc(zval *object, zval **gc_data, int *gc_data_count) /* {{{ */
{
    *gc_data = NULL;
    *gc_data_count = 0;
    return zend_std_get_properties(object);
}
/* }}} */

static HashTable *php_raylib_texture_get_properties(zval *object)/* {{{ */
{
    php_raylib_texture_object *obj;
    HashTable *props;
    raylib_texture_prop_handler *hnd;
    zend_string *key;

    obj = Z_TEXTURE_OBJ_P(object);
    props = zend_std_get_properties(object);

    if (obj->prop_handler == NULL) {
        return NULL;
    }

    ZEND_HASH_FOREACH_STR_KEY_PTR(obj->prop_handler, key, hnd) {
                zval *ret, val;
                ret = php_raylib_texture_property_reader(obj, hnd, &val);
                if (ret == NULL) {
                    ret = &EG(uninitialized_zval);
                }
                zend_hash_update(props, key, ret);
            } ZEND_HASH_FOREACH_END();

    return props;
}
/* }}} */

void php_raylib_texture_free_storage(zend_object *object TSRMLS_DC)
{
    php_raylib_texture_object *intern = php_raylib_texture_fetch_object(object);

    zend_object_std_dtor(&intern->std);

    UnloadTexture(intern->texture);
}

zend_object * php_raylib_texture_new(zend_class_entry *ce TSRMLS_DC)
{
    php_raylib_texture_object *intern;
    intern = (php_raylib_texture_object*) ecalloc(1, sizeof(php_raylib_texture_object) + zend_object_properties_size(ce));
    intern->prop_handler = &php_raylib_texture_prop_handlers;

    zend_object_std_init(&intern->std, ce TSRMLS_CC);
    object_properties_init(&intern->std, ce);

    intern->std.handlers = &php_raylib_texture_object_handlers;

    return &intern->std;
}

// PHP property handling

static int php_raylib_texture_width(struct Texture2D *texture) /* {{{ */
{
    return texture->width;
}
/* }}} */

static int php_raylib_texture_height(struct Texture2D *texture) /* {{{ */
{
    return texture->height;
}
/* }}} */

static int php_raylib_texture_mipmaps(struct Texture2D *texture) /* {{{ */
{
    return texture->mipmaps;
}
/* }}} */

static int php_raylib_texture_format(struct Texture2D *texture) /* {{{ */
{
    return texture->format;
}
/* }}} */

static int php_raylib_texture_id(struct Texture2D *texture) /* {{{ */
{
    return texture->id;
}
/* }}} */

// PHP object handling

PHP_METHOD(Texture, __construct)
{
    zend_string *fileName;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_STR(fileName)
    ZEND_PARSE_PARAMETERS_END();

    php_raylib_texture_object *intern = Z_TEXTURE_OBJ_P(getThis());
    intern->texture = LoadTexture(fileName->val);
}

PHP_METHOD(Texture, width)
{
    php_raylib_texture_object *intern = Z_TEXTURE_OBJ_P(getThis());
    RETURN_LONG(intern->texture.width);
}

PHP_METHOD(Texture, height)
{
    php_raylib_texture_object *intern = Z_TEXTURE_OBJ_P(getThis());
    RETURN_LONG(intern->texture.height);
}

PHP_METHOD(Texture, format)
{
    php_raylib_texture_object *intern = Z_TEXTURE_OBJ_P(getThis());
    RETURN_LONG(intern->texture.format);
}

PHP_METHOD(Texture, id)
{
    php_raylib_texture_object *intern = Z_TEXTURE_OBJ_P(getThis());
    RETURN_LONG(intern->texture.id);
}

PHP_METHOD(Texture, mipmaps)
{
    php_raylib_texture_object *intern = Z_TEXTURE_OBJ_P(getThis());
    RETURN_LONG(intern->texture.mipmaps);
}

PHP_METHOD(Texture, draw)
{
    zend_long posX;
    zend_long posY;
    zval *colorArr;

    ZEND_PARSE_PARAMETERS_START(3, 3)
            Z_PARAM_LONG(posX)
            Z_PARAM_LONG(posY)
            Z_PARAM_ZVAL(colorArr)
    ZEND_PARSE_PARAMETERS_END();

    php_raylib_texture_object *intern = Z_TEXTURE_OBJ_P(getThis());

    DrawTexture(intern->texture, zend_long_2int(posX), zend_long_2int(posY), php_array_to_color(colorArr));
}

const zend_function_entry php_raylib_texture_methods[] = {
        PHP_ME(Texture, __construct, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(Texture, width, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(Texture, height, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(Texture, format, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(Texture, id, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(Texture, mipmaps, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(Texture, draw, NULL, ZEND_ACC_PUBLIC)
        PHP_FE_END
};

static void php_raylib_texture_free_prop_handler(zval *el) /* {{{ */ {
    pefree(Z_PTR_P(el), 1);
} /* }}} */

void php_raylib_texture_startup(INIT_FUNC_ARGS)
{
    zend_class_entry ce;

    //-- Object handlers
    memcpy(&php_raylib_texture_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    php_raylib_texture_object_handlers.offset = XtOffsetOf(php_raylib_texture_object, std);
    php_raylib_texture_object_handlers.free_obj = &php_raylib_texture_free_storage;
    php_raylib_texture_object_handlers.clone_obj = NULL;
    
    // Props
    php_raylib_texture_object_handlers.get_property_ptr_ptr = php_raylib_texture_get_property_ptr_ptr;
    php_raylib_texture_object_handlers.get_gc         = php_raylib_texture_get_gc;
    php_raylib_texture_object_handlers.get_properties = php_raylib_texture_get_properties;
    php_raylib_texture_object_handlers.read_property	= php_raylib_texture_read_property;
    php_raylib_texture_object_handlers.has_property	= php_raylib_texture_has_property;
    
    INIT_NS_CLASS_ENTRY(ce, "raylib", "Texture", php_raylib_texture_methods);
    php_raylib_texture_ce = zend_register_internal_class(&ce TSRMLS_CC);
    php_raylib_texture_ce->create_object = php_raylib_texture_new;
    
    // Props
    zend_hash_init(&php_raylib_texture_prop_handlers, 0, NULL, php_raylib_texture_free_prop_handler, 1);
    php_raylib_texture_register_prop_handler(&php_raylib_texture_prop_handlers, "width", php_raylib_texture_width);
    php_raylib_texture_register_prop_handler(&php_raylib_texture_prop_handlers, "height", php_raylib_texture_height);
    php_raylib_texture_register_prop_handler(&php_raylib_texture_prop_handlers, "mipmaps", php_raylib_texture_mipmaps);
    php_raylib_texture_register_prop_handler(&php_raylib_texture_prop_handlers, "height", php_raylib_texture_format);
    php_raylib_texture_register_prop_handler(&php_raylib_texture_prop_handlers, "id", php_raylib_texture_id);
}

#undef Rectangle
#undef CloseWindow
#undef ShowCursor
#undef DrawTextA
#undef DrawTextExA
#undef LoadImageA