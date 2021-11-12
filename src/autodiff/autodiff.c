/*
 * Copyright (c) 2019- Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Ali Selman Aydin
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#include <autodiff.h>
#include <kml_lib.h>
#include <linear.h>
#include <sigmoid.h>

// #define AUTODIFF_DEBUG

matrix *autodiff_forward(layers *layer_list, matrix *input) {
  layer *current_layer = NULL;
  matrix *output = NULL;

  traverse_layers_forward(layer_list, current_layer) {
    switch (current_layer->type) {
      case LINEAR_LAYER: {
        output = linear_layer_functions.forward(input, current_layer->internal);
#ifdef AUTODIFF_DEBUG
        kml_debug("++++++++++++++++++++++++++++++++++++++++++++\n");
        kml_debug("linear layer forward pass output\n");
        print_matrix(output);
        kml_debug("--------------------------------------------\n");
#endif
        break;
      }
      case SIGMOID_LAYER: {
        output =
            sigmoid_layer_functions.forward(input, current_layer->internal);
#ifdef AUTODIFF_DEBUG
        kml_debug("++++++++++++++++++++++++++++++++++++++++++++\n");
        kml_debug("sigmoid layer forward pass output\n");
        print_matrix(output);
        kml_debug("--------------------------------------------\n");
#endif
        break;
      }
    }
    input = output;
  }

  return output;
}

void autodiff_backward(layers *layer_list, matrix *loss_derivative) {
  layer *current_layer;
  matrix *cumulative_derivative = NULL, *cumulative_derivative_updated = NULL;

  cumulative_derivative = loss_derivative;

  traverse_layers_backward(layer_list, current_layer) {
    switch (current_layer->type) {
      case LINEAR_LAYER: {
        linear_layer *linear_l = current_layer->internal;
        cumulative_derivative_updated =
            linear_layer_functions.backward(cumulative_derivative, linear_l);
#ifdef AUTODIFF_DEBUG
        kml_debug("++++++++++++++++++++++++++++++++++++++++++++\n");
        kml_debug("linear layer backward pass weight gradient\n");
        print_matrix(linear_l->gradient);
        kml_debug("bias gradient\n");
        print_matrix(linear_l->bias_gradient);
        kml_debug("--------------------------------------------\n");
#endif
        break;
      }
      case SIGMOID_LAYER: {
        sigmoid_layer *sigmoid_l = current_layer->internal;
        cumulative_derivative_updated =
            sigmoid_layer_functions.backward(cumulative_derivative, sigmoid_l);
#ifdef AUTODIFF_DEBUG
        kml_debug("++++++++++++++++++++++++++++++++++++++++++++\n");
        kml_debug("sigmoid layer backward pass weight gradient\n");
        print_matrix(sigmoid_l->gradient);
        kml_debug("--------------------------------------------\n");
#endif
        break;
      }
    }
    if (cumulative_derivative != NULL &&
        cumulative_derivative != cumulative_derivative_updated &&
        cumulative_derivative != loss_derivative) {
      free_matrix(cumulative_derivative);
    }
    cumulative_derivative = cumulative_derivative_updated;
  }
  free_matrix(cumulative_derivative);
}

void cleanup_autodiff(layers *layer_list) {
  layer *current_layer;
  traverse_layers_backward(layer_list, current_layer) {
    switch (current_layer->type) {
      case LINEAR_LAYER: {
        linear_layer *linear_l = current_layer->internal;
        free_matrix(linear_l->output);
        break;
      }
      case SIGMOID_LAYER: {
        sigmoid_layer *sigmoid_l = current_layer->internal;
        free_matrix(sigmoid_l->output);
        break;
      }
    }
  }
}
