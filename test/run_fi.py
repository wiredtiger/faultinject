#!/usr/bin/env python
#
# Public Domain 2014-2017 MongoDB, Inc.
# Public Domain 2008-2014 WiredTiger, Inc.
#
# This is free and unencumbered software released into the public domain.
#
# Anyone is free to copy, modify, publish, use, compile, sell, or
# distribute this software, either in source code form or as a compiled
# binary, for any purpose, commercial or non-commercial, and by any
# means.
#
# In jurisdictions that recognize copyright laws, the author or authors
# of this software dedicate any and all copyright interest in the
# software to the public domain. We make this dedication for the benefit
# of the public at large and to the detriment of our heirs and
# successors. We intend this dedication to be an overt act of
# relinquishment in perpetuity of all present and future rights to this
# software under copyright law.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# run_fi.py
#      Command line fault injection test runner
#

import os, sys, shlex, threading, time, signal
from subprocess import Popen, PIPE
from collections import namedtuple

DEF_FAULTINJECT_LIBRARY_NAME = '__wt'
CUR_DIR = os.getcwd()
PMP_PATH = os.path.dirname(os.path.abspath(__file__)) + '/pmp.sh'
DEF_PYTHON_PATH = CUR_DIR + '/../lang/python:'
DEF_PYTHON_PATH += CUR_DIR + '/lang/python:'
DEF_PYTHON_PATH += CUR_DIR + '/../test/suite'
FI_TMP_DIR = CUR_DIR + '/FI_TEST/'

verbose = 0

def usage():
    print 'Blah..'

def dbg(level, msg):
    if verbose >= level:
        print msg

class Testsuite(object):
    def __init__(self, proceed_on_failure, fi_lib_name,
        fi_ld_lib_path, fi_ld_load_loc, fi_python_path):

        self.fi_lib_name = fi_lib_name
        self.fi_ld_lib_path = fi_ld_lib_path
        self.fi_ld_load_loc =  fi_ld_load_loc
        self.proceed_on_failure = proceed_on_failure
        self.fi_python_path = fi_python_path

        self.test_env = self.generate_global_test_env()
        self.testset_list = []

    def generate_global_test_env(self):
        run_env = dict(os.environ, FAULTINJECT_TMP_DIR=FI_TMP_DIR)
        if not 'LD_PRELOAD' in run_env:
            run_env['LD_PRELOAD'] = self.fi_ld_load_loc
        elif not 'libfaultinject.so' in run_env['LD_PRELOAD']:
            run_env['LD_PRELOAD'] += ':' + self.fi_ld_load_loc
        if not 'LD_LIBRARY_PATH' in run_env:
            run_env['LD_LIBRARY_PATH'] = self.fi_ld_lib_path
        else:
            run_env['LD_LIBRARY_PATH'] += ':' + self.fi_ld_lib_path
        if not 'PYTHON_PATH' in run_env:
            run_env['PYTHON_PATH'] = self.fi_python_path
        else:
            run_env['PYTHON_PATH'] += ':' + self.fi_python_path
        run_env['FAULTINJECT_LIBRARY_NAME'] = self.fi_lib_name
        return run_env

    def set_testset_list(self, testset_list):
        self.testset_list = testset_list

    def get_testset_count(self):
        return len(self.testset_list)

    def dump_config(self, config_file):
        with open(config_file, 'w') as f:
            for testset in self.testset_list:
                # Dump this specific testset
                f.write('cmd=' + testset.cmd + '\n')
                f.write('    failcountbeg=' + str(testset.fail_count_beg) + '\n')
                f.write('    failcountend=' + str(testset.fail_count_end) + '\n')
                f.write('    failcountignore=' + ",".join(str(i) for i in testset.fail_count_ignore) + '\n')
                f.write('    timeout=' + str(testset.timeout) + '\n')

    def run(self):
        for testset in self.testset_list:
            for failcount in range(testset.fail_count_beg, testset.fail_count_end + 1):
                if not failcount in testset.fail_count_ignore:
                    dbg(1, 'Running test : ' + testset.cmd)
                    test = Test(testset.cmd, self.test_env, failcount, testset.timeout)
                    result = test.run()

                    tmp_dbg_str = '[' + '{0: <50}'.format(testset.cmd)
                    tmp_dbg_str += ' , fi_count: ' + str(failcount) + ']  ..  '
                    if result:
                        tmp_dbg_str += '[PASS]'
                    else:
                        tmp_dbg_str += '[FAIL]'
                    dbg(0, tmp_dbg_str)
                    if not result and not self.proceed_on_failure:
                        dbg(1, 'Aborted testing at the first test failure.')
                        sys.exit(2)

Testset = namedtuple('Testset', ['cmd', 'fail_count_beg', 'fail_count_end', 'fail_count_ignore', 'timeout'])

class Test(object):
    def __init__(self, cmd, global_test_env, failcount, timeout):
        self.cmd = cmd
        self.failcount = failcount
        self.run_env = dict(global_test_env,
            FAULTINJECT_FAIL_COUNT=str(self.failcount))
        self.timeout = timeout
        self.proc = None

    def pre_test_cleanup(self):
        # Recreate an empty test directory
        cmd = 'rm -rf ' + FI_TMP_DIR
        cmd += ' && '
        cmd += 'mkdir ' + FI_TMP_DIR
        process = Popen(cmd, shell=True)    
        process.wait()

    def dump_testconfig(self, filename):
        with open(filename, 'w') as f:
            # Dump this specific test
            f.write('cmd=' + self.cmd + '\n')
            f.write('    failcountbeg=' + str(self.failcount) + '\n')
            f.write('    failcountend=' + str(self.failcount) + '\n')
            f.write('    timeout=' + str(self.timeout) + '\n')

    def post_test_cleanup(self, save_files):
        #save_files = True
        if save_files:
            # Move core and log files into a separate dir
            save_dir = CUR_DIR + '/FI_TEST.' + str(self.proc.pid) + '/'
            dbg(1, 'Pid for the failed process: ' + str(self.proc.pid))
            dbg(0, 'Saving the generated logs and config in dir: ' + save_dir)

            cmd = 'mv ' + FI_TMP_DIR + ' ' + save_dir
            cmd += ' && '
            cmd += 'mv ' + CUR_DIR + '/core.*.' + str(self.proc.pid) + ' ' + save_dir
            process = Popen(cmd, stderr=PIPE, shell=True)
            process.wait()

            # Dump config to reproduce
            conf_file = save_dir + 'config_' + str(self.proc.pid) + '.fi'
            self.dump_testconfig(conf_file)
        else:
            # delete the core files and logs if not asking to save them
            cmd = 'rm -f ' + CUR_DIR + '/core.*.' + str(self.proc.pid) 
            cmd += ' && '
            cmd += 'rm -rf ' + FI_TMP_DIR
            dbg(1, 'Cleaning up temporary files')
            process = Popen(cmd, shell=True)
            process.wait()

    def did_test_fail(self, retcode):
        # Following signals are erroneous exits, but likely generated by the
        # application themselves as part of graceful handling of the error. We will
        # not treat these as bugs in our case
        SIGABRT = -6
        err_ignore_list = [SIGABRT]

        if retcode < 0 and (retcode not in err_ignore_list):
            dbg(1, 'exit code ' + str(retcode) + ' considered as non-graceful failure.')
            return True
        return False

    def run(self):
        self.proc = Process(self.cmd, self.run_env)
        self.pre_test_cleanup()
        retcode = self.proc.run(self.timeout)
        dbg(1, 'Process exited with return code: ' + str(retcode))

        test_failed = self.did_test_fail(retcode)
        self.post_test_cleanup(save_files = test_failed)

        return (not test_failed)

class Process(object):
    def __init__(self, cmd, run_env):
        self.cmd = cmd
        self.run_env = run_env
        self.process = None
        self.pid = None

    def dump_backtraces(self):
        # Obtain backtraces for all threads using pmp every few seconds and dump in
        # a file, might help later to debug
        dbg(1, 'Collecting backtraces for pid ' + str(self.pid))
        filename = FI_TMP_DIR + '/fi_pid_' + str(self.pid) + '_hung_btt.log'
        with open(filename, 'w') as f:
            for i in range(3):
                pmp_process = Popen([PMP_PATH, str(self.pid)], stdout=PIPE, stderr=PIPE)
                out = pmp_process.communicate()
                f.write(out[0] + '\n')
                dbg(1, 'Backtrace:\n' + out[0]) 
                time.sleep(1)

    def run(self, timeout):
        def exec_proc():
            dbg(2, 'Executing command .. ' + self.cmd)
            if verbose > 1:
                self.process = Popen(shlex.split(self.cmd), env=self.run_env)
            else:
                self.process = Popen(shlex.split(self.cmd),
                     stdout=PIPE, stderr=PIPE, env=self.run_env)
            self.process.communicate()

        thread = threading.Thread(target=exec_proc)
        thread.start()

        thread.join(timeout)
        self.pid = self.process.pid
        if thread.is_alive():
            dbg(1, 'Process timed-out, assuming process to be hung, collect backtraces and terminate ..')
            # Let's save a few iterations of backtraces for all threads, might help debug
            self.dump_backtraces()

            # Kill the process, this should get us core as well
            self.process.send_signal(signal.SIGQUIT)
            thread.join()

        return self.process.returncode

def append_testset_list_from_cmd_list(testset_list, cmd_list, fail_count_beg, fail_count_end,
    fail_count_ignore, timeout):
    if fail_count_beg > fail_count_end:
        dbg(0, 'Fault injection begin count can not be greater than end count')
        sys.exit(2)
    for cmd_itr in cmd_list:
        testset_list.append(Testset(cmd = cmd_itr, fail_count_beg = fail_count_beg,
            fail_count_end = fail_count_end, fail_count_ignore = fail_count_ignore, timeout = timeout))

def append_testset_list_from_config(testset_list, conf_file):
    with open(conf_file, 'r') as f:
        lines = [x.strip('\n') for x in f.readlines()]

    fail_count_beg = None
    fail_count_end = None
    fail_count_ignore = []
    timeout = None
    cmd_list = []

    for line in lines:
        if 'cmd=' == line[:len('cmd=')]:
            if fail_count_beg != None:
                if len(cmd_list) == 0:
                    dbg(0, 'Need to have atleast one cmd found before failcountbeg')
                    sys.exit(2)
                append_testset_list_from_cmd_list(testset_list, cmd_list, fail_count_beg, fail_count_end,
                    fail_count_ignore, timeout)
                fail_count_beg = None
                fail_count_end = None
                fail_count_ignore = []
                timeout = None
                cmd_list = []

            cmd_list.append(line[len('cmd='):])
            continue
        if '    failcountbeg=' == line[:len('    failcountbeg=')]:
            fail_count_beg = int(line[len('    failcountbeg='):])
            continue
        if '    failcountend=' == line[:len('    failcountend=')]:
            fail_count_end = int(line[len('    failcountend='):])
            continue
        if '    failcountignore=' == line[:len('    failcountignore=')]:
            fail_count_ignore = [int(s) for s in line[len('    failcountignore='):]]
            continue
        if '    timeout=' == line[:len('    timeout=')]:
            timeout = int(line[len('    timeout='):])
            continue

    if len(cmd_list) == 0:
        dbg(0, 'No cmd found in the config file')
        sys.exit(2)
    if fail_count_beg == None:
        dbg(0, 'Did not find failcountbeg for the last cmd to run')
        sys.exit(2)
    append_testset_list_from_cmd_list(testset_list, cmd_list, fail_count_beg, fail_count_end,
        fail_count_ignore, timeout)

if __name__ == '__main__':
    # default parameters
    fail_count_beg = 1
    fail_count_end = 100
    fail_count_ignore = []
    proceed_on_failure = False
    read_from_config = False
    dump_config = False
    timeout = 300
    fi_lib_path = None

    # Process arguments passed
    args = sys.argv[1:]
    cmd_list = []
    while len(args) > 0:
        arg = args.pop(0)

        # Command line options
        if arg[0] == '-':
            option = arg[1:]
            if option == '-config' or option == 'c':
                config_file_read = args.pop(0)
                read_from_config = True
                continue
            if option == '-configdump' or option == 'C':
                config_file_dump = args.pop(0)
                dump_config = True
                continue
            if option == '-failcountbeg' or option == 'b':
                fail_count_beg = int(args.pop(0))
                continue
            if option == '-failcountend' or option == 'e':
                fail_count_end = int(args.pop(0))
                continue
            if option == '-failcountignore' or option == 'i':
                fail_count_ignore = [int(s) for s in args.pop(0).split(',')]
                continue
            if option == '-filibpath' or option == 'l':
                fi_lib_path = args.pop(0)
                continue
            if option == '-proceedonfailure' or option == 'p':
                proceed_on_failure = True
                continue
            if option == '-timeout' or option == 't':
                timeout = int(args.pop(0))
                continue
            if option == '-verbose':
                verbose = int(args.pop(0))
                if verbose < 0 or verbose > 2:
                    dbg(0, 'A valid value for verbose is between 0 and 2.')
                    sys.exit(2)
                continue
            if option == 'v':
                verbose = 1
                continue
            if option == 'vv':
                verbose = 2
                continue
            dbg(0, 'unknown arg: ' + arg)
            usage()
            sys.exit(2) 
        cmd_list.append(arg)

    if fi_lib_path == None:
        dbg(0, "Fault injection library path not specified.")
        sys.exit(2)

    # Set various paths for the test env
    ld_lib_path = fi_lib_path + '/.libs'
    ld_lib_path += ':'
    ld_lib_path += CUR_DIR + '/.libs'
    ld_preload = fi_lib_path + '/.libs/libfaultinject.so'

    testsuite = Testsuite(proceed_on_failure,
        DEF_FAULTINJECT_LIBRARY_NAME,
        ld_lib_path,
        ld_preload,
        DEF_PYTHON_PATH)

    testset_list = []
    if read_from_config:
        append_testset_list_from_config(testset_list, config_file_read)
    if len(cmd_list) != 0:
        append_testset_list_from_cmd_list(testset_list, cmd_list, fail_count_beg, fail_count_end,
            fail_count_ignore, timeout)
    testsuite.set_testset_list(testset_list)

    if testsuite.get_testset_count() == 0:
        dbg(0, 'No tests specified to run')
    else:
        if dump_config:
            testsuite.dump_config(config_file_dump)
        testsuite.run()
