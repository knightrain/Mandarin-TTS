#!/usr/bin/env pyhon
# -*- coding: utf-8 -*-

import re

g_regex = [re.compile(r'\s'), re.compile(r'\s')]
char_map = {}
words_list = []
def import_from_unicode(unifile):
    re_s = re.compile(r'(\(.*?\))')
    with open(unifile) as lines:
        for line in lines:
            line = line.strip()
            line = re_s.sub('', line)
            items = g_regex[0].split(line, 1)
            code = int('0x'+items[0], 16)
            if code > 0x10000:
                continue 
            char = unichr(code)
            if char not in char_map:
                char_map[char] = items[1].lower()
                
def is_special_phon(zh_word, pinyins):
    pinyins = pinyins.split(" ")
    if zh_word[0] == unichr(0x4e00) or zh_word[0] == unichr(0x4e0d):
        if pinyins[0][-1] == "2":
            return False
    for i in range(len(zh_word)):
        if zh_word[i] == unichr(0x4e00) or zh_word[i] == unichr(0x4e0d):
            if i + 1 != len(zh_word) and pinyins[i][-1] == "2" and pinyins[i+1][-1] == "4":
                pass
        if zh_word[i] in char_map:
            pin = char_map[zh_word[i]]
            pin = pin.split(' ')
            if pin[0][-1] != pinyins[i][-1]:
                return True 
        else:
            return True
    return False

def import_from_zhlist(zh_list):
    re_c = [re.compile(r'^(.)\s([a-z]*[1-5])$'),
            re.compile(r'^\((.*?)\)\s([^\s]*)')]
    with open(zh_list) as lines:
        for line in lines:
            line = line.decode("utf-8")
            m = re_c[0].match(line)
            if m:
                char = m.group(1)
                p = m.group(2)
                if char not in char_map:
                    char_map[char] = p
            else:
                m = re_c[1].match(line)
                if m:
                    word = m.group(1).replace(' ', '')
                    p = m.group(2)
                    for num in range(0, 6):
                        p = p.replace(str(num), str(num) + " ")
                    p = p.rstrip()
                    if is_special_phon(word, p):
                        words_list.append([word, p])

def zh_word_cmp(word1, word2):
    len1 = len(word1)
    len2 = len(word2)
    min_len = min(len1, len2)
    i = 0
    while i < min_len:
        if word1[i] > word2[i]:
            return 1
        elif word1[i] < word2[i]:
            return -1
        i += 1
    if len1 > len2:
        return 1
    elif len1 < len2:
        return -1
    return 0

def write_to_list(listfile):
    with open(listfile, 'w') as wfile:
        chars = char_map.keys()
        chars.sort()
        for char in chars:
            string = char + ' ' + char_map[char] + '\n'
            string = string.encode("utf-8")
            wfile.write(string)

        for word, p in words_list:
            string = '(' + word + ') ' + p + '\n'
            string = string.encode("utf-8")
            wfile.write(string)
			
def main():
	import_from_unicode("HanyuPinlu.txt")
	import_from_unicode("Mandarin.txt")
	import_from_zhlist("zh_list")
	import_from_zhlist("zh_listx")
	write_to_list('Mandarin.list')

if __name__ == '__main__':
	main()
			
