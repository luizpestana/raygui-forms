#include "main.h"
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "tiny-json.h"

static inline void *realloc_it(void *ptrmem, size_t size) {
    void *p = realloc(ptrmem, size);
    if (!p) {
        free(ptrmem);
    }
    return p;
}

static inline bool json_isValue(json_t const* property, char const* text) {
    if (json_getType(property) == JSON_TEXT) {
        char const* value = json_getValue(property);
        return (strlen(value) == strlen(text) && strncmp(value, text, strlen(value)) == 0);
    }
    return false;
}

static inline int json_isValues(json_t const* property, int count, ...) {
    if (json_getType(property) == JSON_TEXT) {
        char const* value = json_getValue(property);
        va_list ap;
        va_start(ap, count);
        for (int i = 0; i < count; i++) {
            char const* text = va_arg(ap, char const*);
            if (strlen(value) == strlen(text) && strncmp(value, text, strlen(value)) == 0) {
                va_end(ap);
                return i;
            }
        }
        va_end(ap);
    }
    return -1;
}

typedef enum {
    RGFORM_INVALID,
    RGFORM_STRING,
    RGFORM_BOOLEAN,
    RGFORM_NUMBER,
    RGFORM_INTEGER
} rgform_field_type;

typedef struct rgform_field {
    char const* name;
    rgform_field_type type;
    char* str_value;
    bool bool_value;
    int max_length;
    float height;
    struct rgform_field* next_field;
} rgform_field;

typedef struct {
    char* js;
    rgform_field* first_field;
    int field_count;
} rgform;

static inline rgform* rgform_init(char const* filename) {
    if (!FileExists(filename)) {
        printf("Form json file not found.");
        return NULL;
    }
    char* js = LoadFileText(filename);

    json_t mem[JSON_TOKENS];
    json_t const* json = json_create( js, mem, sizeof mem / sizeof *mem );
    if (!json) {
        printf("Error json create.");
        free(js);
        return NULL;
    }

    json_t const* json_type = json_getProperty(json, "type");
    if (json_type && !json_isValue(json_type, "object")) {
        printf("Error, the root type should be object.");
        free(js);
        return NULL;
    }

    json_t const* json_properties = json_getProperty(json, "properties");
    if (!json_properties || json_getType(json_properties) != JSON_OBJ) {
        printf("Error, the properties list not found.");
        free(js);
        return NULL;
    }

    rgform* form = memcpy(malloc(sizeof(rgform)), &((rgform){
            .js = js,
            .first_field = NULL,
            .field_count = 0
    }), sizeof(rgform));
    rgform_field* prev_field;

    json_t const* json_property;
    for (json_property = json_getChild(json_properties); json_property != 0; json_property = json_getSibling(json_property)) {
        if (json_getType( json_property) == JSON_OBJ) {

            rgform_field_type type = RGFORM_INVALID;
            json_type = json_getProperty(json_property, "type");
            if (json_type) {
                switch (json_isValues(json_type, 4, "string", "boolean", "number", "integer")) {
                    case 0: type = RGFORM_STRING; break;
                    case 1: type = RGFORM_BOOLEAN; break;
                    case 2: type = RGFORM_NUMBER; break;
                    case 3: type = RGFORM_INTEGER; break;
                }
            }
            if (type != RGFORM_INVALID) {
                rgform_field* field = memcpy(malloc(sizeof(rgform_field)), &((rgform_field) {
                        .name = json_getName(json_property),
                        .type = type
                }), sizeof(rgform_field));
                if (type == RGFORM_STRING) {
                    field->max_length = 255;
                    field->str_value = malloc((field->max_length + 1) * sizeof(char));
                    strcpy(field->str_value, "");
                }
                if (form->field_count == 0) {
                    form->first_field = field;
                } else {
                    prev_field->next_field = field;
                }
                prev_field = field;
                form->field_count++;
            }
        }
    }

    return form;
}

static inline void rgform_render(rgform* form, float left, float top) {
    if (!form) return;
    rgform_field* field = form->first_field;
    while (field) {
        if (field->type == RGFORM_BOOLEAN) {
            field->bool_value = GuiCheckBox((Rectangle){ left, top, 15, 15 }, field->name, field->bool_value);
            top += 20;
        } else {
            GuiDrawText(field->name, (Rectangle) {left, top, 125, 10}, TEXT_ALIGN_LEFT, BLACK);
            top += 10;
            if (GuiTextBox((Rectangle) {left, top, 125, 30}, field->str_value, field->max_length, field->bool_value)) {
                field->bool_value = !field->bool_value;
            }
            top += 35;
        }
        field = field->next_field;
    }
}

static inline void rgform_free(rgform* form) {
    if (!form) return;
    rgform_field* field = form->first_field;
    rgform_field* prev_field;
    while (field) {
        if (field->type == RGFORM_STRING) {
            free(field->str_value);
        }
        prev_field = field;
        field = field->next_field;
        free(prev_field);
    }
    free(form->js);
    free(form);
}

int main(void)
{
    rgform* form = rgform_init("../form.json");

    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raygui forms example");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

        rgform_render(form, 10, 10);

        EndDrawing();
    }

    CloseWindow();

    rgform_free(form);

    return EXIT_SUCCESS;
}