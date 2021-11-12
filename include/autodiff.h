/*
 * Copyright (c) 2019- Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Ali Selman Aydin
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#ifndef AUTODIFF_H
#define AUTODIFF_H

#include <layers.h>
#include <matrix.h>

matrix *autodiff_forward(layers *layer_list, matrix *input);
void autodiff_backward(layers *layer_list, matrix *loss_derivative);
void cleanup_autodiff(layers *layer_list);

#endif
