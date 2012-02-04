#!/usr/bin/env pyhon
# -*- coding: utf-8 -*-

import re

g_regex = [re.compile(r'\s'), re.compile(r'\s')]
mappings = {}
def import_from_unicode(unifile):
	re_s = re.compile(r'(\(.*?\))')
	with open(unifile) as lines:
		for line in lines:
			line = line.strip()
			line = re_s.sub('', line)
			items = g_regex[0].split(line, 1)
			char = unichr(int('0x'+items[0], 16))
			if char in mappings:
				mappings[char] = items[1]

def import_from_zhlist(zh_list):

def write_to_list(listfile):
	keys = mappings.keys()
	keys.sort()
	with open(listfile, 'w') as wfile:
		for key in keys:
			string = key + ' ' + mappings[key] + '\n'
			string = string.encode("utf-8")
			wfile.write(string)
			
def main():
	import_from_unicode("HanyuPinlu.txt")
	import_from_unicode("Mandarin.txt")
	write_to_list('my.list')

if __name__ == '__main__':
	main()
			
