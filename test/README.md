# What is this

# Setup

# Example

The following example describes how to run wtperf and induce errors with fault inject count from 1 to 200. I am going to set 30 secs as timeout, because I know with faults induced it will error out much before that:

```
~/work/wt-32/build_posix$ python ../test/suite/run_fi.py -b 1 -e 200 -t 30 "./bench/wtperf/wtperf -O ../bench/wtperf/runners/medium-btree.wtperf -o verbose=2"
[./bench/wtperf/wtperf -O ../bench/wtperf/runners/medium-btree.wtperf -o verbose=2 , fi_count: 1]  ..  [PASS]
[./bench/wtperf/wtperf -O ../bench/wtperf/runners/medium-btree.wtperf -o verbose=2 , fi_count: 2]  ..  [PASS]
[./bench/wtperf/wtperf -O ../bench/wtperf/runners/medium-btree.wtperf -o verbose=2 , fi_count: 3]  ..  [PASS]
[./bench/wtperf/wtperf -O ../bench/wtperf/runners/medium-btree.wtperf -o verbose=2 , fi_count: 4]  ..  [PASS]
.
.
[./bench/wtperf/wtperf -O ../bench/wtperf/runners/medium-btree.wtperf -o verbose=2 , fi_count: 110]  ..  [PASS]
[./bench/wtperf/wtperf -O ../bench/wtperf/runners/medium-btree.wtperf -o verbose=2 , fi_count: 111]  ..  [PASS]
[./bench/wtperf/wtperf -O ../bench/wtperf/runners/medium-btree.wtperf -o verbose=2 , fi_count: 112]  ..  [PASS]
Saving the generated logs and config in dir: ~/work/wt-32/build_posix/FI_TEST.26350/
[./bench/wtperf/wtperf -O ../bench/wtperf/runners/medium-btree.wtperf -o verbose=2 , fi_count: 113]  ..  [FAIL]
```

It gets stuck for a while at count 113, and when it times-out, it gets backtraces (using poor mans profiler) and then kills wtperf collecting a core.
Tells the directory where all that information would be. :ooking inside the directory:

```
~/work/wt-32/build_posix$ cd ~/work/wt-32/build_posix/FI_TEST.26350/
~/work/wt-32/build_posix/FI_TEST.26350$ ls
total 2552
-rw-rw-r--. 1 sulabh sulabh      143 Apr  7 17:42 config_26350.fi
-rw-------. 1 sulabh sulabh 35274752 Apr  7 17:42 core.wtperf.26350
-rw-rw-r--. 1 sulabh sulabh      864 Apr  7 17:42 fi_pid_26350_close.log
-rw-rw-r--. 1 sulabh sulabh      920 Apr  7 17:42 fi_pid_26350_fdatasync.log
-rw-rw-r--. 1 sulabh sulabh      216 Apr  7 17:42 fi_pid_26350_ftruncate.log
-rw-rw-r--. 1 sulabh sulabh     1025 Apr  7 17:42 fi_pid_26350_hung_btt.log
-rw-rw-r--. 1 sulabh sulabh     1027 Apr  7 17:42 fi_pid_26350_inject_bt.log
-rw-rw-r--. 1 sulabh sulabh      200 Apr  7 17:42 fi_pid_26350_mmap.log
-rw-rw-r--. 1 sulabh sulabh     2102 Apr  7 17:42 fi_pid_26350_open.log
-rw-rw-r--. 1 sulabh sulabh      836 Apr  7 17:42 fi_pid_26350_pread.log
-rw-rw-r--. 1 sulabh sulabh     1577 Apr  7 17:42 fi_pid_26350_pwrite.log
-rw-rw-r--. 1 sulabh sulabh      527 Apr  7 17:42 fi_pid_26350_rename.log
```

Looking at the content of the log files:

```
~/work/wt-32/build_posix/FI_TEST.26350$ cat fi_pid_26350_hung_btt.log
1:__session_close,__conn_close,start_run,start_all_runs,main
1:pthread_cond_timedwait@@GLIBC_2.3.2,__wt_cond_wait_signal,__wt_cond_wait,__sweep_server,start_thread,clone
1:pthread_cond_timedwait@@GLIBC_2.3.2,__wt_cond_wait_signal,__wt_cond_auto_wait_signal,__wt_cond_auto_wait,__wt_evict_thread_run,__wt_thread_run,start_thread,clone

1:__strcmp_sse2_unaligned,__session_close,__conn_close,start_run,start_all_runs,main
1:pthread_cond_timedwait@@GLIBC_2.3.2,__wt_cond_wait_signal,__wt_cond_wait,__sweep_server,start_thread,clone
1:pthread_cond_timedwait@@GLIBC_2.3.2,__wt_cond_wait_signal,__wt_cond_auto_wait_signal,__wt_cond_auto_wait,__wt_evict_thread_run,__wt_thread_run,start_thread,clone

1:pthread_cond_timedwait@@GLIBC_2.3.2,__wt_cond_wait_signal,__wt_cond_wait,__sweep_server,start_thread,clone
1:pthread_cond_timedwait@@GLIBC_2.3.2,__wt_cond_wait_signal,__wt_cond_auto_wait_signal,__wt_cond_auto_wait,__wt_evict_thread_run,__wt_thread_run,start_thread,clone
1:config=0x0),__conn_close,start_run,start_all_runs,main

~/work/wt-32/build_posix/FI_TEST.26350$ cat fi_pid_26350_inject_bt.log
Induced fault in following backtrace:
pwrite
__posix_file_write
__wt_write
__block_write_off
__wt_block_write_off
__wt_block_write
__bm_write
__wt_bt_write
__rec_split_write
__rec_split
__rec_split_crossing_bnd
__wt_bulk_insert_row
__curbulk_insert_row
populate_thread
start_thread
clone

Induced fault in following backtrace:
pwrite
__posix_file_write
__wt_write
__block_write_off
__wt_block_write_off
__wt_block_write
__bm_write
__wt_bt_write
__rec_split_write
__rec_split_finish_std
__rec_split_finish
__wt_bulk_wrapup
__curfile_close
__session_close
__conn_close
start_run
start_all_runs
main
__libc_start_main
_start

Induced fault in following backtrace:
ftruncate
__posix_file_truncate
__wt_ftruncate
__wt_block_truncate
__wt_block_extlist_truncate
__ckpt_process
__wt_block_checkpoint
__bm_checkpoint
__rec_write_wrapup
__wt_reconcile
__wt_evict_file
__wt_cache_op
__checkpoint_tree
__wt_checkpoint_close
__wt_conn_btree_sync_and_close
__wt_session_release_btree
__curfile_close
__session_close
__conn_close
start_run
```

# Reproducing

When a test run triggers a failure, information is saved to assist in reproducing the failure. In the example above the following is the configuration saved for a bug to be reproduced:

```
~/work/wt-32/build_posix/FI_TEST.26350$ cat config_26350.fi 
cmd=./bench/wtperf/wtperf -O ../bench/wtperf/runners/medium-btree.wtperf -o verbose=2
    failcountbeg=113
    failcountend=113
    timeout=30
```

The number in the directory is the process id, the file numbers are process/thread ids. With backtrace for each thread and fault inject records of each thread it might be useful
Now you can always use the generated config file to get this run:

```
~/work/wt-32/build_posix$ python ../test/suite/run_fi.py -c FI_TEST.26350/config_26350.fi 
Saving the generated logs and config in dir: ~/work/wt-32/build_posix/FI_TEST.115409/
[./bench/wtperf/wtperf -O ../bench/wtperf/runners/medium-btree.wtperf -o verbose=2 , fi_count: 113]  ..  [FAIL]
```

The Python program has other options as well:

```
-C : dumps config to specified file before doing a run
-i : ignore list, these fault injection counts are skipped in a given run
-v / -vv : Verbose output. vv dumps all the execution output on stdout too
```

## Configuration file format
Config file has the following structure, where N commands to test follow the parameters the test will run with. This block can repeat with another set of N' commands with a different set of test parameters:

```
cmd=command-1-t-exec
cmd=command-2-to-exec
...
cmd=command-n-to-exec
   failcountbeg=x
   failcountend=y
   failcountignore=a,b,c
   timeout=300
<another-block-of-n'-commands-followed-by-parameters>
```

For example, to run two sets of python commands with different parameters:

cmd=python ../test/suite/run.py -v 3 test_alter01
cmd=python ../test/suite/run.py -v 3 test_async01
cmd=python ../test/suite/run.py -v 3 test_async02
    failcountbeg=1
    failcountend=200
    timeout=30
cmd=python ../test/suite/run.py -v 3 test_async03
cmd=python ../test/suite/run.py -v 3 test_verify
cmd=python ../test/suite/run.py -v 3 test_version
    failcountbeg=1
    failcountend=100
    failcountignore=31,32,56 
    timeout=40


# Code layout

This is how I have structured the code:

Each command to run with its range of parameters (fail count range from beg to end, minus ignore) is called a Testset.
Each testset eventually represents a series of tests, represented by class Test, which is a single command to run with one particular failcount.
A class Testsuite maintains the list of Testset and executes them
Finally we have a Process class, which is responsible for the actual execution.

```
main() ---> Testsuite (has list of Testset) <--- Test  <--- Process 
```

Each of these classes encapsulates functionalities like they should.
For now it classifies two cases as failures:
   1. process is still running after timeout. (collect backtraces and then send SIGQUIT signal)
   2. A negative process exit code, which is not SIGABRT

It runs neatly, but for now you have to specify commands to execute either as arguments or through config file. It doesn't have a discovery of any kinds. I don't think discovery is important. I want this to blindly execute binaries it has been asked to.
It runs those python tests fine, but it can't for now assimilate cores from python tests.
