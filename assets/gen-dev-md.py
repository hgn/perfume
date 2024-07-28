#!/usr/bin/env python3

import re
import sys
from typing import List, Dict, Union

def parse_header(file_path: str) -> List[Dict[str, Union[str, List[Dict[str, str]]]]]:
    with open(file_path, 'r') as file:
        lines = file.readlines()

    functions = []
    current_function = None
    inside_comment = False

    for line in lines:
        line = line.strip()
        if line.startswith('/**'):
            inside_comment = True
            current_function = {'description': '', 'params': [], 'returns': ''}
        elif inside_comment and line.startswith('* @brief'):
            current_function['description'] += line.replace('* @brief', '').strip() + ' '
        elif inside_comment and line.startswith('* @param'):
            param = line.replace('* @param', '').strip()
            if ' ' in param:
                param_name, param_desc = param.split(' ', 1)
                current_function['params'].append({'name': param_name, 'description': param_desc, 'type': ''})
        elif inside_comment and line.startswith('* @return'):
            current_function['returns'] += line.replace('* @return', '').strip() + ' '
        elif inside_comment and line.startswith('*') and not line.startswith('*/'):
            if current_function:
                if 'returns' in line:
                    current_function['returns'] += line.replace('*', '').strip() + ' '
                elif 'param' in line:
                    if current_function['params']:
                        current_function['params'][-1]['description'] += line.replace('*', '').strip() + ' '
                else:
                    current_function['description'] += line.replace('*', '').strip() + ' '
        elif line.startswith('*/'):
            inside_comment = False
        elif line.endswith(';') and '(' in line and ')' in line and current_function:
            match = re.match(r'(\w+)\s+(\w+)\s*\((.*)\);', line)
            if match:
                return_type, function_name, params = match.groups()
                current_function['name'] = function_name
                current_function['return_type'] = return_type
                if params.strip():
                    param_list = [p.strip().split(' ') for p in params.split(',')]
                    for idx, param in enumerate(current_function['params']):
                        if idx < len(param_list):
                            param['type'] = ' '.join(param_list[idx][:-1])
                functions.append(current_function)
                current_function = None

    return functions

def format_text(text: str, width: int = 80) -> str:
    lines = []
    while len(text) > width:
        split_pos = text.rfind(' ', 0, width)
        if split_pos == -1:
            split_pos = width
        lines.append(text[:split_pos].strip())
        text = text[split_pos:].strip()
    lines.append(text)
    return '\n'.join(lines)

def generate_markdown(functions: List[Dict[str, Union[str, List[Dict[str, str]]]]]) -> str:
    markdown = '## API Documentation\n\n'
    for function in functions:
        markdown += f"### {function['name']}\n\n"
        markdown += f"**Description:**\n\n{format_text(function['description'].strip())}\n\n"
        markdown += '#### Parameters\n\n'
        for param in function['params']:
            markdown += f"- **{param['name']}** (`{param['type']}`):\n\n  {format_text(param['description'].strip())}\n\n"
        markdown += f"**Returns:**\n\n`{function['return_type']}` - {format_text(function['returns'].strip())}\n\n"
    return markdown

def main() -> None:
    if len(sys.argv) != 2:
        print("Usage: gen-dev-md.py <header-file>")
        sys.exit(1)

    header_path = sys.argv[1]
    functions = parse_header(header_path)
    markdown = generate_markdown(functions)
    print(markdown)

if __name__ == "__main__":
    main()

