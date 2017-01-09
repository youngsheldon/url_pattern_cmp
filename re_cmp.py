#!/usr/bin/env python
# -*- coding: utf-8 -*-
# @Author: anchen
# @Date:   2016-12-22 00:12:05
# @Last Modified by:   anchen
# @Last Modified time: 2017-01-08 22:42:48
import os 
import re
import commands 
import sys
reload(sys)
sys.setdefaultencoding('utf-8')

regex_before = '(https?:|www\.)[\w./\- ]{5,30}(\. ?)((c(c|n|(o|0)m))|org|net|so|im)|(https?:|www\.)[\w./\- ]{5,33}\w|\w[\w./\- ]{3,30}(\. ?)((c(c|n|(o|0)m))|org|net|so|im)[\w./\- ]{3,30}'

regex_after = '(https?:|www\.)[\w./\- ]{5,30}(\. ?)((c(c|n|(o|0)m))|org|net|so|im)[\\+\\?\\=\\&\\_:\%\w./\- ]+[/\w$]|(https?:|www\.)[\w./\- ]{5,33}\w[\%:\w./\- ]+[/\w$]|\w[\w./\- ]{0,30}(\. ?)\w[\w./\- ]{0,30}[/\w$]|(https?:|www\.)[\w./\- ]{5,33}\w'

class Recmp(object):
    def __init__(self, source_path):
        self.source_path = source_path
        self.file_index = self.source_path.split('/')[0]

    def GetFilePathList(self,rootDir):
        FilePathList=[]
        for root,dirs,files in os.walk(rootDir):
            for filespath in files:
                FilePathList.append(os.path.join(root,filespath))
        return FilePathList

    def write_to_file(self,path,content):
        with open(path,'a+') as f:
            f.write(content)

    def set_regex(self,flag):
        if flag == 1:
            regex = regex_before
        else:
            regex = regex_after
        with open('set.ini','w+') as f:
            f.write(regex)

    def regex_pattern(self,expr,content):
        ret = re.compile(expr).findall(content)
        return ret 

    def url_fliter(self,in_f,out_f):
        with open(in_f,'rb') as f:
            for line in f:
                v = line.strip()
                if '|' in v:
                    self.write_to_file(out_f,v + '\n')

    def get_url_access_state(self,url):
        cmd = 'curl -I -m 1 -o /dev/null -s -w %{http_code}  ' + url 
        state= commands.getstatusoutput(cmd)  
        return state[1]

    def distinct_url(self,path):
        url_list = []
        all_list = []
        with open(path,'r') as f:
            for line in f:
                v = line.strip().split('|')
                url = v[1]
                if url not in url_list:
                    url_list.append(url)
                    all_list.append(line.strip())
        return all_list

    def sort_diff_url(self,dev):
        path = dev + '_diff.txt'
        sort_path = dev + '_diff_sort.csv'
        url_relate_list = self.distinct_url(path)
        index =0 
        for content in url_relate_list:
            url = content.split('|')[1]
            sms = content.split('|')[2]
            state = self.get_url_access_state(url)
            out = str(index) + ',' + url + ',' + state + ',' + sms +'\n'
            self.write_to_file(sort_path,out)
            index += 1 
            print '[' + str(index) + ']' + url + ':' + str(state) 
        diff_url_num = self.get_diff_url(sort_path)
        print 'diff_url_num=' + str(diff_url_num)
        return diff_url_num

    def diff(self,path,dev):
        with open(path,'rb') as f:
            for line in f:
                v = line.strip().split('|')
                index = v[0]
                url = v[1]
                sms = v[2]
                out = str(index) + '|' + str(url) + '|' + str(sms) + '\n'
                ret = self.regex_pattern(regex_before,sms)
                if ret:
                    self.write_to_file(dev + '_same.txt',out)
                else:
                    if len(str(url)) > 7:
                        expr = '^\d.*\d$'
                        ret2 = self.regex_pattern(expr,url)
                        if ret2:
                            pass 
                        else:
                            ret3 = url.find(' ')
                            if ret3 != -1:
                                pass
                            else:
                                self.write_to_file(dev + '_diff.txt',out)   

    def calcul_line_num(self,target_file):
        cmd = 'cat ' + target_file + ' |wc -l'
        ret= commands.getstatusoutput(cmd) 
        return ret[1]

    def get_uniq_line_num(self,target_file):
        cmd1 = 'cat ' + target_file + ' | ' + 'awk -F\'|\' \'{print $2}\' >> tmp.txt'
        cmd2 = 'sort tmp.txt | uniq > ' + 'unique.txt'
        os.system(cmd1)
        os.system(cmd2)
        os.system('rm tmp.txt')
        ret = self.calcul_line_num('unique.txt')
        os.system('rm unique.txt')
        return ret 

    def get_diff_url(self,input_file):
        diff_url_list = []
        with open(input_file) as f:
            for line in f:
                v = line.strip().split(',')
                url = v[1]
                state = v[2]
                sms = v[3]
                if state != '000':
                    diff_url_list.append(url + ',' + sms)
        with open(self.file_index + 'diff_url.csv','a+') as f:
            for v in diff_url_list:
                f.write(v + '\n')
        return len(diff_url_list)

    def handle_re_before(self,input_file):
        self.set_regex(1)
        re_before_out = str(self.file_index) + '_before_re.txt'
        out = './re ' + input_file + ' >> ' + re_before_out
        os.system(out)   
        print 'finish re before' 
        sort_before_out = str(self.file_index) + 'sort_before.txt'    
        self.url_fliter(re_before_out,sort_before_out)
        print 'finish sort before'
        be_num = self.calcul_line_num(sort_before_out)
        be_uniq_num = self.get_uniq_line_num(sort_before_out)
        print 'be_num=' + str(be_num)
        print 'be_uniq_num=' + str(be_uniq_num)
        out = 'rm ' + re_before_out
        os.system(out) 
        return str(be_num),str(be_uniq_num)

    def handle_re_after(self,input_file):
        self.set_regex(2)
        re_after_out = str(self.file_index) + '_after_re.txt'
        out = './re ' + input_file + ' >> ' + re_after_out
        os.system(out)    
        print 'finish re after' 
        sort_after_out = str(self.file_index) + 'sort_after.txt'    
        self.url_fliter(re_after_out,sort_after_out)
        print 'finish sort after'
        af_num = self.calcul_line_num(sort_after_out)
        af_uniq_num = self.get_uniq_line_num(sort_after_out)
        print 'af_num=' + str(af_num)
        print 'af_uniq_num=' + str(af_uniq_num)
        out = 'rm ' + re_after_out
        os.system(out) 

        self.diff(sort_after_out,str(self.file_index))
        print 'finish diff'
        diff_url_num = self.sort_diff_url(str(self.file_index))
        print 'finish diff sort'
        return str(af_num),str(af_uniq_num),str(diff_url_num)

    def run(self):
        output_file_name = str(self.file_index) + '_sms.txt'
        file_list = self.GetFilePathList(self.source_path)
        for fp in file_list:
            with open(fp,'rb') as f:
                for line in f:
                    v = line.strip().split(',')
                    sms_text = v[12].split('\"')
                    out = sms_text[1] + '\n'
                    self.write_to_file(output_file_name,out)
        print 'finish get sms'
        be_num,be_uniq_num = self.handle_re_before(output_file_name)
        af_num,af_uniq_num,diff_url_num = self.handle_re_after(output_file_name)
        out = '日期' + ',' + '调整前匹配URL的数量' + ',' + '调整前匹配URL的数量(去重)' + ',' + '调整后匹配URL的数量' + ',' + '调整后匹配URL的数量(去重)' + ',' + '调整后新增URL(去重)' + '\n'
        out += self.file_index + ',' + be_num + ',' + be_uniq_num + ',' + af_num + ',' + af_uniq_num + ',' + diff_url_num + '\n'
        self.write_to_file(self.file_index + '_sum.csv',out)
        cmd = 'mkdir sum_' + self.file_index
        os.system(cmd)
        cmd2 = 'mv ' + self.file_index + '*' + ' ./sum_' + self.file_index + '/'
        os.system(cmd2)

path = sys.argv[1]
obj = Recmp(path)
obj.run()