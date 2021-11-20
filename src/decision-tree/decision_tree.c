/*
 * Copyright (c) 2019- Ibrahim Umit Akgun
 * Copyright (c) 2019- Erez Zadok
 * Copyright (c) 2019- Stony Brook University
 * Copyright (c) 2019- The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#include <decision_tree.h>
#include <kml_lib.h>
#include <utility.h>

typedef enum kml_dt_idx {
  feature_class = 0,
  relation = 1,
  value = 2,
} kml_dt_idx;

static void parse_dt_node_data(char *buffer, kml_dt_node *node);
static void parse_dt_leaf_data(char *buffer, kml_dt_node *node);

#ifdef DECISIONTREE_DEBUG
static void print_kml_dt_node(kml_dt_node *node) {
#ifndef KML_KERNEL
  printf(" node %d %f %s ", node->feature_idx, node->val,
         node->relation == lte ? "left" : "right");
#else
  char buffer[32];
  get_float_str(buffer, 32, node->val);
  printk(KERN_CONT " node %d %s %s ", node->feature_idx, buffer,
         node->relation == lte ? "left" : "right");
#endif
}
#endif

static int get_level(char *level_str) {
  int level = 0;

  while (*level_str != '\0') {
    if (*level_str++ == '|') {
      level++;
    }
  }

  return level - 1;
}

kml_dt *build_decision_tree_model_from_file(const char *model_path) {
  filep model_file = NULL;
  char data[LINE_BUF_SIZE], line[LINE_BUF_SIZE];
  unsigned long long offset = 0;
  char *line_break = NULL, *feature_start = NULL;
  int count = 0, retval = 0;
  kml_dt_node *add_node = NULL;
  kml_dt *dt;
  kml_dt_node *traverse_stack[10] = {NULL};
  int level = 0;
  kml_dt_node *traverse;

  dt = kml_calloc(1, sizeof(kml_dt));
  dt->root = NULL;

  model_file = kml_file_open(model_path, "r", O_RDONLY);

  // reading decision tree node information
  for (; count < 88; count++) {
    retval = kml_file_read(model_file, data, LINE_BUF_SIZE, &offset);
    line_break = strchr((char *)data, '\n');
#ifdef DECISIONTREE_DEBUG
#ifdef KML_KERNEL
    printk("read retval %d %lld\n", retval, offset);
#else
    printf("read retval %d %lld\n", retval, offset);
#endif
#endif
#ifndef KML_KERNEL
    offset -= (unsigned long long)((LINE_BUF_SIZE - 1) - (line_break - data));
#else
    offset -= (unsigned long long)((retval - 1) - (line_break - data));
#endif
#ifdef DECISIONTREE_DEBUG
#ifdef DECISIONTREE_DEBUG
#ifdef KML_KERNEL
    printk("offset retval %lld\n", offset);
#else
    printf("offset retval %lld\n", offset);
#endif
#endif
#endif
    kml_memset(line, 0, LINE_BUF_SIZE);
    strncpy(line, data, line_break - data);

    add_node = kml_calloc(1, sizeof(kml_dt_node));
    feature_start = strchr((char *)line, 'f');
    if (feature_start == NULL) {
      feature_start = strchr((char *)line, 'c');
      parse_dt_leaf_data(feature_start, add_node);
    } else {
      parse_dt_node_data(feature_start, add_node);
    }

    if (dt->root == NULL) {
      dt->root = add_node;
      traverse_stack[0] = add_node;
#ifdef DECISIONTREE_DEBUG
      kml_debug("added as root ");
      print_kml_dt_node(add_node);
#endif
    } else {
      level = get_level(line);
      traverse = traverse_stack[level - 1];
      // kml_assert(traverse != NULL);

      if (add_node->feature_idx == -1) {
        // leaf
        if (traverse->relation == lte) {
          traverse->left = add_node;
        } else {
          traverse->right = add_node;
        }
#ifdef DECISIONTREE_DEBUG
        kml_debug("added ");
        print_kml_dt_node(add_node);
        kml_debug("new leaf under ");
        print_kml_dt_node(traverse);
#endif
      } else {
        // node
        traverse = traverse_stack[level];
        if (traverse != NULL &&
            traverse->feature_idx == add_node->feature_idx &&
            traverse->val == add_node->val) {
          // replace
          traverse->relation = add_node->relation;
          kml_free(add_node);
#ifdef DECISIONTREE_DEBUG
          kml_debug("replaced to ");
          print_kml_dt_node(traverse);
#endif
        } else {
          traverse = traverse_stack[level - 1];
          if (traverse->relation == lte) {
            traverse->left = add_node;
          } else {
            traverse->right = add_node;
          }
          traverse_stack[level] = add_node;
#ifdef DECISIONTREE_DEBUG
          kml_debug("added ");
          print_kml_dt_node(add_node);
          kml_debug("new node under ");
          print_kml_dt_node(traverse);
#endif
        }
      }
    }
#ifdef DECISIONTREE_DEBUG
    kml_debug("\n");
#endif
  }

  kml_file_close(model_file);

  return dt;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(build_decision_tree_model_from_file);
#endif

static void parse_dt_node_data(char *buffer, kml_dt_node *node) {
  int idx = 0;
  char *token = NULL, *split = buffer, *separator = " ";
  char *feature_break;
  while ((token = strsep(&split, separator)) != NULL) {
    switch (idx) {
      case feature_class: {
        feature_break = strchr(token, '_');
        node->feature_idx = *(++feature_break) - '0';
        break;
      }
      case relation: {
        if (token[0] == '>') {
          node->relation = greater;
          token = strsep(&split, separator);
        } else {
          node->relation = lte;
        }
        break;
      }
      case value: {
        node->val = convert_string_to_float_single_dec(token);
        break;
      }
    }
    idx++;
  }
}

static void parse_dt_leaf_data(char *buffer, kml_dt_node *node) {
  int idx = 0;
  char *token = NULL, *split = buffer, *separator = " ";
  while ((token = strsep(&split, separator)) != NULL) {
    switch (idx) {
      case 0: {
        node->feature_idx = -1;
        break;
      }
      case 1: {
        node->val = convert_string_to_float_single_dec(token);
        break;
      }
    }
    idx++;
  }
}

static void pretty_print_dt_node(kml_dt_node *node, int level) {
  int i = 0;
  char buffer[64] = {0};
  char float_buf[32] = {0};

  for (; i < level; ++i) {
    kml_debug("|   ");
  }
  get_float_str(float_buf, 32, node->val);
  snprintf(buffer, 64, "feature_%d %s\n", node->feature_idx, float_buf);
  kml_debug(buffer);
}

void print_kml_decision_tree(kml_dt_node *node) {
  static int level = 0;

  if (node == NULL) {
    return;
  }

  level++;
  pretty_print_dt_node(node, level);
  print_kml_decision_tree(node->left);
  print_kml_decision_tree(node->right);
  level--;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(print_kml_decision_tree);
#endif

// only for classification for now
// that is why it is returning int
int predict_decision_tree(kml_dt *dt, matrix *data_row) {
  kml_dt_node *traverse = dt->root;
  float compare = 0;

  while (traverse->feature_idx != -1) {
    compare = data_row->vals.f[mat_index(data_row, 0, traverse->feature_idx)];
    if (compare <= traverse->val) {
      traverse = traverse->left;
    } else {
      traverse = traverse->right;
    }
  }

  return (int)traverse->val;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(predict_decision_tree);
#endif
