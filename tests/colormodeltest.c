#include <glib-object.h>
#include "gegl.h"
#include "ctest.h"
#include "csuite.h"
#include <string.h>

static GeglColorSpace * rgb_color_space; 
static GeglColorSpace * gray_color_space; 

static GeglDataSpace * float_data_space; 
static GeglDataSpace * u8_data_space; 

static void
test_color_model_g_object_new(Test *test)
{
  {
    GeglColorModel * color_model = g_object_new(GEGL_TYPE_COMPONENT_COLOR_MODEL, 
                                                "color_space", rgb_color_space,
                                                "data_space", float_data_space,
                                                NULL);

    ct_test(test, color_model != NULL);
    ct_test(test, GEGL_IS_COMPONENT_COLOR_MODEL(color_model));
    ct_test(test, GEGL_IS_COLOR_MODEL(color_model));
    ct_test(test, g_type_parent(GEGL_TYPE_COLOR_MODEL) == GEGL_TYPE_OBJECT);
    ct_test(test, g_type_parent(GEGL_TYPE_COMPONENT_COLOR_MODEL) == GEGL_TYPE_COLOR_MODEL);
    ct_test(test, !strcmp("GeglColorModel", g_type_name(GEGL_TYPE_COLOR_MODEL)));
    ct_test(test, !strcmp("GeglComponentColorModel", g_type_name(GEGL_TYPE_COMPONENT_COLOR_MODEL)));

    g_object_unref(color_model);
  }
}

static void
test_color_model_pixel_rgb_float(Test *test)
{
  {
    gint * bits_per_channel;
    gchar **channel_names;
    gchar *name;

    GeglColorModel * color_model = g_object_new(GEGL_TYPE_COMPONENT_COLOR_MODEL, 
                                                "color_space", rgb_color_space,
                                                "data_space", float_data_space,
                                                NULL);


    ct_test(test, 96 == gegl_color_model_bits_per_pixel(color_model));
    ct_test(test, 3 == gegl_color_model_num_channels(color_model));
    ct_test(test, 3 == gegl_color_model_num_colors(color_model));
    ct_test(test, FALSE == gegl_color_model_has_alpha(color_model));
    ct_test(test, FALSE == gegl_color_model_has_z(color_model));

    name = gegl_color_model_name(color_model);
    ct_test(test, !strcmp("rgb-float", name));

    channel_names = gegl_color_model_channel_names(color_model);

    ct_test(test, !strcmp("red", channel_names[0]));
    ct_test(test, !strcmp("green", channel_names[1]));
    ct_test(test, !strcmp("blue", channel_names[2]));

    bits_per_channel = gegl_color_model_bits_per_channel(color_model);

    ct_test(test, 32 == bits_per_channel[0]);
    ct_test(test, 32 == bits_per_channel[1]);
    ct_test(test, 32 == bits_per_channel[2]);

    g_object_unref(color_model);
  }
}

static void
test_color_model_rgba_float(Test *test)
{
  {
    gint * bits_per_channel;
    gchar **channel_names;
    gchar *name;

    GeglColorModel * color_model = g_object_new(GEGL_TYPE_COMPONENT_COLOR_MODEL, 
                                                "color_space", rgb_color_space,
                                                "data_space", float_data_space,
                                                "has_alpha", TRUE,
                                                NULL);


    ct_test(test, 128 == gegl_color_model_bits_per_pixel(color_model));
    ct_test(test, 4 == gegl_color_model_num_channels(color_model));
    ct_test(test, 3 == gegl_color_model_num_colors(color_model));
    ct_test(test, TRUE == gegl_color_model_has_alpha(color_model));
    ct_test(test, FALSE == gegl_color_model_has_z(color_model));

    name = gegl_color_model_name(color_model);
    ct_test(test, !strcmp("rgba-float", name));

    channel_names = gegl_color_model_channel_names(color_model);

    ct_test(test, !strcmp("red", channel_names[0]));
    ct_test(test, !strcmp("green", channel_names[1]));
    ct_test(test, !strcmp("blue", channel_names[2]));
    ct_test(test, !strcmp("alpha", channel_names[3]));

    bits_per_channel = gegl_color_model_bits_per_channel(color_model);

    ct_test(test, 32 == bits_per_channel[0]);
    ct_test(test, 32 == bits_per_channel[1]);
    ct_test(test, 32 == bits_per_channel[2]);
    ct_test(test, 32 == bits_per_channel[3]);

    g_object_unref(color_model);
  }
}


static void
test_color_model_gray_float(Test *test)
{
  {
    gint * bits_per_channel;
    gchar **channel_names;
    gchar *name;

    GeglColorModel * color_model = g_object_new(GEGL_TYPE_COMPONENT_COLOR_MODEL, 
                                                "color_space", gray_color_space,
                                                "data_space", float_data_space,
                                                NULL);


    ct_test(test, 32 == gegl_color_model_bits_per_pixel(color_model));
    ct_test(test, 1 == gegl_color_model_num_channels(color_model));
    ct_test(test, 1 == gegl_color_model_num_colors(color_model));
    ct_test(test, FALSE == gegl_color_model_has_alpha(color_model));
    ct_test(test, FALSE == gegl_color_model_has_z(color_model));

    name = gegl_color_model_name(color_model);
    ct_test(test, !strcmp("gray-float", name));

    channel_names = gegl_color_model_channel_names(color_model);

    ct_test(test, !strcmp("gray", channel_names[0]));

    bits_per_channel = gegl_color_model_bits_per_channel(color_model);

    ct_test(test, 32 == bits_per_channel[0]);

    g_object_unref(color_model);
  }
}

static void
test_color_model_graya_float(Test *test)
{
  {
    gint * bits_per_channel;
    gchar **channel_names;
    gchar *name;

    GeglColorModel * color_model = g_object_new(GEGL_TYPE_COMPONENT_COLOR_MODEL, 
                                                "color_space", gray_color_space,
                                                "data_space", float_data_space,
                                                "has_alpha", TRUE,
                                                NULL);

    ct_test(test, 64 == gegl_color_model_bits_per_pixel(color_model));
    ct_test(test, 2 == gegl_color_model_num_channels(color_model));
    ct_test(test, 1 == gegl_color_model_num_colors(color_model));
    ct_test(test, TRUE == gegl_color_model_has_alpha(color_model));
    ct_test(test, FALSE == gegl_color_model_has_z(color_model));

    name = gegl_color_model_name(color_model);
    ct_test(test, !strcmp("graya-float", name));

    channel_names = gegl_color_model_channel_names(color_model);

    ct_test(test, !strcmp("gray", channel_names[0]));
    ct_test(test, !strcmp("alpha", channel_names[1]));

    bits_per_channel = gegl_color_model_bits_per_channel(color_model);

    ct_test(test, 32 == bits_per_channel[0]);
    ct_test(test, 32 == bits_per_channel[1]);

    g_object_unref(color_model);
  }
}

static void
test_color_model_rgb_u8(Test *test)
{
  {
    gint * bits_per_channel;
    gchar **channel_names;
    gchar *name;

    GeglColorModel * color_model = g_object_new(GEGL_TYPE_COMPONENT_COLOR_MODEL, 
                                                "color_space", rgb_color_space,
                                                "data_space", u8_data_space,
                                                NULL);


    ct_test(test, 24 == gegl_color_model_bits_per_pixel(color_model));
    ct_test(test, 3 == gegl_color_model_num_channels(color_model));
    ct_test(test, 3 == gegl_color_model_num_colors(color_model));
    ct_test(test, FALSE == gegl_color_model_has_alpha(color_model));
    ct_test(test, FALSE == gegl_color_model_has_z(color_model));

    name = gegl_color_model_name(color_model);
    ct_test(test, !strcmp("rgb-u8", name));

    channel_names = gegl_color_model_channel_names(color_model);

    ct_test(test, !strcmp("red", channel_names[0]));
    ct_test(test, !strcmp("green", channel_names[1]));
    ct_test(test, !strcmp("blue", channel_names[2]));

    bits_per_channel = gegl_color_model_bits_per_channel(color_model);

    ct_test(test, 8 == bits_per_channel[0]);
    ct_test(test, 8 == bits_per_channel[1]);
    ct_test(test, 8 == bits_per_channel[2]);

    g_object_unref(color_model);
  }
}

static void
test_color_model_rgba_u8(Test *test)
{
  {
    gint * bits_per_channel;
    gchar **channel_names;
    gchar *name;

    GeglColorModel * color_model = g_object_new(GEGL_TYPE_COMPONENT_COLOR_MODEL, 
                                                "color_space", rgb_color_space,
                                                "data_space", u8_data_space,
                                                "has_alpha", TRUE,
                                                NULL);


    ct_test(test, 32 == gegl_color_model_bits_per_pixel(color_model));
    ct_test(test, 4 == gegl_color_model_num_channels(color_model));
    ct_test(test, 3 == gegl_color_model_num_colors(color_model));
    ct_test(test, TRUE == gegl_color_model_has_alpha(color_model));
    ct_test(test, FALSE == gegl_color_model_has_z(color_model));

    name = gegl_color_model_name(color_model);
    ct_test(test, !strcmp("rgba-u8", name));

    channel_names = gegl_color_model_channel_names(color_model);

    ct_test(test, !strcmp("red", channel_names[0]));
    ct_test(test, !strcmp("green", channel_names[1]));
    ct_test(test, !strcmp("blue", channel_names[2]));
    ct_test(test, !strcmp("alpha", channel_names[3]));

    bits_per_channel = gegl_color_model_bits_per_channel(color_model);

    ct_test(test, 8 == bits_per_channel[0]);
    ct_test(test, 8 == bits_per_channel[1]);
    ct_test(test, 8 == bits_per_channel[2]);
    ct_test(test, 8 == bits_per_channel[3]);

    g_object_unref(color_model);
  }
}


static void
test_color_model_gray_u8(Test *test)
{
  {
    gint * bits_per_channel;
    gchar **channel_names;
    gchar *name;

    GeglColorModel * color_model = g_object_new(GEGL_TYPE_COMPONENT_COLOR_MODEL, 
                                                "color_space", gray_color_space,
                                                "data_space", u8_data_space,
                                                NULL);


    ct_test(test, 8 == gegl_color_model_bits_per_pixel(color_model));
    ct_test(test, 1 == gegl_color_model_num_channels(color_model));
    ct_test(test, 1 == gegl_color_model_num_colors(color_model));
    ct_test(test, FALSE == gegl_color_model_has_alpha(color_model));
    ct_test(test, FALSE == gegl_color_model_has_z(color_model));

    name = gegl_color_model_name(color_model);
    ct_test(test, !strcmp("gray-u8", name));

    channel_names = gegl_color_model_channel_names(color_model);

    ct_test(test, !strcmp("gray", channel_names[0]));

    bits_per_channel = gegl_color_model_bits_per_channel(color_model);

    ct_test(test, 8 == bits_per_channel[0]);

    g_object_unref(color_model);
  }
}

static void
test_color_model_graya_u8(Test *test)
{
  {
    gint * bits_per_channel;
    gchar **channel_names;
    gchar *name;

    GeglColorModel * color_model = g_object_new(GEGL_TYPE_COMPONENT_COLOR_MODEL, 
                                                "color_space", gray_color_space,
                                                "data_space", u8_data_space,
                                                "has_alpha", TRUE,
                                                NULL);

    ct_test(test, 16 == gegl_color_model_bits_per_pixel(color_model));
    ct_test(test, 2 == gegl_color_model_num_channels(color_model));
    ct_test(test, 1 == gegl_color_model_num_colors(color_model));
    ct_test(test, TRUE == gegl_color_model_has_alpha(color_model));
    ct_test(test, FALSE == gegl_color_model_has_z(color_model));

    name = gegl_color_model_name(color_model);
    ct_test(test, !strcmp("graya-u8", name));

    channel_names = gegl_color_model_channel_names(color_model);

    ct_test(test, !strcmp("gray", channel_names[0]));
    ct_test(test, !strcmp("alpha", channel_names[1]));

    bits_per_channel = gegl_color_model_bits_per_channel(color_model);

    ct_test(test, 8 == bits_per_channel[0]);
    ct_test(test, 8 == bits_per_channel[1]);

    g_object_unref(color_model);
  }
}

static void
test_color_model_create_storage(Test *test)
{
  {
    GeglColorModel * color_model = g_object_new(GEGL_TYPE_COMPONENT_COLOR_MODEL, 
                                                "color_space", rgb_color_space,
                                                "data_space", float_data_space,
                                                "has_alpha" , TRUE,
                                                NULL);

    GeglStorage * storage = gegl_color_model_create_storage(color_model, 10, 10);

    ct_test(test, 4 == gegl_storage_data_type_bytes(storage));
    ct_test(test, 4 == gegl_storage_num_bands(storage));
    ct_test(test, 10 == gegl_storage_width(storage));
    ct_test(test, 10 == gegl_storage_height(storage));

    ct_test(test, 4 == gegl_component_storage_num_banks(GEGL_COMPONENT_STORAGE(storage)));

    g_object_unref(storage);
    g_object_unref(color_model);
  }
}

static void
test_color_model_create_storage_create_data_buffer(Test *test)
{
  {
    GeglColorModel * color_model = g_object_new(GEGL_TYPE_COMPONENT_COLOR_MODEL, 
                                                "color_space", rgb_color_space,
                                                "data_space", float_data_space,
                                                "has_alpha" , TRUE,
                                                NULL);

    GeglStorage * storage = gegl_color_model_create_storage(color_model, 2, 2);
    GeglDataBuffer *data_buffer = gegl_storage_create_data_buffer(storage);

    ct_test(test, 4 == gegl_data_buffer_num_banks(data_buffer));
    ct_test(test, 16 == gegl_data_buffer_bytes_per_bank(data_buffer));
    ct_test(test, 64 == gegl_data_buffer_total_bytes(data_buffer));
    ct_test(test, NULL != gegl_data_buffer_banks_data(data_buffer));

    g_object_unref(storage);
    g_object_unref(data_buffer);
    g_object_unref(color_model);
  }
}

static void
color_model_test_setup(Test *test)
{
 rgb_color_space = g_object_new(GEGL_TYPE_COLOR_SPACE_RGB, NULL); 
 gray_color_space = g_object_new(GEGL_TYPE_COLOR_SPACE_GRAY, NULL); 

 float_data_space = g_object_new(GEGL_TYPE_DATA_SPACE_FLOAT, NULL); 
 u8_data_space = g_object_new(GEGL_TYPE_DATA_SPACE_U8, NULL); 
}

static void
color_model_test_teardown(Test *test)
{
  g_object_unref(rgb_color_space);
  g_object_unref(gray_color_space);

  g_object_unref(float_data_space);
  g_object_unref(u8_data_space);
}

Test *
create_color_model_test()
{
  Test* t = ct_create("GeglColorModelTest");

  g_assert(ct_addSetUp(t, color_model_test_setup));
  g_assert(ct_addTearDown(t, color_model_test_teardown));
  g_assert(ct_addTestFun(t, test_color_model_g_object_new));

  g_assert(ct_addTestFun(t, test_color_model_pixel_rgb_float));
  g_assert(ct_addTestFun(t, test_color_model_rgba_float));
  g_assert(ct_addTestFun(t, test_color_model_gray_float));
  g_assert(ct_addTestFun(t, test_color_model_graya_float));

  g_assert(ct_addTestFun(t, test_color_model_rgb_u8));
  g_assert(ct_addTestFun(t, test_color_model_rgba_u8));
  g_assert(ct_addTestFun(t, test_color_model_gray_u8));
  g_assert(ct_addTestFun(t, test_color_model_graya_u8));

  g_assert(ct_addTestFun(t, test_color_model_create_storage));
  g_assert(ct_addTestFun(t, test_color_model_create_storage_create_data_buffer));

  return t; 
}
