#!/usr/bin/env python

# Generate the faultinject source code from the prototype input.

import filecmp, os, re, shutil, sys, textwrap

verbose = 1

input_file_name = '../src/faultinject.c.in'
tmp_file_name = '__t'
output_file_name = '../src/faultinject.c'

replace_symbols_re = re.compile(r'(.*)@(.*)@(.*)')
func_def_parts_re = re.compile(r'(?P<ret_type>.+)\((?P<name>\w+)\)\((?P<args>[^)]*)')
param_names_re = re.compile(r'(\w+)(?:,|$)')

# compare_srcfile --
#   Compare two files, and if they differ, update the source file.
# Copied from WiredTiger dist/dist.py
def compare_srcfile(tmp, src):
    if not os.path.isfile(src) or not filecmp.cmp(tmp, src, shallow=False):
        print('Updating ' + src)
        shutil.copyfile(tmp, src)
    os.remove(tmp)

def get_func_list():
    # Get the list of functions being overridden.
    func_list_file = open('function.list', 'r')
    func_list = list(func_list_file)
    func_list_file.close()

    # Clean up the function list.
    func_list = [k for k in func_list if not k.startswith('#')]
    func_list2 = []
    for func_def in func_list:
        match = func_def_parts_re.match(func_def)
        if not match:
            sys.stderr.write("Skipping un-parseable function: " + func_def)
        else:
            if verbose > 1:
                print 'func def: {0} ret: {1}, name: {2}, args: {3}'. \
                    format( func_def,       \
                    match.group('ret_type'),    \
                    match.group('name'),    \
                    match.group('args') )
            func_list2.append(func_def)
    return func_list2

def generate_typedef_content():
    content = ''
    for func_def in func_list:
        match = func_def_parts_re.match(func_def)
        new_def = 'typedef {ret} (*libc_{name}_t)({args});\n'.format(
            ret=match.group('ret_type'), \
            name=match.group('name'), \
            args=match.group('args'))
        content = content + new_def
    return content  

def generate_function_declaration_content():
    content = ''
    for func_def in func_list:
        match = func_def_parts_re.match(func_def)
        new_def = 'static libc_{name}_t libc_{name} = NULL;\n'.format(
            name=match.group('name'))
        content += new_def
    return content

def generate_constructor_assignment_content():
    content = ''
    for func_def in func_list:
        match = func_def_parts_re.match(func_def)
        new_def = '''
    libc_{name} = (libc_{name}_t)(intptr_t)dlsym(RTLD_NEXT, "{name}");
    if (libc_{name} == NULL || (err_string = dlerror()) != NULL) {{
        fprintf(stderr, "Error initializing {name}: %d, %s\\n", errno, err_string);
        _exit(1);
    }}
'''.format(name=match.group('name'))
        content += new_def
    return content

def generate_global_config_declaration_content():
    content = ''
    for func_def in func_list:
        match = func_def_parts_re.match(func_def)
        new_config = '''
static uint64_t g_max_{name}_op_count = 0;
static uint64_t g_{name}_op_count = 0;
'''.format(name=match.group('name'))
        content += new_config
    return content

def generate_constructor_config_content():
    content = ''
    for func_def in func_list:
        match = func_def_parts_re.match(func_def)
        new_config = '''
	if ((env_string = getenv("FAULTINJECT_{upper_name}_COUNT")) != NULL &&
	    strlen(env_string) != 0) {{
		errno = 0;
		g_max_{name}_op_count = strtoull(env_string, NULL, 10);
#ifdef HAVE_TRACE
		printf("Set max {name} op count to: %d\\n", (int)g_max_{name}_op_count);
#endif
		if (errno != 0) {{
			fprintf(stderr, "Failed to parse environment variable\\n");
			g_max_{name}_op_count = 0;
		}}
	}}
'''.format(upper_name=match.group('name').upper(), name=match.group('name'))
        content += new_config
    return content

# For now never fail an operation check.
def generate_operation_check_content():
    content = ''
    for func_def in func_list:
        match = func_def_parts_re.match(func_def)
        new_config = '''
	else if (g_max_{name}_op_count > 0 &&
        ++g_{name}_op_count > g_max_{name}_op_count) {{
#ifdef HAVE_TRACE
		printf("failing {name} with op count: %d\\n", (int)g_{name}_op_count);
#endif
		return (EFAULT);
	}}'''.format(name=match.group('name'))
        content += new_config
    # Trim the first else from the content.
    content = content.replace('else ', '', 1)
    return content

def generate_function_definition_content():
    content = ''
    for func_def in func_list:
        match = func_def_parts_re.match(func_def)
        outargs_str = ''
        outargs = param_names_re.findall(match.group('args'))
        for outarg in outargs:
            outargs_str = outargs_str + outarg + ', '
        # Generate code to print out function parameters based on the
        # argument types.
        param_prints_str = ''
        for param in match.group('args').split(','):
            # Make sure there is a single space between words
            param = ' '.join(param.strip().replace(
                '*', ' * ').split())
            pname = param.split()[-1]
            output_type = ''
            if param.startswith('char *') or    \
                param.startswith('const char *'):
                output_type = 's'
            elif param.startswith('int') or     \
                param.startswith('long'):
                output_type = 'd'
            if output_type != '':
                param_prints_str += '''
        fprintf(log_fd, "{name}:%{type}, ", {name});'''.format(
                    name=pname, type=output_type)
            elif param == '...':
                param_prints_str += '''
        fprintf(log_fd, "...");'''
            else:
                param_prints_str += '''
        fprintf(log_fd, "{name}:{type}, ");'''.format(
                    name=pname, type=param.rsplit(' ', 1)[0])
        # Trim our trailing comma
        outargs_str = outargs_str[:-2]
        open_vararg_str = ''
        # Get error return values right - return NULL for functions that
        # return a pointer else EFAULT
        ret_value = 'NULL'
        if match.group('ret_type').find('*') == -1:
            ret_value = '-1'
        # open functions have an optional mode parameter. Deal with
        # that here.
        if match.group('name') == 'open' or match.group('name') == 'open64':
            # Add in the mode parameter to the outarg list.
            outargs_str += ', mode'
            open_vararg_str = '''
    va_list ap;
    mode_t mode;

    va_start(ap, oflag);
#if SIZEOF_MODE_T < SIZEOF_INT
    mode = (mode_t)va_arg(ap, int);
#else
    mode = va_arg(ap, mode_t);
#endif
    va_end(ap);

    /*
     * In pthread environments the dlsym call may call our open, we simply
     * ignore it because libc is already loaded.
     */
    if (!libc_{name}) {{
        errno = EFAULT;
        return ({retval});
    }}
'''.format(name=match.group('name'), retval=ret_value)
        new_def = '''
FAULT_INJECT_API {ret} {name}({args})
{{
    int ret;
    FILE *log_fd;

{open_vararg}
    if (faultinject_caller_interesting()) {{
        #ifdef HAVE_TRACE
            if (faultinject_trace_operation()) {{
                /* Log the operation */
                log_fd = (*libc_fopen)("/tmp/faultinject_{name}.log", "a");
                if (log_fd != NULL) {{
                    fprintf(log_fd, "Intercepted call to: {name}(");
                    {param_prints}
                    fprintf(log_fd, ");\\n");
                    (*libc_fclose)(log_fd);
                }}
            }}
        #else
            log_fd = NULL; log_fd = log_fd;
        #endif
            if ((ret = faultinject_fail_operation()) != 0) {{
                errno = ret;
                return ({retval});
            }}
    }}
    return (*libc_{name})({outargs});
}}
        '''.format(name=match.group('name'),
            ret=match.group('ret_type'),
            open_vararg=open_vararg_str,
            param_prints=param_prints_str,
            retval=ret_value,
            outargs=outargs_str,
            args=match.group('args'))
        content += new_def
    return content

def generate_output(match):
    content_string = ''
    if match == 'CONSTRUCTOR_ASSIGNMENTS':
        content_string = generate_constructor_assignment_content()
    elif match == 'CONSTRUCTOR_CONFIG':
        content_string = generate_constructor_config_content()
    elif match == 'FUNCTION_DECLARATIONS':
        content_string = generate_function_declaration_content()
    elif match == 'FUNCTION_DEFINITIONS':
        content_string = generate_function_definition_content()
    elif match == 'GLOBAL_CONFIG_DECLARATIONS':
        content_string = generate_global_config_declaration_content()
    elif match == 'OPERATION_CHECK':
        content_string = generate_operation_check_content()
    elif match == 'TYPEDEFS':
        content_string = generate_typedef_content()
    else:
        print 'Unsupported replace symbol: ' + match
        sys.exit(1)

    return content_string


func_list = get_func_list()

tmp_file = open(tmp_file_name, 'w')

tmp_file.write('''
/* Do not edit. Automatically generated from content in faultinject.c.in */
''')

for line in open(input_file_name, 'r'):
    match = replace_symbols_re.search(line)
    if match:
        tmp_file.write(match.group(1) + generate_output(match.group(2)) + \
            match.group(3))
    else:
        tmp_file.write(line)
tmp_file.close()

compare_srcfile(tmp_file_name, output_file_name)
