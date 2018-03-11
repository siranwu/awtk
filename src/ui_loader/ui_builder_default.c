/**
 * File:   ui_builder_default.c
 * Author: Li XianJing <xianjimli@hotmail.com>
 * Brief:  ui_builder default
 *
 * Copyright (c) 2018 - 2018  Li XianJing <xianjimli@hotmail.com>
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2018-02-14 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "base/button.h"
#include "base/check_button.h"
#include "base/dialog.h"
#include "base/enums.h"
#include "base/group_box.h"
#include "base/image.h"
#include "base/label.h"
#include "base/progress_bar.h"
#include "base/utf8.h"
#include "base/value.h"
#include "base/window.h"
#include "ui_loader/ui_builder_default.h"
#include "ui_loader/ui_loader_default.h"
#include "base/resource_manager.h"

static ret_t ui_builder_default_on_widget_start(ui_builder_t* b, const widget_desc_t* desc) {
  rect_t r;
  xy_t x = desc->layout.x;
  xy_t y = desc->layout.y;
  wh_t w = desc->layout.w;
  wh_t h = desc->layout.h;
  widget_t* widget = NULL;
  uint16_t type = desc->type;
  widget_t* parent = b->widget;

  if (parent != NULL) {
    widget_layout_calc(&desc->layout, &r, parent->w, parent->h);
    x = r.x;
    y = r.y;
    w = r.w;
    h = r.h;
  }

  if (parent && parent->type == WIDGET_DIALOG) {
    dialog_t* dlg = DIALOG(parent);
    parent = dlg->client;
  }

  switch (type) {
    case WIDGET_DIALOG:
      widget = dialog_create(parent, x, y, w, h);
      break;
    case WIDGET_NORMAL_WINDOW:
      widget = window_create(parent, x, y, w, h);
      break;
    case WIDGET_IMAGE:
      widget = image_create(parent, x, y, w, h);
      break;
    case WIDGET_BUTTON:
      widget = button_create(parent, x, y, w, h);
      break;
    case WIDGET_LABEL:
      widget = label_create(parent, x, y, w, h);
      break;
    case WIDGET_PROGRESS_BAR:
      widget = progress_bar_create(parent, x, y, w, h);
      break;
    case WIDGET_GROUP_BOX:
      widget = group_box_create(parent, x, y, w, h);
      break;
    case WIDGET_CHECK_BUTTON:
      widget = check_button_create(parent, x, y, w, h);
      break;
    case WIDGET_RADIO_BUTTON:
      widget = check_button_create_radio(parent, x, y, w, h);
      break;
    default:
      log_debug("%s: not supported type %d\n", __func__, type);
      break;
  }

  b->widget = widget;
  if (b->root == NULL) {
    b->root = widget;
  }

  log_debug("%d %d %d %d %d\n", type, x, y, w, h);

  return RET_OK;
}

static ret_t ui_builder_default_on_widget_prop(ui_builder_t* b, const char* name,
                                               const char* value) {
  value_t v;
  if (strcmp(name, "text") == 0) {
    wchar_t str[128];
    uint32_t len = strlen(value);
    return_value_if_fail(len < ARRAY_SIZE(str), RET_BAD_PARAMS);

    value_set_wstr(&v, utf8_to_utf16(value, str, ARRAY_SIZE(str)));
    widget_set_prop(b->widget, name, &v);
  } else if (strcmp(name, "align_v") == 0) {
    const key_type_value_t* item = align_v_name_find(value);
    if (item != NULL) {
      value_set_int(&v, item->value);
      widget_set_prop(b->widget, name, &v);
    } else {
      log_debug("%s %s is invalid.\n", __func__, value);
    }
  } else if (strcmp(name, "align_h") == 0) {
    const key_type_value_t* item = align_h_name_find(value);
    if (item != NULL) {
      value_set_int(&v, item->value);
      widget_set_prop(b->widget, name, &v);
    } else {
      log_debug("%s %s is invalid.\n", __func__, value);
    }
  } else {
    value_set_str(&v, value);
    widget_set_prop(b->widget, name, &v);
  }

  return RET_OK;
}

static ret_t ui_builder_default_on_widget_prop_end(ui_builder_t* b) {
  (void)b;
  return RET_OK;
}

static ret_t ui_builder_default_on_widget_end(ui_builder_t* b) {
  b->widget = b->widget->parent;

  return RET_OK;
}

static ui_builder_t s_ui_builder;

ui_builder_t* ui_builder_default() {
  memset(&s_ui_builder, 0x00, sizeof(ui_builder_t));

  s_ui_builder.on_widget_start = ui_builder_default_on_widget_start;
  s_ui_builder.on_widget_prop = ui_builder_default_on_widget_prop;
  s_ui_builder.on_widget_prop_end = ui_builder_default_on_widget_prop_end;
  s_ui_builder.on_widget_end = ui_builder_default_on_widget_end;

  return &s_ui_builder;
}

widget_t* window_open(const char* name) {
  ui_loader_t* loader = default_ui_loader();
  ui_builder_t* builder = ui_builder_default();
  const resource_info_t* ui = resource_manager_ref(RESOURCE_TYPE_UI, name);
  return_value_if_fail(ui != NULL, NULL);

  ui_loader_load(loader, ui->data, ui->size, builder);
  resource_manager_unref(ui);

  return builder->root;
}

widget_t* dialog_open(const char* name) { return window_open(name); }