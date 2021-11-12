// Copyright FSL Stony Brook

#include <linux/delay.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/module.h>
// for set_fs and get_fs
#include <asm/uaccess.h>
#include <kernel-interfaces/nfs.h>
#include <linux/blkdev.h>
#include <linux/fs.h>

// trace headers included
#include <kml_lib.h>
#include <linux/nfs_fs.h>
#include <nfs_net_classification.h>
#include <trace/events/filemap.h>
#include <trace/events/vmscan.h>
#include <trace/events/writeback.h>
#include <utility.h>

MODULE_AUTHOR("Umit Akgun");
MODULE_LICENSE("GPL");

static struct task_struct *kml_nfs_update_thread;
static struct task_struct *kml_perf_monitoring_thread;

enum kml_perf_stats {
  ops_interval = 0,
  ops_total = 1,
  ops_sec_interval = 2,
  ops_sec_total = 3,
  second_interval = 4,
  second_total = 5
} kml_perf_stats;

bool perf_monitoring_failed = false;
bool module_exiting = false;

void nfs_add_to_page_cache(unsigned long offset);

static const int readrandom_ranking[] = {4096};
static const int readreverse_ranking[] = {262144};
static const int readseq_ranking[] = {262144};
static const int readrandomwriterandom_ranking[] = {4096};
static const int *workload_rankigs[] = {readrandom_ranking,
                                        readrandomwriterandom_ranking,
                                        readseq_ranking, readreverse_ranking};
static const char workload_names[4][25] = {
    "readrandom", "readrandomwriterandom", "readseq", "readreverse"};

static nfs_class_net *nfs_net = NULL;
static const int n_features = 8;
static u64 start_time = 0;
static u64 data_process_total = 0;
static atomic_int data_process_count;
static u64 inference_timing_total = 0;
static atomic_int inference_count;
extern uint64_t kml_total_memory_usage;

#define LAST_N_SEC_OP 5
static long last_n_sec_perf[LAST_N_SEC_OP] = {0};
static int last_n_sec_perf_idx = 0;

static unsigned int current_rsize;

static unsigned int workload_voting[4] = {0};

int nfs_update(void *data) {
  int class = 0;
  u64 inference_start = 0, inference_end = 0;
  int log_idx = 0;
  long average_ops_sec = 0;
  int seconds = 0;
  int i = 0, max_idx = 0, max_voting = 0;
  filep rsize_file;
  char buffer[7] = {0};
  unsigned long long offset = 0;

  current_rsize = nfs_get_current_rsize();

  while (1) {
    msleep(1000);
    if (kthread_should_stop()) {
      printk("nfs_update thread should stop\n");
      break;
    }
    printk("----------------------- %d -----------------------", ++seconds);

    kernel_fpu_begin();
    inference_start = kml_get_current_time();
    class = predict_nfs_class(nfs_net, current_rsize);
    inference_end = kml_get_current_time();
    kernel_fpu_end();
    // current_rsize = workload_rankigs[class][0];
    // nfs_set_current_rsize(current_rsize);
    workload_voting[class]++;
    max_idx = 0;
    max_voting = 0;
    for (i = 0; i < 4; ++i) {
      if (workload_voting[i] > max_voting) {
        max_voting = workload_voting[i];
        max_idx = i;
      }
    }
    current_rsize = workload_rankigs[max_idx][0];
    printk("rsize set:\t\t %d\n", current_rsize);

    kml_memset(buffer, ' ', 7);
    snprintf(buffer, 7, "%d", current_rsize);
    buffer[6] = '\n';
    rsize_file = kml_file_open("/tmp/rsize", "w+", O_RDWR);

    kml_file_write(rsize_file, buffer, 7, &offset);

    kml_file_close(rsize_file);
    offset = 0;

    printk("predicted class:\t\t %s\n", workload_names[class]);
    inference_timing_total += kml_get_time_diff(inference_end, inference_start);
    kml_atomic_add(&inference_count, 1);
    printk("inference took in avg.:\t\t %lld\n",
           inference_timing_total / kml_atomic_int_read(&inference_count));

    if (kml_atomic_int_read(&data_process_count) > 0) {
      printk("data processing took in avg.:\t %lld\n",
             data_process_total / kml_atomic_int_read(&data_process_count));
    }
    printk("kml total memory usage:\t\t %lld\n", kml_total_memory_usage);
#ifdef LOG_LAST_N
    printk(KERN_CONT "perf [ ");
#endif
    average_ops_sec = 0;
    for (log_idx = 0; log_idx < LAST_N_SEC_OP; ++log_idx) {
#ifdef LOG_LAST_N
      printk(KERN_CONT "%ld ", last_n_sec_perf[log_idx]);
#endif
      average_ops_sec += last_n_sec_perf[log_idx];
    }
#ifdef LOG_LAST_N
    printk(KERN_CONT "] -> %ld\n", average_ops_sec / LAST_N_SEC_OP);
#else
    printk("ops/sec average:\t\t %ld\n", average_ops_sec / LAST_N_SEC_OP);
#endif
  }

  return 0;
}

int perf_monitoring(void *data) {
  size_t read_size;
  loff_t offset = 0;
  char buffer[256] = {0};
  long ops_interval_stat = 0, ops_total_stat = 0, ops_sec_interval_stat = 0,
       ops_sec_total_stat = 0, second_interval_stat = 0, second_total_stat = 0;

  struct file *perf_file = kml_file_open("/tmp/detail.txt", NULL, O_LARGEFILE);

  if (perf_file == NULL) {
    perf_monitoring_failed = true;
    printk("kml nfs perf thread could not find perf file\n");
    return 0;
  }

  while (1) {
    if ((read_size = kernel_read(perf_file, buffer, 256, &offset)) == 0) {
      if (read_size == 0) {
        msleep(1000);
      }
    } else {
      int idx = 0, result = 0;
      char *token = NULL, *long_token = NULL, *split = buffer, *separator = " ";
      while ((token = strsep(&split, separator)) != NULL) {
        // printk("%d %s %d\n", idx, token, result);
        switch (idx) {
          case ops_interval: {
            result |= kstrtol(token, 10, &ops_interval_stat);
            break;
          }
          case ops_total: {
            result |= kstrtol(token, 10, &ops_total_stat);
            break;
          }
          case ops_sec_interval: {
            long_token = strsep(&token, ".");
            result |= kstrtol(long_token, 10, &ops_sec_interval_stat);
            break;
          }
          case ops_sec_total: {
            long_token = strsep(&token, ".");
            result |= kstrtol(long_token, 10, &ops_sec_total_stat);
            break;
          }
          case second_interval: {
            long_token = strsep(&token, ".");
            result |= kstrtol(long_token, 10, &second_interval_stat);
            break;
          }
          case second_total: {
            long_token = strsep(&token, ".");
            result |= kstrtol(long_token, 10, &second_total_stat);
            break;
          }
        }
        idx++;
      }
      if (result == 0) {
        // printk("kml perf stat ops/sec: %ld\n", ops_sec_interval_stat);
        last_n_sec_perf[last_n_sec_perf_idx] = ops_sec_interval_stat;
        last_n_sec_perf_idx = (last_n_sec_perf_idx + 1) % 5;
      }
    }

    if (kthread_should_stop()) {
      printk("nfs_update thread should stop\n");
      break;
    }
  }

  kml_file_close(perf_file);
  return 0;
}

void nfs_add_to_page_cache(unsigned long offset) {
  u64 time_passed;
  u64 data_process_start, data_process_end;
  double data[6];

  time_passed = kml_get_current_time();
  time_passed = kml_get_time_diff(time_passed, start_time);

  if (module_exiting) return;

  kernel_fpu_begin();
  kml_memset(data, 0, sizeof(double) * 6);
  data[0] = time_passed / 1000;
  data[1] = 1;
  data[3] = (double)offset;

  data_process_start = kml_get_current_time();
  if (nfs_data_processing(data, nfs_net, current_rsize, 1, true, false)) {
  }
  data_process_end = kml_get_current_time();

  data_process_total += kml_get_time_diff(data_process_end, data_process_start);
  kml_atomic_add(&data_process_count, 1);

  kernel_fpu_end();
}

void nfs_mm_vmscan_shrink(unsigned long nr_reclaimed) {
  u64 time_passed;
  u64 data_process_start, data_process_end;
  double data[6];

  time_passed = kml_get_current_time();
  time_passed = kml_get_time_diff(time_passed, start_time);

  if (module_exiting) return;

  kernel_fpu_begin();
  kml_memset(data, 0, sizeof(double) * 6);
  data[0] = time_passed / 1000;
  data[1] = 5;
  data[3] = (double)nr_reclaimed;

  data_process_start = kml_get_current_time();
  if (nfs_data_processing(data, nfs_net, current_rsize, 5, true, false)) {
  }
  data_process_end = kml_get_current_time();

  data_process_total += kml_get_time_diff(data_process_end, data_process_start);
  kml_atomic_add(&data_process_count, 1);

  kernel_fpu_end();
}

void trace_nfs4_read(u32 fhandle, loff_t offset) {
  u64 time_passed;
  u64 data_process_start, data_process_end;
  double data[6];

  time_passed = kml_get_current_time();
  time_passed = kml_get_time_diff(time_passed, start_time);

  if (module_exiting) return;

  kernel_fpu_begin();
  kml_memset(data, 0, sizeof(double) * 6);
  data[0] = time_passed / 1000;
  data[1] = 3;
  data[3] = fhandle;
  data[4] = offset;

  data_process_start = kml_get_current_time();
  if (nfs_data_processing(data, nfs_net, current_rsize, 3, true, false)) {
  }
  data_process_end = kml_get_current_time();

  data_process_total += kml_get_time_diff(data_process_end, data_process_start);
  kml_atomic_add(&data_process_count, 1);

  kernel_fpu_end();
}

void trace_nfs4_readdone(u32 fhandle) {
  u64 time_passed;
  u64 data_process_start, data_process_end;
  double data[6];

  time_passed = kml_get_current_time();
  time_passed = kml_get_time_diff(time_passed, start_time);

  if (module_exiting) return;

  kernel_fpu_begin();
  kml_memset(data, 0, sizeof(double) * 6);
  data[0] = time_passed / 1000;
  data[1] = 4;
  data[3] = fhandle;

  data_process_start = kml_get_current_time();
  if (nfs_data_processing(data, nfs_net, current_rsize, 4, true, false)) {
  }
  data_process_end = kml_get_current_time();

  data_process_total += kml_get_time_diff(data_process_end, data_process_start);
  kml_atomic_add(&data_process_count, 1);

  kernel_fpu_end();
}

static int __init kml_nfs_init(void) {
  nfs_model_config config;
  filep mean_file, stddev_file;
  matrix *mean_matrix, *stddev_matrix;
  kernel_fpu_begin();
  config.batch_size = 1;
  config.learning_rate = 0.01;
  config.momentum = 0.99;
  config.num_features = n_features;
  config.model_type = FLOAT;
  nfs_net = build_nfs_class_net(&config);
  // decision tree

  set_weights_biases_from_file(
      nfs_net->layer_list->layer_list_head,
      "/home/kml/"
      "ml-models-analyses/nfs-data-collection/nn_arch_data/linear0_w.csv",
      "/home/kml/"
      "ml-models-analyses/nfs-data-collection/nn_arch_data/linear0_bias.csv");
  set_weights_biases_from_file(nfs_net->layer_list->layer_list_head->next->next,
                               "/home/kml/ml-models-analyses/"
                               "nfs-data-collection/nn_arch_data/"
                               "linear1_w.csv",
                               "/home/kml/ml-models-analyses/"
                               "nfs-data-collection/nn_arch_data/"
                               "linear1_bias.csv");
  set_weights_biases_from_file(
      nfs_net->layer_list->layer_list_head->next->next->next->next,
      "/home/kml/ml-models-analyses/"
      "nfs-data-collection/nn_arch_data/linear2_w.csv",
      "/home/kml/ml-models-analyses/"
      "nfs-data-collection/nn_arch_data/"
      "linear2_bias.csv");
  set_weights_biases_from_file(
      nfs_net->layer_list->layer_list_tail,
      "/home/kml/"
      "ml-models-analyses/nfs-data-collection/nn_arch_data/linear2_w.csv",
      "/home/kml/"
      "ml-models-analyses/nfs-data-collection/nn_arch_data/linear2_bias.csv");

  mean_matrix = allocate_matrix(1, n_features, FLOAT);
  stddev_matrix = allocate_matrix(1, n_features, FLOAT);
  mean_file = kml_file_open(
      "/home/kml/ml-models-analyses/"
      "nfs-data-collection/"
      "nn_arch_data/mean.csv",
      "r", O_RDONLY);
  stddev_file = kml_file_open(
      "/home/kml/ml-models-analyses/"
      "nfs-data-collection/"
      "nn_arch_data/stddev.csv",
      "r", O_RDONLY);
  load_matrix_from_file(mean_file, mean_matrix);
  load_matrix_from_file(stddev_file, stddev_matrix);
  kml_file_close(mean_file);
  kml_file_close(stddev_file);

  set_nfs_data(&nfs_net->norm_data_stat, mean_matrix, stddev_matrix, 1202);

  // initializing decision tree
  // dt = build_decision_tree_model_from_file(
  //     "/home/umit/research/kernel-ml/kml/ml-models-analyses/readahead-per-disk/"
  //     "decision_tree_model/"
  //     "readahead_model.dt");
  // print_kml_decision_tree(dt->root);
  // class = predict_decision_tree(dt, row);
  kernel_fpu_end();

  udelay(1000);

  kml_nfs_update_thread = kthread_run(&nfs_update, NULL, "kml_nfs_update");
  kml_perf_monitoring_thread =
      kthread_run(&perf_monitoring, NULL, "kml_perf_monitoring");

  udelay(1000);

  set_trace_nfs_add_to_page_cache_fptr((void *)&nfs_add_to_page_cache);
  set_nfs_mm_vmscan_lru_shrink_inactive_fptr((void *)&nfs_mm_vmscan_shrink);
  set_trace_nfs4_read_fptr((void *)&trace_nfs4_read);
  set_trace_nfs4_readdone_fptr((void *)&trace_nfs4_readdone);

  kml_atomic_int_init(&data_process_count, 0);
  kml_atomic_int_init(&inference_count, 0);
  start_time = kml_get_current_time();

  udelay(1000);

  kml_atomic_int_init(&data_process_count, 0);
  kml_atomic_int_init(&inference_count, 0);
  start_time = kml_get_current_time();

  printk(KERN_WARNING "KML rocksdb nfs_update started");
  return 0;
}

static void __exit kml_nfs_exit(void) {
  kthread_stop(kml_nfs_update_thread);
  printk("kml readahed update thread stopped\n");
  if (!perf_monitoring_failed && kml_perf_monitoring_thread != NULL)
    kthread_stop(kml_perf_monitoring_thread);
  printk("kml readahed perf thread stopped\n");
  udelay(1000);
  module_exiting = true;
  udelay(1000);
  set_trace_nfs_add_to_page_cache_fptr(NULL);
  set_nfs_mm_vmscan_lru_shrink_inactive_fptr(NULL);
  set_trace_nfs4_read_fptr(NULL);
  set_trace_nfs4_readdone_fptr(NULL);
  udelay(1000);
  kernel_fpu_begin();
  clean_nfs_class_net(nfs_net);
  kernel_fpu_end();
  printk(KERN_WARNING "KML rocksdb nfs_update ended\n");
}

module_init(kml_nfs_init);
module_exit(kml_nfs_exit);
