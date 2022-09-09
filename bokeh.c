/* This file is an image processing operation for GEGL
 *
 * GEGL is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * GEGL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GEGL; if not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright 2006 Øyvind Kolås <pippin@gimp.org>
 * 2022 Beaver, GEGL Bokeh 
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES

enum_start (gegl_median_blur_neighborhoodo)
  enum_value (GEGL_MEDIAN_BLUR_NEIGHBORHOOD_SQUAREo,  "square",  N_("Square"))
  enum_value (GEGL_MEDIAN_BLUR_NEIGHBORHOOD_CIRCLEo,  "circle",  N_("Circle"))
  enum_value (GEGL_MEDIAN_BLUR_NEIGHBORHOOD_DIAMONDo, "diamond", N_("Diamond"))
enum_end (GeglMedianBlurNeighborhoodo)


property_enum (neighborhood, _("Shape"),
               GeglMedianBlurNeighborhoodo, gegl_median_blur_neighborhoodo,
               GEGL_MEDIAN_BLUR_NEIGHBORHOOD_CIRCLEo)
  description (_("Neighborhood type"))


property_color  (mcol, _("Color of the Bokeh"), "#ffffff")


property_double  (scale, _("Amount of Bokeh shapes"), 0.12)
    description  (_("The scale of the noise function"))
    value_range  (0.050, 0.35)

property_double  (shape, _("Clarity of Background Bokehs"), 3.0)
    description  (_("Interpolate between Manhattan and Euclidean distance."))
    value_range  (0.0, 4.0)

property_seed    (seed, _("Random seed"), rand)
    description  (_("The random seed for the noise function"))


property_color (hvalue, _("Color"), "#ff23cf")
    /* TRANSLATORS: the string 'black' should not be translated */
    ui_meta     ("role", "output-extent")


property_int  (median, _("Internal Median to increase Bokeh Size"), 25)
  value_range (1, 80)
  ui_range    (1, 80)
  ui_meta     ("unit", "pixel-distance")
  description (_("Neighborhood radius, a negative value will calculate with inverted percentiles"))

property_double  (percentile, _("Internal Median Blur Percentile"), 100)
  value_range (31, 100)
  description (_("Neighborhood color percentile"))
    ui_meta     ("role", "output-extent")



property_color (pink, _("Color"), "#ff23cf")
    description(_("The color to make transparent."))
    ui_meta     ("role", "output-extent")

property_double (opacity, _("Opacity Increase/Decrease (use if native opacity bar doesn't work)"), 0.6)
    description (_("Global opacity value that is always used on top of the optional auxiliary input buffer."))
    value_range (0.1, 1.3)
    ui_range    (0.1, 1.3)

property_double (transparency, _("Transparency threshold"), 0.5)
    description(_("The limit below which colors become transparent."))
    value_range (0.5, 0.6)
    ui_meta     ("role", "output-extent")

property_int (lens, _("Blur"), 2)
   description(_("Blurring tiny Bokehs will make it snow"))
   value_range (0, 12)
   ui_range    (0, 12)
   ui_gamma   (1.5)



#else

#define GEGL_OP_META
#define GEGL_OP_NAME     bokeh
#define GEGL_OP_C_SOURCE bokeh.c

#include "gegl-op.h"

static void attach (GeglOperation *operation)
{
  GeglNode *gegl = operation->node;
  GeglNode *input, *output, *c2a, *color, *opacity, *median, *divide,  *mcol, *coloroverlay, *blur, *noise;

  input    = gegl_node_get_input_proxy (gegl, "input");
  output   = gegl_node_get_output_proxy (gegl, "output");


 
  c2a = gegl_node_new_child (gegl,
                                  "operation", "gegl:color-to-alpha",
                                  NULL);
color = gegl_node_new_child (gegl,
                                  "operation", "gegl:color",
                                  NULL);
opacity = gegl_node_new_child (gegl,
                                  "operation", "gegl:opacity",
                                  NULL);
median = gegl_node_new_child (gegl,
                                  "operation", "gegl:median-blur",
                                  NULL);
divide = gegl_node_new_child (gegl,
                                  "operation", "gegl:divide",
                                  NULL);
mcol = gegl_node_new_child (gegl,
                                  "operation", "gegl:multiply",
                                  NULL);

coloroverlay = gegl_node_new_child (gegl,
                                  "operation", "gegl:color-overlay",
                                  NULL);
noise = gegl_node_new_child (gegl,
                                  "operation", "gegl:cell-noise",
                                  NULL);

blur = gegl_node_new_child (gegl,
                                  "operation", "gegl:lens-blur",
                                  NULL);




  gegl_node_link_many (input, color, divide, median, c2a, mcol, opacity, blur, output, NULL);
gegl_node_link_many (input, noise, NULL);
gegl_node_link_many (input, coloroverlay, NULL);
  gegl_node_connect_from (mcol, "aux", coloroverlay, "output");
  gegl_node_connect_from (divide, "aux", noise, "output");







    gegl_operation_meta_redirect (operation, "value", color, "value");
    gegl_operation_meta_redirect (operation, "transparency", c2a, "transparency-threshold");
    gegl_operation_meta_redirect (operation, "pink", c2a, "color");
    gegl_operation_meta_redirect (operation, "opacity", opacity, "value");
    gegl_operation_meta_redirect (operation, "median", median, "radius");
    gegl_operation_meta_redirect (operation, "percentile", median, "percentile");
    gegl_operation_meta_redirect (operation, "hvalue", color, "value");
    gegl_operation_meta_redirect (operation, "mcol", coloroverlay, "value");
    gegl_operation_meta_redirect (operation, "neighborhood", median, "neighborhood");
    gegl_operation_meta_redirect (operation, "scale", noise, "scale");
    gegl_operation_meta_redirect (operation, "shape", noise, "shape");
    gegl_operation_meta_redirect (operation, "seed", noise, "seed");
    gegl_operation_meta_redirect (operation, "lens", blur, "radius");

}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class;

  operation_class = GEGL_OPERATION_CLASS (klass);

  operation_class->attach = attach;

  gegl_operation_class_set_keys (operation_class,
    "name",        "gegl:bokeh",
    "title",       _("Bokeh Effect -Requires Alpha Channel "),
    "categories",  "Generic",
    "reference-hash", "1a1210akk00k101x2001b2hc",
    "description", _("Create a fake bokeh effect using GEGL. For edits directly on top of the image without layers use the normal or other blending options (like overlay or softlight). If the image is glitching it is because your layer has no alpha channel. "
                     ""),
    NULL);
}

#endif
