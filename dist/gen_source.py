#!/usr/bin/env python

# Generate the faultinject source code from the prototype input.

import filecmp, os, re, shutil, sys, textwrap

input_file_name='../src/faultinject.c.in'
tmp_file_name='__t'
output_file_name='../src/faultinject.c'

replace_symbols_re = re.compile(r'(.*)@(.*)@(.*)')
func_def_parts_re = re.compile(r'(?P<ret_type>\w+) (?P<name>\w+)\((?P<args>[^)]*)')
param_names_re = re.compile(r'(\w+)(?:,|$)')

# compare_srcfile --
#	Compare two files, and if they differ, update the source file.
# Copied from WiredTiger dist/dist.py
def compare_srcfile(tmp, src):
	if not os.path.isfile(src) or not filecmp.cmp(tmp, src, shallow=False):
		print('Updating ' + src)
		shutil.copyfile(tmp, src)
	os.remove(tmp)

def generate_typedef_content():
	content = ''
	for func_def in func_list:
		match = func_def_parts_re.match(func_def)
		# Ignore comments in the function definition file
		if not match:
			continue;
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
		# Ignore comments in the function definition file
		if not match:
			continue;
		new_def = 'static libc_{name}_t libc_{name} = NULL;\n'.format(
		    name=match.group('name'))
		content = content + new_def
	return content

def generate_constructor_assignment_content():
	content = ''
	for func_def in func_list:
		match = func_def_parts_re.match(func_def)
		# Ignore comments in the function definition file
		if not match:
			continue;
		new_def = '''
	libc_{name} = (libc_{name}_t)(intptr_t)dlsym(RTLD_NEXT, "{name}");
	if (libc_{name} == NULL || dlerror())
		_exit(1);
'''.format(name=match.group('name'))
		content = content + new_def
	return content

# For now never fail an operation check.
def generate_operation_check_content():
	return ''

def generate_function_definition_content():
	content = ''
	for func_def in func_list:
		match = func_def_parts_re.match(func_def)
		# Ignore comments in the function definition file
		if not match:
			continue;
		outargs_str = ''
		outargs = param_names_re.findall(match.group('args'))
		for outarg in outargs:
			outargs_str = outargs_str + outarg + ', '
		# Trim our trailing comma
		outargs_str = outargs_str[:-2]
		open_vararg_str = ''
		# open functions have an optional mode parameter. Deal with
		# that here.
		if 'open' in match.group('name'):
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
		return (-1);
	}}
'''.format(name=match.group('name'))
		new_def = '''
{ret} FAULT_INJECT_API {name}({args})
{{
	int ret;
{open_vararg}
	if ((ret = faultinject_fail_operation()) != 0) {{
		errno = ret;
		return (ret);
	}}
	return (*libc_{name})({outargs});
}}
		'''.format(name=match.group('name'),
		    ret=match.group('ret_type'),
		    open_vararg=open_vararg_str,
		    outargs=outargs_str,
		    args=match.group('args'))
		content += new_def
	return content

def generate_output(match):
	content_string = ''
	if match == 'TYPEDEFS':
		content_string = generate_typedef_content()
	elif match == 'FUNCTION_DECLARATIONS':
		content_string = generate_function_declaration_content()
	elif match == 'CONSTRUCTOR_ASSIGNMENTS':
		content_string = generate_constructor_assignment_content()
	elif match == 'OPERATION_CHECK':
		content_string = generate_operation_check_content()
	elif match == 'FUNCTION_DEFINITIONS':
		content_string = generate_function_definition_content()
	else:
		print 'Unsupported replace symbol: ' + match
		sys.exit(1)

	return content_string

# Get the list of functions being overridden.
func_list_file = open('function.list', 'r')
func_list = list(func_list_file)
func_list_file.close()

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
